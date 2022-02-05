
#include <peripheral_interface.h>
#include <zigbee.h>

#define ZIGBEE_ESCAPE_CODE					(0x7du)
#define ZIGBEE_ESCAPE_EXCLUSIVE_VALUE		(0x20u)
#define ZIGBEE_START_DELIMITER				(0x7eu)
#define ZIGBEE_FRAME_TYPE_AT_COMMAND		(0x08u)
#define ZIGBEE_FRAME_TYPE_AT_RESPONSE		(0x88u)
#define ZIGBEE_FRAME_TYPE_TRANSMIT_REQUEST	(0x10u)
#define ZIGBEE_FRAME_TYPE_TRANSMIT_STATUS	(0x8bu)
#define ZIGBEE_FRAME_TYPE_RECEIVE_PACKET	(0x90u)

#define CNVBYORDS(x) ((((x) & 0xff) << 8) | ((x) >> 8))

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
const uint8_t c_byEscapeCharacters[] = {
	ZIGBEE_START_DELIMITER,	// Frame delimiter
	ZIGBEE_ESCAPE_CODE,		// Escape
	0x11,					// XON
	0x13					// XOFF
};

/**
 */
void CZigBee::CRxNotify::rxNotify(uint8_t *a_pbyData, uint32_t a_unLength)
{
	if (getOwner()->getMainteInterface()) {
		getOwner()->getMainteInterface()->transmitAsync(a_pbyData, a_unLength);
		return;
	}

	// receive api2 packet and change to api1 packet then notify to client task.
	for (uint32_t i = 0; i < a_unLength; i++) {
		if (m_bRxEscape) {
			getOwner()->addRxData(ZIGBEE_ESCAPE_EXCLUSIVE_VALUE ^ a_pbyData[i]);
			m_bRxEscape = false;
		} else {
			if (getOwner()->isEscapeCharacter(&a_pbyData[i])) {
				m_bRxEscape = true;
				continue;
			}
			m_bRxEscape = false;
			if (a_pbyData[i] == ZIGBEE_START_DELIMITER) {
				getOwner()->initRxPtr();
				getOwner()->addRxData(a_pbyData[i]);
				continue;
			}
			getOwner()->addRxData(a_pbyData[i]);
		}

		if (!(getOwner()->isRxComplete())) {
			continue;
		}
		getOwner()->rxHandler();
	}
}

/**
 */
void CZigBee::CRxNotify::errNotify(uint32_t a_unErrorCode)
{
	::printf("l(%4d): %s: error=0x%08x\n", __LINE__, __PRETTY_FUNCTION__, (unsigned) a_unErrorCode);
}

/**
 */
