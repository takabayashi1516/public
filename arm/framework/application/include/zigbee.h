/**
 *
 */
#ifndef __ZIGBEE_H
#define __ZIGBEE_H

#include <stddef.h>
#include <stdint.h> /* READ COMMENT ABOVE. */
#include <framework.h>

#ifdef __cplusplus

class CUart;

/**
 */
class CZigBee : public CHandlerBase {
public:
	///
	enum ERequest {
		ERequestTransmit = 100,
		ERequestAtCommand,
		ERequestSetMainteMode,
	};

public:
	/**
	 */
	class CRxNotify : public CUart::CHandler {
	public:
		///
		CRxNotify(CZigBee *a_pobjOwner) : m_pobjOwner(a_pobjOwner), m_bRxEscape(false) {}
		///
		virtual void rxNotify(uint8_t *a_pbyData, uint32_t a_unLength);
		///
		virtual void errNotify(uint32_t a_unErrorCode);

	private:
		CZigBee *getOwner() const {
			return m_pobjOwner;
		}

	private:
		///
		CZigBee	*m_pobjOwner;
		///
		bool	m_bRxEscape;
	};

	/**
	 */
	class CRfEventListener {
	public:
		///
		virtual void onReceive(uint8_t a_byDstAddr64[8], uint16_t a_hwDstAddr16,
				uint8_t *a_pbyData, uint16_t a_hwLength) = 0;
		///
		virtual void onTransmitResult(uint8_t a_byFrameId,
				uint8_t a_byDeliveryStatus, uint8_t a_byDiscoveryStatus) = 0;
		///
		virtual void onAtResponse(uint8_t a_byFrameId, uint8_t a_byStatus,
				uint8_t *a_pbyValue, uint16_t a_hwLength) = 0;
		///
		virtual void onCommand(uint8_t *a_pbyCommand, uint16_t a_hwLength) = 0;
	};

private:
	/**
	 */
	class CRequest {
	public:
		///
		CRequest(uint8_t a_byFrameId, uint8_t a_byDstAddr64[],
				uint16_t a_hwDstAddr16, uint8_t *a_pbyData,
				uint16_t a_hwLength, bool a_bAllocate)
			: m_byFrameId(a_byFrameId), m_hwDstAddr16(a_hwDstAddr16),
					m_pbyData(a_pbyData), m_hwLength(a_hwLength),
					m_bAllocate(a_bAllocate), m_bRaw(false) {
			if (a_byDstAddr64) {
				::memcpy(m_byDstAddr64, a_byDstAddr64, sizeof(m_byDstAddr64));
			}
			if (!m_bAllocate) {
				return;
			}
			uint16_t l = _RNDUP_REMINDER(m_hwLength, PERI_IF_BUF_ALINE);
			m_pbyData = new uint8_t[l];
			assert_param(m_pbyData);
			::memset(m_pbyData, 0, l);
			::memcpy(m_pbyData, a_pbyData, m_hwLength);
		}
		///
		CRequest(uint8_t *a_pbyData, uint16_t a_hwLength, bool a_bAllocate)
			: m_byFrameId(0), m_hwDstAddr16(0), m_pbyData(a_pbyData),
					m_hwLength(a_hwLength), m_bAllocate(a_bAllocate), m_bRaw(true) {
			::memset(m_byDstAddr64, 0, sizeof(m_byDstAddr64));
			if (!m_bAllocate) {
				return;
			}
			uint16_t l = _RNDUP_REMINDER(m_hwLength, PERI_IF_BUF_ALINE);
			m_pbyData = new uint8_t[l];
			assert_param(m_pbyData);
			::memset(m_pbyData, 0, l);
			::memcpy(m_pbyData, a_pbyData, m_hwLength);
		}
		///
		virtual ~CRequest() {
			if (!m_bAllocate || !m_pbyData) {
				return;
			}
			delete [] m_pbyData;
		}
	public:
		///
		uint8_t		m_byFrameId;
		///
		uint8_t		m_byDstAddr64[8];
		///
		uint16_t	m_hwDstAddr16;
		///
		uint8_t		*m_pbyData;
		///
		uint16_t	m_hwLength;
		///
		bool		m_bAllocate;
		///
		bool		m_bRaw;
	};

