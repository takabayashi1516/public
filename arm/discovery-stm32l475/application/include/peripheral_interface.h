/**
 *
 */

#ifndef __PERIPHERAL_INTERFACE_H
#define __PERIPHERAL_INTERFACE_H

#include <stddef.h>
#include <stdint.h> /* READ COMMENT ABOVE. */
#include <stm32l4xx.h>
#include <stm32l4xx_hal_def.h>
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_i2c.h>
#include <stm32l4xx_hal_uart.h>

#include <usbd_def.h>

#include <framework.h>

#define PERI_IF_BUF_ALINE (4)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	void USART1_IRQHandler(void);
	void USART2_IRQHandler(void);
	void USART3_IRQHandler(void);
	void UART4_IRQHandler(void);
	void SPI1_IRQHandler(void);
	void SPI2_IRQHandler(void);
	void SPI3_IRQHandler(void);
	void Apl_UART_RxCallback(UART_HandleTypeDef *huart);
	void OTG_FS_IRQHandler(void);
	void Apl_CDC_RxCallback(USBD_HandleTypeDef *husb);
	void Apl_CDC_TxCmpltCallback(USBD_HandleTypeDef *husb);
	void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
	void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
	void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus

/**
 *
 */
class CRingBuffComm : public CRingBuffer {
public:
	///
	CRingBuffComm(const IRQn_Type a_cnIrq, const uint32_t a_cunSize)
		: CRingBuffer(a_cunSize), m_cnIrq(a_cnIrq) {}
	///
	virtual ~CRingBuffComm() {}

protected:
	///
	virtual void onRequireLock();
	///
	virtual void onRequireUnLock();

private:
	///
	const IRQn_Type	m_cnIrq;
};

/**
 *
 */
class CI2cBus : public CHandlerBase {
public:
	///
	enum ERequest {
		ERequestWrite = 100,
		ERequestWriteRead
	};

public:
	/**
	 *
	 */
	class CRequest {
	public:
		///
		CRequest();
	public:
		///
		uint16_t	m_hwSla;
		///
		uint16_t	m_unDataLength;
		///
		uint8_t		*m_pbyData;
		///
		uint16_t	m_unBufferLength;
		///
		uint8_t		*m_pbyBuffer;
	};

	/**
	 *
	 */
	class CExecWrite : public CHandlerBase::CExecBase {
	public:
		///
		CExecWrite(CI2cBus *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CI2cBus	*m_pobjOwner;
	};

	/**
	 *
	 */
	class CExecWriteRead : public CHandlerBase::CExecBase {
	public:
		///
		CExecWriteRead(CI2cBus *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CI2cBus	*m_pobjOwner;
	};

public:
	///
	CI2cBus(I2C_TypeDef *a_pobjDevice);
	///
	virtual ~CI2cBus();
	///
	int onInitialize();
	///
	int onTerminate();

	///
	int transmit(uint8_t a_bySla, uint8_t *a_pbyData, uint32_t a_unLength);
	///
	int receive(uint8_t a_bySla, uint8_t *a_pbyData, uint32_t a_unDataLength, uint8_t *a_pbyBuffer, uint32_t a_unBufferLength);

	///
	I2C_TypeDef *getDevice() const {
		return m_pobjDevice;
	}
	///
	I2C_HandleTypeDef *getHandle() const {
		return const_cast<I2C_HandleTypeDef *>(m_pobjHandle);
	}

private:
	I2C_TypeDef			*m_pobjDevice;
	I2C_HandleTypeDef	*m_pobjHandle;
	CExecWrite			m_objExecWrite;
	CExecWriteRead		m_objExecWriteRead;
};

/**
 *
 */
class CSpiBus : public CHandlerBase {
public:
	///
	enum ERequest {
		ERequestWrite = 100,
		ERequestWriteRead,
		ERequestRead
	};
	///
	enum EBus {
		EBus1 = 0,
		EBus2,
		EBus3,
		EBusSupremum
	};
	///
	enum EChannel {
		EChannel0 = 0,
		EChannel1,	/* rfu */
		EChannelSupremum,
		EChannelSlave = 0x0100
	};
	///
	enum EMode {
		EModeMaster = 0,
		EModeSlave
	};
	///
	enum ESsCntrlMode {
		ESsCntrlModeHardware = 0,
		ESsCntrlModeSoftware
	};

public:
	/**
	 *
	 */
	class CSlaveBuffer : public CTripleBuffer {
	public:
		CSlaveBuffer(CSpiBus *a_pobjOwner, uint32_t a_unSlaveBuffeSize);
		~CSlaveBuffer();
	protected:
		virtual void onRequireLock();
		virtual void onRequireUnLock();
	private:
		CSpiBus	*m_pobjOwner;
	};
	/**
	 *
	 */
	class CHandler {
	public:
		///
		CHandler();
		///
		virtual ~CHandler();
		///
		virtual void rxNotify(uint8_t *a_pbyData, uint32_t a_unLength) = 0;
	};