int CZigBee::CExecTransmit::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CRequest*)) {
	}

	int result = 0;
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	if (req->m_bRaw) {
		result = m_pobjOwner->getInterface()->transmitAsync(req->m_pbyData, req->m_hwLength);
		if (req->m_bAllocate) {
			delete req;
		}
		return result;
	}

	if (m_pobjOwner->getMainteInterface()) {
//		::printf("l(%4d): %s: in maintenance mode\n", __LINE__, __PRETTY_FUNCTION__);
		if (req->m_bAllocate) {
			delete req;
		}
		return result;
	}

	xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(new uint8_t[
			_RNDUP_REMINDER(sizeof(xbee_frame_t) + req->m_hwLength, PERI_IF_BUF_ALINE)]);
	assert_param(f);
	f->header.start_delimiter = ZIGBEE_START_DELIMITER;
	f->data.frame_type = ZIGBEE_FRAME_TYPE_TRANSMIT_REQUEST;
	f->header.length = CNVBYORDS(sizeof(f->data.frame_type)
			+ sizeof(f->data.cmd_data.tx_request_64) + req->m_hwLength);
	f->data.cmd_data.tx_request_64.frame_id = req->m_byFrameId;
	::memcpy(f->data.cmd_data.tx_request_64.dst_addr_64, req->m_byDstAddr64,
			sizeof(f->data.cmd_data.tx_request_64.dst_addr_64));
	f->data.cmd_data.tx_request_64.dst_addr_16 = CNVBYORDS(req->m_hwDstAddr16);
	f->data.cmd_data.tx_request_64.broadcast_radius = 0x00u;
	f->data.cmd_data.tx_request_64.options = 0x00u;
	::memcpy(f->data.cmd_data.tx_request_64.dst_addr_64, req->m_byDstAddr64,
			sizeof(f->data.cmd_data.tx_request_64.dst_addr_64));
	::memcpy(f->data.cmd_data.tx_request_64.data, req->m_pbyData, req->m_hwLength);

	(reinterpret_cast<uint8_t *>(&(f->data.cmd_data.tx_request_64.data)))[req->m_hwLength] =
			m_pobjOwner->calcSumCheck(&(f->data), CNVBYORDS(f->header.length));

	// convert api2
	uint8_t *api2_data;
	uint16_t api2_len = m_pobjOwner->convateToApi2(api2_data, reinterpret_cast<uint8_t *>(f),
			CNVBYORDS(f->header.length) + sizeof(f->header) + sizeof(uint8_t));
	delete [] reinterpret_cast<uint8_t *>(f);
	// transmit uart
	result = m_pobjOwner->getInterface()->transmitAsync(api2_data, api2_len);
	delete [] api2_data;

	if (req->m_bAllocate) {
		delete req;
	}

	return result;
}

/**
 */
int CZigBee::CExecAtCommand::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (m_pobjOwner->getMainteInterface()) {
		return 0;
	}

	if (a_cnLength != sizeof(CRequest)) {
	}

	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(new uint8_t[sizeof(xbee_frame_t) + req->m_hwLength + 16]);
	assert_param(f);
	f->header.start_delimiter = ZIGBEE_START_DELIMITER;
	f->data.frame_type = ZIGBEE_FRAME_TYPE_AT_COMMAND;
	f->header.length = CNVBYORDS(sizeof(f->data.frame_type) + sizeof(f->data.cmd_data.at_cmd.frame_id) + req->m_hwLength);
	f->data.cmd_data.at_cmd.frame_id = req->m_byFrameId;
	::memcpy(f->data.cmd_data.at_cmd.command, req->m_pbyData, req->m_hwLength);
	(reinterpret_cast<uint8_t *>(&(f->data.cmd_data.at_cmd.command)))[req->m_hwLength] =
			m_pobjOwner->calcSumCheck(&(f->data), CNVBYORDS(f->header.length));
	uint8_t *api2_data;
	uint16_t api2_len = m_pobjOwner->convateToApi2(api2_data, reinterpret_cast<uint8_t *>(f),
			CNVBYORDS(f->header.length) + sizeof(f->header) + sizeof(uint8_t));
	delete [] reinterpret_cast<uint8_t *>(f);

	int result = m_pobjOwner->getInterface()->transmitAsync(api2_data, api2_len);
	delete [] api2_data;
	if (req->m_bAllocate) {
		delete req;
	}
	return result;
}

/**
 */