	/**
	 */
	class CExecTransmit : public CHandlerBase::CExecBase {
	public:
		///
		CExecTransmit(CZigBee *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CZigBee	*m_pobjOwner;
	};

	/**
	 */
	class CExecAtCommand : public CHandlerBase::CExecBase {
	public:
		///
		CExecAtCommand(CZigBee *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CZigBee	*m_pobjOwner;
	};

	/**
	 */
	class CExecSetMainteMode : public CHandlerBase::CExecBase {
	public:
		///
		CExecSetMainteMode(CZigBee *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CZigBee	*m_pobjOwner;
	};

public:
	///
	CZigBee(CRfEventListener *a_pobjEventListener);
	///
	virtual ~CZigBee();

	///
	virtual int onInitialize();

	///
	int transmit(uint8_t a_byFrameId, uint8_t a_byDstAddr64[8],
			uint16_t a_hwDstAddr16, uint8_t *a_pbyData, uint16_t a_hwLength);
	///
	int transmitAsync(uint8_t a_byFrameId, uint8_t a_byDstAddr64[8],
			uint16_t a_hwDstAddr16, uint8_t *a_pbyData, uint16_t a_hwLength);
	///
	int transmitToCoodinater(uint8_t a_byFrameId, uint8_t *a_byData,
			uint16_t a_hwLength, bool a_bAsync = true);
	///
	int atCommand(uint8_t a_byFrameId, uint8_t a_byCommand[],
			uint16_t a_hwLength);
	///
	int setMainteMode(CUart *a_pobjUart);
	///
	int transmitRaw(uint8_t *a_pbyData, uint16_t a_hwLength);

	///
	void setInterface(CUart *a_pobjUart) {
		m_pobjUart = a_pobjUart;
	}
	///
	CUart *getInterface() const {
		return m_pobjUart;
	}
	///
	CRxNotify *getInterfaceListener() const {
		return m_pobjRxNotify;
	}
	///
	void initRxPtr();
	///
	void addRxData(uint8_t a_byData) {
		*m_pbyPoolPointer = a_byData;
		m_pbyPoolPointer++;
	}
	///
	bool isRxComplete();
	///
	void rxHandler();
	///
	void setMainteInterface(const CUart *a_pobjMainteUart) {
		m_pobjMainteUart = const_cast<CUart *>(a_pobjMainteUart);
	}
	///
	CUart *getMainteInterface() const {
		return m_pobjMainteUart;
	}

private:
	///
	CRfEventListener *getListener() const {
		return m_pobjEventListener;
	}
	///
	uint8_t calcSumCheck(void *a_pData, uint16_t a_hwLength);
	///
	uint16_t convateToApi2(uint8_t*& a_pbyApi2, uint8_t *a_pbyApi1, uint16_t a_hwLength);
#if 0
	///
	uint16_t convateFromApi2(uint8_t*& a_pbyApi1, uint8_t *a_pbyApi2, uint16_t a_hwLength);
#endif
	///
	bool isEscapeCharacter(uint8_t a_byApi1);
	///
	bool isEscapeCharacter(const uint8_t *a_pbyApi2);
	///
	uint16_t toEscape(uint8_t *a_pbyApi2, uint8_t a_byApi1);
#if 0
	///
	uint16_t fromEscape(uint8_t& a_byApi1, const uint8_t *a_pbyApi2);
#endif

private:
	///
	CRxNotify			*m_pobjRxNotify;
	///
	CUart				*m_pobjUart;
	///
	CRfEventListener	*m_pobjEventListener;
	///
	uint8_t				*m_pbyRxBuffer;
	///
	uint8_t				*m_pbyPoolPointer;
	///
	CExecTransmit		*m_pobjExecTransmit;
	///
	CExecAtCommand		*m_pobjExecAtCommand;
	///
	CExecSetMainteMode	*m_pobjExecSetMainteMode;
	///
	CThreadInterfaceBase::CLock	*m_pobjTxComplete;
	///
	CUart				*m_pobjMainteUart;
};

#endif /* __cplusplus */

#endif /* __ZIGBEE_H */