	/**
	 *
	 */
	class CRequest {
	public:
		///
		CRequest();
	public:
		///
		EChannel	m_nChannel;
		///
		uint32_t	m_unLength;
		///
		uint8_t		*m_pbyData;
		///
		uint8_t		*m_pbyBuffer;
		///
		bool		m_bAlloc;
	};

	/**
	 *
	 */
	class CExecWrite : public CHandlerBase::CExecBase {
	public:
		///
		CExecWrite(CSpiBus *a_pobjOwner) : m_pobjOwner(a_pobjOwner),
				m_pobjSemaphore(new CSemaphore()) {}
		///
		virtual ~CExecWrite() {
			delete m_pobjSemaphore;
		}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
		///
		CSemaphore *getSemaphore() const {
			return m_pobjSemaphore;
		}
	private:
		CSpiBus		*m_pobjOwner;
		CSemaphore	*m_pobjSemaphore;
	};

	/**
	 *
	 */
	class CExecWriteRead : public CHandlerBase::CExecBase {
	public:
		///
		CExecWriteRead(CSpiBus *a_pobjOwner) : m_pobjOwner(a_pobjOwner),
				m_pobjSemaphore(new CSemaphore()) {}
		virtual ~CExecWriteRead() {
			delete m_pobjSemaphore;
		}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
		///
		CSemaphore *getSemaphore() const {
			return m_pobjSemaphore;
		}
	private:
		CSpiBus		*m_pobjOwner;
		CSemaphore	*m_pobjSemaphore;
	};

public:
	///
	CSpiBus(SPI_TypeDef *a_pobjDevice, EMode a_nMode = EModeMaster, ESsCntrlMode a_nSsCntrlMode = ESsCntrlModeHardware,
			CHandler *a_pobjHandler = NULL, uint32_t a_unSlaveBuffeSize = 0u);
	///
	virtual ~CSpiBus();
	///
	int onInitialize();
	///
	int onTerminate();

	///
	int transmit(EChannel a_nChannel, uint8_t *a_pbyData, uint32_t a_unLength,
			bool a_bAsync = false);
	///
	int transmitReceive(EChannel a_nChannel, uint8_t *a_pbyData,
			uint8_t *a_pbyBuffer, uint32_t a_unLength,
			bool a_bAsync = false);
	///
	int receive(uint8_t *a_pbyBuffer, uint32_t a_unLength);

	///
	SPI_TypeDef *getDevice() const {
		return m_pobjDevice;
	}
	///
	SPI_HandleTypeDef *getHandle() const {
		return const_cast<SPI_HandleTypeDef *>(m_pobjHandle);
	}
	///
	CExecWrite *getExecuterWrite() const {
		return const_cast<CExecWrite *>(&m_objExecWrite);
	}
	///
	CExecWriteRead *getExecuterWriteRead() const {
		return const_cast<CExecWriteRead *>(&m_objExecWriteRead);
	}
	///
	CHandler *getHandler() const {
		return m_pobjHandler;
	}
	///
	const EMode getMode() const {
		return m_cnMode;
	}
	///
	const ESsCntrlMode getSsCntrlMode() const {
		return m_cnSsCntrlMode;
	}
	///
	CSlaveBuffer *getSlaveBuffer() const {
		return m_pobjSlaveBuffer;
	}
	///
	bool pushSlaveBuffer(uint8_t *a_pbyData);