int CZigBee::CExecSetMainteMode::doExec(const void *a_lpParam,
		const int a_cnLength)
{
	if (a_cnLength != sizeof(CUart *)) {
	}
	m_pobjOwner->setMainteInterface(
			reinterpret_cast<CUart *>(const_cast<void *>(a_lpParam)));
	return 0;
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
uint16_t CZigBee::convateToApi2(uint8_t*& a_pbyApi2, uint8_t *a_pbyApi1,
		uint16_t a_hwLength)
{
	uint16_t len = 0u;
	a_pbyApi2 = new uint8_t[a_hwLength * 2];
	assert_param(a_pbyApi2);
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

#if 0
/**
 */
uint16_t CZigBee::convateFromApi2(uint8_t*& a_pbyApi1, uint8_t *a_pbyApi2,
		uint16_t a_hwLength)
{
	uint16_t len = 0u;
	a_pbyApi1 = new uint8_t[a_hwLength];
	assert_param(a_pbyApi1);
	a_pbyApi1[0] = a_pbyApi2[0];
	a_pbyApi2++;
	uint8_t *api1 = &a_pbyApi1[1];
	for (unsigned i = 0; i < a_hwLength;) {
		uint16_t l = fromEscape(*api1, a_pbyApi2);
		i += l;
		a_pbyApi2 += l;
		api1++;
		len++;
	}
	return len;
}
#endif

/**
 */
bool CZigBee::isEscapeCharacter(uint8_t a_byApi1)
{
	for (unsigned i = 0; i < sizeof(c_byEscapeCharacters)
			/ sizeof(c_byEscapeCharacters[0]); i++) {
		if (c_byEscapeCharacters[i] == a_byApi1) {
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

#if 0
/**
 */
uint16_t CZigBee::fromEscape(uint8_t& a_byApi1, const uint8_t *a_pbyApi2)
{
	if (!isEscapeCharacter(a_pbyApi2)) {
		a_byApi1 = *a_pbyApi2;
		return sizeof(uint8_t);
	}
	a_byApi1 = ZIGBEE_ESCAPE_EXCLUSIVE_VALUE ^ a_pbyApi2[1];
	return sizeof(uint16_t);
}
#endif

/**
 */
CZigBee::CZigBee(CRfEventListener *a_pobjEventListener)
	:	CHandlerBase(~0u, 512u, 256u, 6u, NULL, true), m_pobjRxNotify(NULL),
			m_pobjUart(NULL), m_pobjEventListener(a_pobjEventListener),
			m_pobjExecTransmit(NULL), m_pobjExecAtCommand(NULL),
			m_pobjExecSetMainteMode(NULL), m_pobjTxComplete(NULL),
			m_pobjMainteUart(NULL)
{
	m_pobjTxComplete = new CThreadInterfaceBase::CLock(false);
	m_pobjRxNotify = new CRxNotify(this);
	m_pbyRxBuffer = new uint8_t[256];
	m_pbyPoolPointer = m_pbyRxBuffer;
	m_pobjExecTransmit = new CExecTransmit(this);
	m_pobjExecAtCommand = new CExecAtCommand(this);
	m_pobjExecSetMainteMode = new CExecSetMainteMode(this);
	assert_param(m_pobjTxComplete);
	assert_param(m_pobjRxNotify);
	assert_param(m_pbyRxBuffer);
	assert_param(m_pobjExecTransmit);
	assert_param(m_pobjExecAtCommand);
	assert_param(m_pobjExecSetMainteMode);
	waitStartUp();
}

/**
 */
CZigBee::~CZigBee()
{
	delete m_pobjExecSetMainteMode;
	delete m_pobjExecAtCommand;
	delete m_pobjExecTransmit;
	delete m_pobjRxNotify;
	delete [] m_pbyRxBuffer;
	delete m_pobjTxComplete;
}

/**
 */
int CZigBee::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	(void) registerExec(ERequestTransmit, m_pobjExecTransmit);
	(void) registerExec(ERequestAtCommand, m_pobjExecAtCommand);
	(void) registerExec(ERequestSetMainteMode, m_pobjExecSetMainteMode);

	return CThreadInterfaceBase::onInitialize();
}

/**
 */
int CZigBee::transmit(uint8_t a_byFrameId, uint8_t a_byDstAddr64[],
		uint16_t a_hwDstAddr16, uint8_t *a_pbyData, uint16_t a_hwLength)
{
	CRequest *req = new CRequest(a_byFrameId, a_byDstAddr64, a_hwDstAddr16,
			a_pbyData, a_hwLength, false);
	assert_param(req);
	if (!req) {
		return -1;
	}
	int rc = requestSync(ERequestTransmit, &req, sizeof(req));
	delete req;
	return rc;
}

/**
 */
int CZigBee::transmitAsync(uint8_t a_byFrameId, uint8_t a_byDstAddr64[],
		uint16_t a_hwDstAddr16, uint8_t *a_pbyData, uint16_t a_hwLength)
{
	CRequest *req = new CRequest(a_byFrameId, a_byDstAddr64, a_hwDstAddr16,
			a_pbyData, a_hwLength, true);
	assert_param(req);
	if (!req) {
		return -1;
	}
	return requestAsync(ERequestTransmit, &req, sizeof(req));
}

/**
 */
int CZigBee::transmitToCoodinater(uint8_t a_byFrameId, uint8_t *a_byData,
		uint16_t a_hwLength, bool a_bAsync)
{
	uint8_t adr64[8] = {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};
	uint16_t adr16 = 0xfffeu;
	if (!a_bAsync) {
		return transmit(a_byFrameId, adr64, adr16, a_byData, a_hwLength);
	}
	return transmitAsync(a_byFrameId, adr64, adr16, a_byData, a_hwLength);
}

/**
 */
int CZigBee::atCommand(uint8_t a_byFrameId, uint8_t a_byCommand[],
		uint16_t a_hwLength)
{
	CRequest *req = new CRequest(a_byFrameId, NULL, 0u, a_byCommand,
			a_hwLength, false);
	assert_param(req);
	if (!req) {
		return -1;
	}
	int rc = requestSync(ERequestAtCommand, &req, sizeof(req));
	delete req;
	return rc;
}

/**
 */
int CZigBee::setMainteMode(CUart *a_pobjUart)
{
	return requestSync(ERequestSetMainteMode, a_pobjUart, sizeof(void *));
}

/**
 */
int CZigBee::transmitRaw(uint8_t *a_pbyData, uint16_t a_hwLength)
{
	CRequest *req = new CRequest(a_pbyData, a_hwLength, true);
	assert_param(req);
	if (!req) {
		return -1;
	}
	return requestAsync(ERequestTransmit, &req, sizeof(req));
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
	if (!CNVBYORDS(f->header.length)) {
		return false;
	}
	if (static_cast<uint32_t>(m_pbyPoolPointer - m_pbyRxBuffer) <
			(CNVBYORDS(f->header.length) + sizeof(xbee_frame_header_t)
					+ sizeof(uint8_t))) {
		return false;
	}
	return true;
}

/**
 */
void CZigBee::rxHandler()
{
	// if analise api1 packet are receive packet then notify to client task.
	xbee_frame_t *f = reinterpret_cast<xbee_frame_t *>(m_pbyRxBuffer);
	if (f->data.frame_type == ZIGBEE_FRAME_TYPE_RECEIVE_PACKET) {
		getListener()->onReceive(f->data.cmd_data.rx_packet_64.src_addr_64,
				f->data.cmd_data.rx_packet_64.src_addr_16,
				f->data.cmd_data.rx_packet_64.data,
				CNVBYORDS(f->header.length) - static_cast<uint16_t>(
						f->data.cmd_data.rx_packet_64.data - &(f->data.frame_type)));
		initRxPtr();
		return;
	}
	if (f->data.frame_type == ZIGBEE_FRAME_TYPE_TRANSMIT_STATUS) {
		getListener()->onTransmitResult(f->data.cmd_data.tx_status.frame_id,
				f->data.cmd_data.tx_status.delivery_status,
				f->data.cmd_data.tx_status.discovery_status);
		initRxPtr();
		return;
	}
	if (f->data.frame_type == ZIGBEE_FRAME_TYPE_AT_RESPONSE) {
		getListener()->onAtResponse(f->data.cmd_data.at_res.frame_id,
				f->data.cmd_data.at_res.status,
				f->data.cmd_data.at_res.value,
				CNVBYORDS(f->header.length) - 5);
		initRxPtr();
		return;
	}
	getListener()->onCommand(reinterpret_cast<uint8_t *>(f),
			CNVBYORDS(f->header.length) + sizeof(xbee_frame_header_t)
					+ sizeof(uint8_t));
	initRxPtr();
}
