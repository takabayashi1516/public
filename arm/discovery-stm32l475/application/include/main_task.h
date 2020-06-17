/**
 *
 */

#ifndef __FACTORY_H
#define __FACTORY_H

#include <peripheral_interface.h>
#include <bme280.h>
#include <zigbee.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	void EXTI0_IRQHandler(void);
	void EXTI15_10_IRQHandler(void);
	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
	int startMain(void);
	void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);
	void __attribute__((noreturn)) _exit(int code);
	void __attribute__((noreturn)) abort(void);
	void __attribute__((noreturn)) wrap_abort(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */


#ifdef __cplusplus

class CMain;
class CAiLevelSens;

/**
 *
 */
class CEdgeObserver : public CZigBee::CRfEventListener {
public:
	///
	CEdgeObserver(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
	///
	virtual ~CEdgeObserver() {}
	///
	virtual void onReceive(uint8_t a_byDstAddr64[8], uint16_t a_hwDstAddr16,
			uint8_t *a_pbyData, uint16_t a_hwLength);
	///
	virtual void onTransmitResult(uint8_t a_byFrameId,
			uint8_t a_byDeliveryStatus, uint8_t a_byDiscoveryStatus);
	///
	virtual void onAtResponse(uint8_t a_byFrameId, uint8_t a_byStatus,
			uint8_t *a_pbyValue, uint16_t a_hwLength);
	///
	virtual void onCommand(uint8_t *a_pbyCommand, uint16_t a_hwLength);

private:
	CMain *getOwner() const {
		return m_pobjOwner;
	}

private:
	CMain	*m_pobjOwner;
};

/**
 *
 */
class CSpiRxHandler : public CSpiBus::CHandler {
public:
	///
	CSpiRxHandler(uint32_t a_unBus, CMain *a_pobjOwner)
		: m_unBus(a_unBus), m_pobjOwner(a_pobjOwner) {}
	///
	virtual void rxNotify(uint8_t *a_pbyData, uint32_t a_unLength);

private:
	///
	uint32_t	m_unBus;
	///
	CMain		*m_pobjOwner;
};

/**
 *
 */
class CUartRxHandler : public CUart::CHandler {
public:
	///
	CUartRxHandler(uint32_t a_unChannel, CMain *a_pobjOwner)
		: m_unChannel(a_unChannel), m_pobjOwner(a_pobjOwner) {}
	///
	virtual void rxNotify(uint8_t *a_pbyData, uint32_t a_unLength);

private:
	///
	uint32_t	m_unChannel;
	///
	CMain		*m_pobjOwner;
};

/**
 *
 */
class CUsbCdcAcmRxHandler : public CUsbCdcAcm::CHandler {
public:
	///
	CUsbCdcAcmRxHandler(CMain *a_pobjOwner)
		: m_pobjOwner(a_pobjOwner) {}
	///
	virtual void rxNotify(uint8_t *a_pbyData, uint32_t a_unLength);

private:
	///
	CMain		*m_pobjOwner;
};

/**
 *
 */
class CMain : public CHandlerBase {
public:
	///
	enum ERequest {
		ERequestZigBeeReceive	= 10,
		ERequestZigBeeTransmitStatus,
		ERequestZigBeeAtResponse,
		ERequestUartReceive		= 20,
		ERequestCdcAcmReceive	= 30,
		ERequestSpiReceive		= 40,
		ERequestInputSwitch		= 100,
	};

	///
	enum ECommStatus {
		ECommStatusDisconnected	= 0,
		ECommStatusConnected,
		ECommStatusMaintenance	= 10,
	};

public:
	/**
	 */
	class CExecZigBeeReceive : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint8_t *a_pbyData, uint32_t a_wLength)
				: m_pbyData(NULL), m_wLength(a_wLength) {
				int l = _RNDUP_REMINDER(m_wLength, PERI_IF_BUF_ALINE);
				m_pbyData = new uint8_t[l];
				assert_param(m_pbyData);
				::memset(m_pbyData, 0, l);
				::memcpy(m_pbyData, a_pbyData, m_wLength);
			}
			///
			virtual ~CRequest() {
				delete [] m_pbyData;
			}
		public:
			///
			uint8_t		*m_pbyData;
			///
			uint32_t	m_wLength;
		};

	public:
		///
		CExecZigBeeReceive(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CMain	*m_pobjOwner;
	};

	/**
	 */
	class CExecZigBeeTransmitStatus : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint8_t a_byFrameId, uint8_t a_byDeliveryStatus,
					uint8_t a_byDiscoveryStatus)
				: m_byFrameId(a_byFrameId), m_byDeliveryStatus(a_byDeliveryStatus),
						m_byDiscoveryStatus(a_byDiscoveryStatus) {}
			///
			virtual ~CRequest() {}
		public:
			///
			uint8_t	m_byFrameId;
			///
			uint8_t	m_byDeliveryStatus;
			///
			uint8_t	m_byDiscoveryStatus;
		};

	public:
		///
		CExecZigBeeTransmitStatus(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CMain	*m_pobjOwner;
	};

	/**
	 */
	class CExecZigBeeAtResponse : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint8_t a_byFrameId, uint8_t a_byStatus,
					uint16_t a_hwLength, uint8_t a_byValue[])
				: m_byFrameId(a_byFrameId), m_byStatus(a_byStatus),
						m_hwLength(a_hwLength), m_pbyValue(NULL) {
				if (!a_byValue) {
					return;
				}
				uint16_t l = _RNDUP_REMINDER(m_hwLength, PERI_IF_BUF_ALINE);
				m_pbyValue = new uint8_t[l];
				assert_param(m_pbyValue);
				::memset(m_pbyValue, 0, l);
				::memcpy(m_pbyValue, a_byValue, m_hwLength);
			}
			///
			virtual ~CRequest() {
				if (m_pbyValue) {
					delete [] m_pbyValue;
				}
			}
		public:
			///
			uint8_t		m_byFrameId;
			///
			uint8_t		m_byStatus;
			///
			uint16_t	m_hwLength;
			///
			uint8_t		*m_pbyValue;
		};

	public:
		///
		CExecZigBeeAtResponse(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CMain	*m_pobjOwner;
	};

	/**
	 */
	class CExecSpiReceive : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint32_t a_unBus, uint8_t *a_pbyData, uint32_t a_wLength)
				: m_unBus(a_unBus), m_pbyData(NULL), m_wLength(a_wLength) {
				int l = _RNDUP_REMINDER(m_wLength, PERI_IF_BUF_ALINE);
				m_pbyData = new uint8_t[l];
				assert_param(m_pbyData);
				::memset(m_pbyData, 0, l);
				::memcpy(m_pbyData, a_pbyData, m_wLength);
			}
			///
			virtual ~CRequest() {
				delete [] m_pbyData;
			}
		public:
			///
			uint32_t	m_unBus;
			///
			uint8_t		*m_pbyData;
			///
			uint32_t	m_wLength;
		};

	public:
		///
		CExecSpiReceive(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CMain	*m_pobjOwner;
	};

	/**
	 */
	class CExecUartReceive : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint32_t a_unChannel, uint8_t *a_pbyData, uint32_t a_wLength)
				: m_unChannel(a_unChannel), m_pbyData(NULL), m_wLength(a_wLength) {
				int l = _RNDUP_REMINDER(m_wLength, PERI_IF_BUF_ALINE);
				m_pbyData = new uint8_t[l];
				assert_param(m_pbyData);
				::memset(m_pbyData, 0, l);
				::memcpy(m_pbyData, a_pbyData, m_wLength);
			}
			///
			virtual ~CRequest() {
				delete [] m_pbyData;
			}
		public:
			///
			uint32_t	m_unChannel;
			///
			uint8_t		*m_pbyData;
			///
			uint32_t	m_wLength;
		};

	public:
		///
		CExecUartReceive(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CMain	*m_pobjOwner;
	};

	/**
	 */
	class CExecCdcAcmReceive : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint8_t *a_pbyData, uint32_t a_wLength)
				: m_pbyData(NULL), m_wLength(a_wLength) {
				int l = _RNDUP_REMINDER(m_wLength, PERI_IF_BUF_ALINE);
				m_pbyData = new uint8_t[l];
				assert_param(m_pbyData);
				::memset(m_pbyData, 0, l);
				::memcpy(m_pbyData, a_pbyData, m_wLength);
			}
			///
			virtual ~CRequest() {
				delete [] m_pbyData;
			}
		public:
			///
			uint8_t		*m_pbyData;
			///
			uint32_t	m_wLength;
		};

	public:
		///
		CExecCdcAcmReceive(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner),
				m_hwPoolLength(0u) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CMain		*m_pobjOwner;
	private:
		///
		uint16_t	m_hwPoolLength;
		///
		uint8_t		m_byBuffer[256];
	};
	/**
	 */
	class CExecInputSwitch : public CHandlerBase::CExecBase {
	public:
		///
		CExecInputSwitch(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	private:
		///
		CMain		*m_pobjOwner;
	};

	/**
	 */
	class CObserver : public CThreadFrame::CObserver {
	public:
		///
		CObserver(CMain *a_pobjOwner);
		///
		void onPreviousProcess(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest);
		///
		void onCompleted(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest, const int a_cnResult);
		///
		void onCanceled(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest);
	private:
		///
		CMain	*m_pobjOwner;
	};

public:
	///
	virtual ~CMain();
	///
	virtual int onInitialize();
	///
	virtual int	onTerminate();
	///
	virtual int	onTimeout();

	///
	CI2cBus *getI2c(int bus) const;
	///
	CUart *getUart(int ch) const;
	///
	CSpiBus *getSpi(int bus) const;
	///
	CUsbCdcAcm *getUsbCdcAcm() const;
	///
	CZigBee *getZigBee() const;
	///
	CAiLevelSens *getAiLevelSens() const {
		return m_pobjAiLevelSens;
	}

	///
	void setDataFromHost(uint32_t a_wIndex, uint32_t a_wData) {
		m_wDataFromHost[a_wIndex] = a_wData;
	}
	///
	uint32_t getDataToHost(uint32_t a_wIndex) const {
		return m_wDataToHost[a_wIndex];
	}
	///
	ECommStatus getCommStatus() const {
		return m_nCommStatus;
	}
	///
	void setCommStatus(ECommStatus a_nStatus) {
		m_nCommStatus = a_nStatus;
	}
	///
	CInterruptRequestManager *getIntRqMngrSwtch() const {
		return m_pobjIntRqMngrSwtch;
	}
	///
	void interruptSwitch(uint32_t a_unSwitch);
	///
	void postDataToEdge();

	///
	static CMain *getSingleton();

private:
	///
	CMain();

	///
	uint32_t getDataFromHost(uint32_t a_wIndex) const {
		return m_wDataFromHost[a_wIndex];
	}
	///
	void makeDataToEdge(uint32_t a_wIndex, uint32_t a_wData) {
		m_wDataToHost[a_wIndex] = a_wData;
	}

private:
	///
	CEdgeObserver	*m_pobjEdgeObserver;
	///
	CZigBee			*m_pobjZigBee;
	///
	CI2cBus			*m_pobjI2c1;
	///
	CI2cBus			*m_pobjI2c2;
	///
	CUart			*m_pobjUart2;
	///
	CUart			*m_pobjUart4;
	///
	CSpiBus			*m_pobjSpi1;
	///
	CSpiBus			*m_pobjSpi2;
	///
	CSpiBus			*m_pobjSpi3;
	///
	CExecZigBeeReceive	*m_pobjExecZigBeeReceive;
	///
	CExecZigBeeTransmitStatus	*m_pobjExecZigBeeTransmitStatus;
	///
	CExecZigBeeAtResponse	*m_pobjExecZigBeeAtResponse;
	///
	CExecUartReceive	*m_pobjExecUartReceive;
	///
	CExecCdcAcmReceive	*m_pobjExecCdcAcmReceive;
	///
	CExecSpiReceive		*m_pobjExecSpiReceive;
	///
	CExecInputSwitch	*m_pobjExecInputSwitch;
	///
	CInterruptRequestManager	*m_pobjIntRqMngrSwtch;
	///
	CAiLevelSens	*m_pobjAiLevelSens;
	///
	CObserver		m_objObserver;
	///
	ECommStatus		m_nCommStatus;
	///
	bool			m_bMainteLedStatus;
	///
	uint32_t		m_wDataToHost[8];
	///
	uint32_t		m_wDataFromHost[4];

	///
	static CMain	*m_pobjMain;
};

/**
 *
 */
class CAiLevelSens : public CAnalogInput {
public:
	///
	CAiLevelSens(CMain *a_objOwner, const ADC_TypeDef *a_cpobjAdc,
			const uint32_t a_cnChannel, const uint32_t a_cnTimeout,
			const uint32_t a_cnAdcValueThreshold);
	///
	virtual int onTimeout();

private:
	CMain	*m_objOwner;
	const uint32_t m_cnAdcValueThreshold;
};

#endif /* __cplusplus */

#endif /* __FACTORY_H */



