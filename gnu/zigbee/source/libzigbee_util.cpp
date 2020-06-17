
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <libzigbee_util.h>

#define ZIGBEE_ESCAPE_CODE					(0x7du)
#define ZIGBEE_ESCAPE_EXCLUSIVE_VALUE		(0x20u)
#define ZIGBEE_START_DELIMITER				(0x7eu)
#define ZIGBEE_FRAME_TYPE_AT_COMMAND		(0x08u)
#define ZIGBEE_FRAME_TYPE_AT_RESPONSE		(0x88u)
#define ZIGBEE_FRAME_TYPE_TRANSMIT_REQUEST	(0x10u)
#define ZIGBEE_FRAME_TYPE_TRANSMIT_STATUS	(0x8bu)
#define ZIGBEE_FRAME_TYPE_RECEIVE_PACKET	(0x90u)

#pragma	pack(1)
/**
 */
typedef struct {
	///
	uint8_t				start_delimiter;
	///
	uint16_t			length;
} xbee_frame_header_t;

/**
 */
typedef struct {
	///
	uint8_t				frame_type;
	///
	union {
		///
		struct {
			///
			uint8_t		frame_id;
			///
			uint8_t		command[0];
		}				at_cmd;
		///
		struct {
			///
			uint8_t		frame_id;
			///
			uint8_t		at_cmd[2];
			///
			uint8_t		status;
			///
			uint8_t		value[0];
		}				at_res;
		///
		struct {
			///
			uint8_t		frame_id;
			///
			uint8_t		dst_addr_64[8];
			///
			uint16_t	dst_addr_16;
			///
			uint8_t		broadcast_radius;
			///
			uint8_t		options;
			///
			uint8_t		data[0];
		}				tx_request_64;
		///
		struct {
			///
			uint8_t		frame_id;
			///
			uint16_t	dst_addr_16;
			///
			uint8_t		retry_count;
			///
			uint8_t		delivery_status;
			///
			uint8_t		discovery_status;
		}				tx_status;
		///
		struct {
			///
			uint8_t		src_addr_64[8];
			///
			uint16_t	src_addr_16;
			///
			uint8_t		options;
			///
			uint8_t		data[0];
		}				rx_packet_64;
	}					cmd_data;
} xbee_frame_data_t;

/**
 */
typedef struct {
	xbee_frame_header_t	header;
	xbee_frame_data_t	data;
} xbee_frame_t;
#pragma	pack()

/**
 */
const uint8_t c_byZigBeeEscapeCharacters[] = {
	ZIGBEE_START_DELIMITER,	// Frame delimiter
	ZIGBEE_ESCAPE_CODE,		// Escape
	0x11,					// XON
	0x13					// XOFF
};

/**
 */
CZigBee::CZigBee(CIOEventDispatcher& a_objDispatcher,
		CRfEventListener *a_pobjListener,
		const char a_szTty[])
	:	m_nBaudRate(B115200), m_bRxEscape(false), m_pbyRxBuffer(NULL),
			m_pbyPoolPointer(NULL), m_pobjEventListener(a_pobjListener)
{
	::memset(&m_objTerminos, 0, sizeof(m_objTerminos));
	int fd = CHandle::EInvalidHandleValue;
	fd = ::open(a_szTty, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		CDebugUtil::printf(CDebugUtil::EError, "%s: l(%d): errno=%d\n",
				__PRETTY_FUNCTION__, __LINE__, errno);
		assert(isValidHandle());
		return;
	}
	setHandle(fd);

	int flg = ::fcntl(getHandle(), F_GETFL, 0);
	flg |= O_NONBLOCK;
	::fcntl(getHandle(), F_SETFL, flg);

	m_objTerminos.c_cflag |= CREAD;
	m_objTerminos.c_cflag |= CLOCAL;
	m_objTerminos.c_cflag |= CS8;
	m_objTerminos.c_cflag |= 0;
	m_objTerminos.c_cflag |= 0;
	::cfsetispeed(&m_objTerminos, m_nBaudRate);
	::cfsetospeed(&m_objTerminos, m_nBaudRate);
	::cfmakeraw(&m_objTerminos);
	::tcsetattr(getHandle(), TCSANOW, &m_objTerminos);
	::ioctl(getHandle(), TCSETS, &m_objTerminos);

	m_pbyRxBuffer = new uint8_t[4096];
	initRxPtr();

	a_objDispatcher.add(this);
}

/**
 */
CZigBee::~CZigBee()
{
	if (getDispatcher()) {
		getDispatcher()->del(this);
	}
	closeHandle();
	delete [] m_pbyRxBuffer;
}