	///
	static void interruptHandler(CSpiBus *a_pobjSpi);
	///
	static CSpiBus *getInstance(EBus a_nBus);
	///
	static CSpiBus *getInstance(void *a_pHandle);

protected:
	IRQn_Type getIrq() const;

private:
	const EMode			m_cnMode;
	const ESsCntrlMode	m_cnSsCntrlMode;
	SPI_TypeDef			*m_pobjDevice;
	SPI_HandleTypeDef	*m_pobjHandle;
	CExecWrite			m_objExecWrite;
	CExecWriteRead		m_objExecWriteRead;
	CHandler			*m_pobjHandler;
	CSlaveBuffer		*m_pobjSlaveBuffer;

	///
	static CSpiBus		*m_pobjDriverList[EBusSupremum];
};

/**
 *
 */
class CUart : public CHandlerBase {
public:
	///
	enum ERequest {
		ERequestWrite = 100,
		ERequestRead
	};
	///
	enum EChannel {
		EChannel1 = 0,
		EChannel2,
		EChannel3,
		EChannel4,
		EChannelSupremum
	};

public:
	/**
	 *
	 */
	class CTx : public CHandlerBase {
	public:
		///
		CTx(CUart *a_pobjOwner);
		///
		virtual ~CTx();
		///
		int onInitialize();
		///
		int onTerminate();
		///
		int transmit(uint8_t *a_pbyData, uint32_t a_unLength);
		///
		int transmitAsync(uint8_t *a_pbyData, uint32_t a_unLength);
	public:
		CUart	*m_pobjOwner;
	};

	/**
	 *
	 */
	class CRequest {
	public:
		///
		CRequest(uint8_t *a_pbyData, uint32_t a_unLength, bool a_bAsync);
		///
		virtual ~CRequest();
	public:
		///
		bool		m_bAsync;
		///
		uint32_t	m_unLength;
		///
		uint8_t		*m_pbyData;
	};

	/**
	 *
	 */
	class CExecWrite : public CHandlerBase::CExecBase {
	public:
		///
		CExecWrite(CUart *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CUart	*m_pobjOwner;
	};

	/**
	 *
	 */
	class CExecRead : public CHandlerBase::CExecBase {
	public:
		///
		CExecRead(CUart *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CUart	*m_pobjOwner;
	};

	/**
	 *
	 */
	class CHandler {
	public:
		///
		CHandler();
		///
		virtual ~CHandler();
		///
		virtual void rxNotify(uint8_t *a_pbyData, uint32_t a_unLength) = 0;
		///
		virtual void errNotify(uint32_t a_unErrorCode) = 0;
	};

public:
	///
	CUart(USART_TypeDef *a_pobjDevice, CHandler *a_pobjHandler,
			UART_InitTypeDef *a_pobjInitTypeDef = NULL);
	///
	virtual ~CUart();
	///
	int onInitialize();
	///
	int onTerminate();
	///
	USART_TypeDef *getDevice() const {
		return m_pobjDevice;
	}
	///
	UART_HandleTypeDef *getHandle() const {
		return const_cast<UART_HandleTypeDef *>(m_pobjHandle);
	}
	///
	CHandler *getHandler() const {
		return m_pobjHandler;
	}
	///
	CInterruptRequestManager *getInterruptRequestManager() const {
		return m_pobjInterruptRequestManager;
	}
	///
	CRingBuffComm *getRingBuffer() const {
		return m_pobjRingBuffer;
	}
	///
	CSemaphore *getCmpltEventSemaphore() const {
		return m_pobjCmpltEvent;
	}
	///
	int transmit(uint8_t *a_pbyData, uint32_t a_unLength);
	///
	int transmitAsync(uint8_t *a_pbyData, uint32_t a_unLength);
	///
	int receive(uint8_t *a_pbyBuffer, uint32_t a_unLength, uint32_t a_unTimeout);
	///
	void rxHandler();
	///
	void txCmpltHandler(uint8_t *a_pbyData, uint32_t a_unLength);
	///
	void errorHandler();
	///
	const CExecWrite *getExecWrite() const {
		return &m_objExecWrite;
	}
	///
	CHandlerBase *getTransmitThreadHandle() const {
		return m_pobjTx;
	}
	///
	int getChannel() const;
	///
	static void interruptHandler(CUart *a_pobjUart);
	///
	static CUart *getInstance(EChannel a_nChannel);

protected:
	IRQn_Type getIrq() const;

private:
	///
	USART_TypeDef				*m_pobjDevice;
	///
	UART_HandleTypeDef			*m_pobjHandle;
	///
	UART_InitTypeDef			*m_pobjInitTypeDef;
	///
	CExecWrite					m_objExecWrite;
	///
	CExecRead					m_objExecRead;
	///
	CHandler					*m_pobjHandler;
	///
	CInterruptRequestManager	*m_pobjInterruptRequestManager;
	///
	CRingBuffComm				*m_pobjRingBuffer;
	///
	CSemaphore					*m_pobjCmpltEvent;
	///
	CTx							*m_pobjTx;