/**
 */
CHandle::RESULT CZigBee::onError(CIOEventDispatcher& a_objDispatcher,
		int a_nError)
{
	CDebugUtil::printf(CDebugUtil::EError, "%s: l(%d): error=%08x\n",
			__PRETTY_FUNCTION__, __LINE__, a_nError);
	return 0;
}

/**
 */
CHandle::RESULT CZigBee::onInput(CIOEventDispatcher& a_objDispatcher)
{
	int result = 0;
	uint8_t data[4];

	for (;;) {
		result = read(data, sizeof(uint8_t));
		if (result <= 0) {
			break;
		}
		if (m_bRxEscape) {
			addRxData(ZIGBEE_ESCAPE_EXCLUSIVE_VALUE ^ *data);
			m_bRxEscape = false;
		} else {
			if (isEscapeCharacter(data)) {
				m_bRxEscape = true;
				continue;
			}
			m_bRxEscape = false;
			if (*data == ZIGBEE_START_DELIMITER) {
				initRxPtr();
				addRxData(*data);
				continue;
			}
			addRxData(*data);
		}

		if (!(isRxComplete())) {
			continue;
		}

		xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(m_pbyRxBuffer);
		if (f->data.frame_type == ZIGBEE_FRAME_TYPE_RECEIVE_PACKET) {
			getListener()->onReceive(f->data.cmd_data.rx_packet_64.src_addr_64,
					f->data.cmd_data.rx_packet_64.src_addr_16, f->data.cmd_data.rx_packet_64.data,
					htons(f->header.length) - static_cast<uint16_t>(f->data.cmd_data.rx_packet_64.data - &(f->data.frame_type)));
			initRxPtr();
			break;
		}
		if (f->data.frame_type == ZIGBEE_FRAME_TYPE_TRANSMIT_STATUS) {
			if (f->data.cmd_data.tx_status.delivery_status != 0x00) {
			}
			getListener()->onTransmitResult(f->data.cmd_data.tx_status.frame_id,
					f->data.cmd_data.tx_status.delivery_status,
					f->data.cmd_data.tx_status.discovery_status);
			initRxPtr();
			break;
		}
		if (f->data.frame_type == ZIGBEE_FRAME_TYPE_AT_RESPONSE) {
			getListener()->onAtResponse(f->data.cmd_data.at_res.frame_id,
					f->data.cmd_data.at_res.status,
					f->data.cmd_data.at_res.value,
					htons(f->header.length) - 5);
			initRxPtr();
			break;
		}
		getListener()->onCommand(reinterpret_cast<uint8_t *>(f),
				htons(f->header.length) + sizeof(xbee_frame_header_t) + sizeof(uint8_t));
		initRxPtr();
		break;
	}

	if (result < 0) {
		CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): errno=%d(%s)\n",
				__PRETTY_FUNCTION__, __LINE__, errno, ::strerror(errno));
	}

	return result;
}

/**
 */
uint8_t CZigBee::calcSumCheck(void *a_pData, uint16_t a_hwLength)
{
	uint16_t sum = 0u;
	uint8_t *p = reinterpret_cast<uint8_t *>(a_pData);
	for (uint16_t i = 0u; i < a_hwLength; i++) {
		sum += p[i];
	}
	sum &= 0xffu;
	sum = 0xffu - sum;
	return sum;
}

/**
 */
uint16_t CZigBee::convateToApi2(uint8_t*& a_pbyApi2, uint8_t *a_pbyApi1, uint16_t a_hwLength)
{
	uint16_t len = 0u;
	a_pbyApi2 = new uint8_t[a_hwLength * 2];
	a_pbyApi2[0] = a_pbyApi1[0];
	a_pbyApi1++;
	uint8_t *api2 = &a_pbyApi2[1];
	for (unsigned i = 0; i < a_hwLength; i++) {
		uint16_t l = toEscape(api2, *a_pbyApi1);
		api2 += l;
		a_pbyApi1++;
		len += l;
	}
	return len;
}

/**
 */
bool CZigBee::isEscapeCharacter(uint8_t a_byApi1)
{
	for (unsigned i = 0; i < sizeof(c_byZigBeeEscapeCharacters) / sizeof(c_byZigBeeEscapeCharacters[0]); i++) {
		if (c_byZigBeeEscapeCharacters[i] == a_byApi1) {
			return true;
		}
	}
	return false;
}

/**
 */
bool CZigBee::isEscapeCharacter(const uint8_t *a_pbyApi2)
{
	return ((*a_pbyApi2) == ZIGBEE_ESCAPE_CODE);
}

/**
 */
uint16_t CZigBee::toEscape(uint8_t *a_pbyApi2, uint8_t a_byApi1)
{
	if (!isEscapeCharacter(a_byApi1)) {
		a_pbyApi2[0] = a_byApi1;
		return sizeof(uint8_t);
	}
	a_pbyApi2[0] = ZIGBEE_ESCAPE_CODE;
	a_pbyApi2[1] = ZIGBEE_ESCAPE_EXCLUSIVE_VALUE ^ a_byApi1;
	return sizeof(uint16_t);
}

/**
 */
void CZigBee::initRxPtr()
{
	xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(m_pbyRxBuffer);
	f->header.start_delimiter = 0u;
	f->header.length = 0u;
	m_pbyPoolPointer = m_pbyRxBuffer;
}

/**
 */
bool CZigBee::isRxComplete()
{
	xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(m_pbyRxBuffer);
	if (!htons(f->header.length)) {
		return false;
	}
	if (static_cast<uint32_t>(m_pbyPoolPointer - m_pbyRxBuffer) <
			(htons(f->header.length) + sizeof(xbee_frame_header_t) + sizeof(uint8_t))) {
		return false;
	}
	return true;
}

/**
 */
int CZigBee::transmit(uint8_t a_byFrameId, uint8_t a_byDstAddr64[8],
		uint16_t a_hwDstAddr16, uint8_t *a_pbyData, uint16_t a_hwLength)
{
	xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(new uint8_t[sizeof(xbee_frame_t) + a_hwLength + 16]);
	f->header.start_delimiter = ZIGBEE_START_DELIMITER;
	f->data.frame_type = ZIGBEE_FRAME_TYPE_TRANSMIT_REQUEST;
	f->header.length = htons(sizeof(f->data.frame_type) + sizeof(f->data.cmd_data.tx_request_64) + a_hwLength);
	f->data.cmd_data.tx_request_64.frame_id = a_byFrameId;
	::memcpy(f->data.cmd_data.tx_request_64.dst_addr_64, a_byDstAddr64,
			sizeof(f->data.cmd_data.tx_request_64.dst_addr_64));
	f->data.cmd_data.tx_request_64.dst_addr_16 = htons(a_hwDstAddr16);
	f->data.cmd_data.tx_request_64.broadcast_radius = 0x00u;
	f->data.cmd_data.tx_request_64.options = 0x00u;
	::memcpy(f->data.cmd_data.tx_request_64.dst_addr_64, a_byDstAddr64,
			sizeof(f->data.cmd_data.tx_request_64.dst_addr_64));
	::memcpy(f->data.cmd_data.tx_request_64.data, a_pbyData, a_hwLength);

	(reinterpret_cast<uint8_t *>(&(f->data.cmd_data.tx_request_64.data)))[a_hwLength] =
			calcSumCheck(&(f->data), htons(f->header.length));

	uint8_t *api2_data;
	uint16_t api2_len = convateToApi2(api2_data, reinterpret_cast<uint8_t *>(f),
			htons(f->header.length) + sizeof(f->header) + sizeof(uint8_t));
	delete [] reinterpret_cast<uint8_t *>(f);

	int result = write(api2_data, api2_len);
	delete [] api2_data;
	return result;
}

/**
 */
int CZigBee::atCommand(uint8_t a_byFrameId, uint8_t a_byCommand[], uint16_t a_hwLength)
{
	xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(new uint8_t[sizeof(xbee_frame_t) + a_hwLength + 16]);
	f->header.start_delimiter = ZIGBEE_START_DELIMITER;
	f->data.frame_type = ZIGBEE_FRAME_TYPE_AT_COMMAND;
	f->header.length = htons(sizeof(f->data.frame_type) + sizeof(f->data.cmd_data.at_cmd.frame_id) + a_hwLength);
	f->data.cmd_data.at_cmd.frame_id = a_byFrameId;
	::memcpy(f->data.cmd_data.at_cmd.command, a_byCommand, a_hwLength);
	(f->data.cmd_data.at_cmd.command)[a_hwLength] = calcSumCheck(&(f->data), htons(f->header.length));
	uint8_t *api2_data;
	uint16_t api2_len = convateToApi2(api2_data, reinterpret_cast<uint8_t *>(f),
			htons(f->header.length) + sizeof(f->header) + sizeof(uint8_t));
	delete [] reinterpret_cast<uint8_t *>(f);
	int result = write(api2_data, api2_len);
	delete [] api2_data;
	return result;
}