	///
	static CUart				*m_pobjDriverList[EChannelSupremum];
};

/**
 *
 */
class CUsbCdcAcm : public CHandlerBase {
public:
	///
	enum ERequest {
		ERequestWrite = 100,
		ERequestWriteComplete,
		ERequestRead
	};

public:
	/**
	 *
	 */
	class CRequest {
	public:
		///
		CRequest();
	public:
		///
		uint32_t	m_unLength;
		///
		uint8_t		*m_pbyData;
	};

	/**
	 *
	 */
	class CExecWrite : public CHandlerBase::CExecBase {
	public:
		///
		CExecWrite(CUsbCdcAcm *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CUsbCdcAcm	*m_pobjOwner;
	};

	/**
	 *
	 */
	class CExecWriteComplete : public CHandlerBase::CExecBase {
	public:
		///
		CExecWriteComplete(CUsbCdcAcm *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CUsbCdcAcm	*m_pobjOwner;
	};

	/**
	 *
	 */
	class CExecRead : public CHandlerBase::CExecBase {
	public:
		///
		CExecRead(CUsbCdcAcm *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CUsbCdcAcm	*m_pobjOwner;
	};

	/**
	 *
	 */
	class CHandler {
	public:
		///
		CHandler();
		///
		virtual ~CHandler();
		///
		virtual void rxNotify(uint8_t *a_pbyData, uint32_t a_unLength) = 0;
	};

public:
	///
	virtual ~CUsbCdcAcm();
	///
	int onInitialize();
	///
	CHandler *getHandler() const {
		return m_pobjHandler;
	}
	///
	CInterruptRequestManager *getInterruptRequestManager() const {
		return m_pobjInterruptRequestManager;
	}
	///
	CInterruptRequestManager *getIntRequestManagerTxCmplt() const {
		return m_pobjIntRequestManagerTxCmplt;
	}
	///
	CRingBuffComm *getRingBuffer() const {
		return m_pobjRingBuffer;
	}
	///
	int transmit(uint8_t *a_pbyData, uint32_t a_unLength);
	///
	int receive(uint8_t *a_pbyBuffer, uint32_t a_unLength, uint32_t a_unTimeout);
	///
	void rxHandler(uint8_t *a_pbyData, uint32_t a_unLength);
	///
	void txCmpltHandler(uint8_t *a_pbyData, uint32_t a_unLength);

	///
	static CUsbCdcAcm *newInstance(CHandler *a_pobjHandler);
	///
	static CUsbCdcAcm *getInstance();

private:
	///
	CUsbCdcAcm(CHandler *a_pobjHandler);

private:
	///
	CHandler					*m_pobjHandler;
	///
	CInterruptRequestManager	*m_pobjInterruptRequestManager;
	///
	CInterruptRequestManager	*m_pobjIntRequestManagerTxCmplt;
	///
	CRingBuffComm				*m_pobjRingBuffer;
	///
	CExecWrite					m_objExecWrite;
	///
	CExecWriteComplete			m_objExecWriteComplete;
	///
	CExecRead					m_objExecRead;
	///
	static CUsbCdcAcm			*m_pobjDriver;
};

/**
 *
 */
class CAnalogInput : public CHandlerBase {
public:
	///
	CAnalogInput(const ADC_TypeDef *a_cpobjAdc, const uint32_t a_cnChannel,
			const uint32_t a_cnTimeout);
	///
	virtual ~CAnalogInput();
	///
	virtual int onInitialize();
	///
	virtual int onTerminate();
	///
	virtual int onTimeout();
	///
	uint32_t getValue() const {
		return m_unAdcValue;
	}

private:
	int read();

private:
	ADC_HandleTypeDef	m_objAdcHandleTypeDef;
	const uint32_t		m_cnChannel;
	uint32_t			m_unAdcValue;
};

#endif /* __cplusplus */

#endif /* __PERIPHERAL_INTERFACE_H */
