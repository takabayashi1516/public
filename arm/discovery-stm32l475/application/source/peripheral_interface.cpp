/**
 *
 */
#include <stdlib.h>
#include <string.h>

#include <stm32l4xx.h>
#include <stm32l4xx_hal_def.h>
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_cortex.h>
#include <stm32l4xx_hal_i2c.h>
#include <stm32l4xx_hal_gpio.h>
#include <stm32l4xx_hal_rcc.h>

#include <usbd_def.h>
#include <usb_device.h>
#include <usbd_cdc_if.h>

#include <peripheral_interface.h>

/**
 */
CSpiBus	*CSpiBus::m_pobjDriverList[CSpiBus::EBusSupremum];

/**
 */
CUart	*CUart::m_pobjDriverList[CUart::EChannelSupremum];

/**
 */
CUsbCdcAcm	*CUsbCdcAcm::m_pobjDriver;

/**
 *
 */
void USART1_IRQHandler(void)
{
	CUart::interruptHandler(CUart::getInstance(CUart::EChannel1));
}

/**
 *
 */
void USART2_IRQHandler(void)
{
	CUart::interruptHandler(CUart::getInstance(CUart::EChannel2));
}

/**
 *
 */
void USART3_IRQHandler(void)
{
	CUart::interruptHandler(CUart::getInstance(CUart::EChannel3));
}

/**
 *
 */
void UART4_IRQHandler(void)
{
	CUart::interruptHandler(CUart::getInstance(CUart::EChannel4));
}

/**
 *
 */
void SPI1_IRQHandler(void)
{
	CSpiBus::interruptHandler(CSpiBus::getInstance(CSpiBus::EBus1));
}

/**
 *
 */
void SPI2_IRQHandler(void)
{
	CSpiBus::interruptHandler(CSpiBus::getInstance(CSpiBus::EBus2));
}

/**
 *
 */
void SPI3_IRQHandler(void)
{
	CSpiBus::interruptHandler(CSpiBus::getInstance(CSpiBus::EBus3));
}

/**
 *
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	for (int i = 0; i < CUart::EChannelSupremum; i++) {
		CUart *uart = CUart::getInstance(static_cast<CUart::EChannel>(i));
		if (uart->getHandle() == huart) {
//			::HAL_UART_DMAStop(huart);
			uart->txCmpltHandler(huart->pTxBuffPtr, huart->TxXferSize);
			break;
		}
	}
}

/**
 *
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	for (int i = 0; i < CUart::EChannelSupremum; i++) {
		CUart *uart = CUart::getInstance(static_cast<CUart::EChannel>(i));
		if (uart->getHandle() == huart) {
			uart->errorHandler();
			break;
		}
	}
}

/**
 *
 */
void Apl_UART_RxCallback(UART_HandleTypeDef *huart)
{
	if (huart->ErrorCode != 0) {
		return;
	}
	for (int i = 0; i < CUart::EChannelSupremum; i++) {
		CUart *uart = CUart::getInstance(static_cast<CUart::EChannel>(i));
		if (uart->getHandle() == huart) {
			uart->rxHandler();
			break;
		}
	}
}

/**
 *
 */
void OTG_FS_IRQHandler(void)
{
	extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
	::HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

/**
 *
 */
void Apl_CDC_RxCallback(USBD_HandleTypeDef *husb)
{
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)(husb->pClassData);
	CUsbCdcAcm::getInstance()->rxHandler(hcdc->RxBuffer, hcdc->RxLength);
}

/**
 *
 */
void Apl_CDC_TxCmpltCallback(USBD_HandleTypeDef *husb)
{
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)(husb->pClassData);
	CUsbCdcAcm::getInstance()->txCmpltHandler(hcdc->TxBuffer, hcdc->TxLength);
}

/**
 *
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	CSpiBus::getInstance(hspi)->getExecuterWrite()->getSemaphore()->signal();
}

/**
 *
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	CSpiBus::getInstance(hspi)->getExecuterWriteRead()->getSemaphore()->signal();
}

/**
 *
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	CSpiBus::getInstance(hspi)->getExecuterWriteRead()->getSemaphore()->signal();
}

/**
 *
 */
void CRingBuffComm::onRequireLock()
{
	if (m_cnIrq != ~0) {
		::HAL_NVIC_DisableIRQ(m_cnIrq);
	}
}

/**
 *
 */
void CRingBuffComm::onRequireUnLock()
{
	if (m_cnIrq != ~0) {
		::HAL_NVIC_EnableIRQ(m_cnIrq);
	}
}

/**
 *
 */
CI2cBus::CRequest::CRequest()
{
	::memset(this, 0, sizeof(CRequest));
}

/**
 *
 */
int CI2cBus::CExecWrite::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CRequest)) {
	}
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	HAL_StatusTypeDef result;
	result = ::HAL_I2C_Master_Transmit(
			m_pobjOwner->getHandle(), (req->m_hwSla) << 1, req->m_pbyData,
			req->m_unDataLength, (uint32_t)~0u);
	return result;
}

/**
 *
 */
int CI2cBus::CExecWriteRead::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CRequest)) {
	}
	HAL_StatusTypeDef result;
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	if (req->m_pbyData && req->m_unDataLength) {
		result = ::HAL_I2C_Master_Transmit(m_pobjOwner->getHandle(), (req->m_hwSla) << 1, req->m_pbyData,
				req->m_unDataLength, ~0u);
		if (result != HAL_OK) {
			return result;
		}
	}
	result = ::HAL_I2C_Master_Receive(m_pobjOwner->getHandle(), ((req->m_hwSla) << 1) | 0x01,
			req->m_pbyBuffer, req->m_unBufferLength, ~0u);
	return result;
}

/**
 *
 */
CI2cBus::CI2cBus(I2C_TypeDef *a_pobjDevice)
	:	CHandlerBase(~0u, 256u, 8u, 6u, NULL, true), m_pobjDevice(a_pobjDevice),
			m_pobjHandle(NULL), m_objExecWrite(this), m_objExecWriteRead(this)
{
	signalPrepare();
	waitStartUp();
}

/**
 *
 */
CI2cBus::~CI2cBus()
{
}

/**
 *
 */
int CI2cBus::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	if (!((getDevice() == I2C1) || (getDevice() == I2C2))) {
		return -1;
	}

	m_pobjHandle = reinterpret_cast<I2C_HandleTypeDef *>(::malloc(sizeof(I2C_HandleTypeDef)));
	::memset(m_pobjHandle, 0, sizeof(I2C_HandleTypeDef));

	I2C_HandleTypeDef *h = m_pobjHandle;

	h->Instance = getDevice();
	h->Init.Timing = 0x00702991;
	h->Init.OwnAddress1 = 0;
	h->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	h->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	h->Init.OwnAddress2 = 0;
	h->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	h->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	h->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (::HAL_I2C_Init(h) != HAL_OK) {
		return -1;
	}
	if (::HAL_I2CEx_ConfigAnalogFilter(h, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
		return -1;
	}
	if (::HAL_I2CEx_ConfigDigitalFilter(h, 0) != HAL_OK) {
		return -1;
	}

	(void) registerExec(ERequestWrite, &m_objExecWrite);
	(void) registerExec(ERequestWriteRead, &m_objExecWriteRead);

	return CThreadInterfaceBase::onInitialize();
}

/**
 *
 */
int CI2cBus::onTerminate()
{
	if (m_pobjHandle) {
		I2C_HandleTypeDef *h = m_pobjHandle;
		::HAL_I2C_DeInit(h);
		::free(m_pobjHandle);
	}
	return 0;
}

/**
 *
 */
int CI2cBus::transmit(uint8_t a_bySla, uint8_t *a_pbyData, uint32_t a_unLength)
{
	CRequest req;
	req.m_hwSla			= a_bySla;
	req.m_unDataLength	= a_unLength;
	req.m_pbyData		= a_pbyData;
	return requestSync(ERequestWrite, &req, sizeof(req));
}

/**
 *
 */
int CI2cBus::receive(uint8_t a_bySla, uint8_t *a_pbyData, uint32_t a_unDataLength, uint8_t *a_pbyBuffer, uint32_t a_unBufferLength)
{
	CRequest req;
	req.m_hwSla				= a_bySla;
	req.m_unDataLength		= a_unDataLength;
	req.m_pbyData			= a_pbyData;
	req.m_unBufferLength	= a_unBufferLength;
	req.m_pbyBuffer			= a_pbyBuffer;
	return requestSync(ERequestWriteRead, &req, sizeof(req));
}

/**
 *
 */
CSpiBus::CSlaveBuffer::CSlaveBuffer(CSpiBus *a_pobjOwner,
		uint32_t a_unSlaveBuffeSize)
	:	CTripleBuffer(a_unSlaveBuffeSize), m_pobjOwner(a_pobjOwner)
{
}

/**
 *
 */
CSpiBus::CSlaveBuffer::~CSlaveBuffer()
{
}

/**
 *
 */
void CSpiBus::CSlaveBuffer::onRequireLock()
{
	CTripleBuffer::onRequireLock();
	// DMA Stream?
	::HAL_NVIC_DisableIRQ(m_pobjOwner->getIrq());
}

/**
 *
 */
void CSpiBus::CSlaveBuffer::onRequireUnLock()
{
	::HAL_NVIC_EnableIRQ(m_pobjOwner->getIrq());
	// DMA Stream?
	CTripleBuffer::onRequireUnLock();
}

/**
 *
 */
CSpiBus::CRequest::CRequest()
{
	::memset(this, 0, sizeof(CRequest));
}

/**
 *
 */
int CSpiBus::CExecWrite::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CRequest)) {
	}
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	HAL_StatusTypeDef result;
	do {
		result = ::HAL_SPI_Transmit_DMA(m_pobjOwner->getHandle(),
				req->m_pbyData, req->m_unLength);
	} while (result != HAL_OK);
	m_pobjSemaphore->wait();
	if (req->m_bAlloc) {
		delete [] req->m_pbyData;
		delete req;
	}
	return result;
}

/**
 *
 */
int CSpiBus::CExecWriteRead::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CRequest)) {
	}
	HAL_StatusTypeDef result;
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	uint8_t *snd_data = req->m_pbyData;
	if (m_pobjOwner->getSlaveBuffer()) {
		snd_data = m_pobjOwner->getSlaveBuffer()->update();
	}
	if (snd_data) {
		do {
			result = ::HAL_SPI_TransmitReceive_DMA(m_pobjOwner->getHandle(),
					snd_data, req->m_pbyBuffer, req->m_unLength);
		} while (result != HAL_OK);
	} else {
		do {
			result = ::HAL_SPI_Receive_DMA(m_pobjOwner->getHandle(),
					req->m_pbyBuffer, req->m_unLength);
		} while (result != HAL_OK);
	}
	m_pobjSemaphore->wait();
	if (m_pobjOwner->getHandler()) {
		m_pobjOwner->getHandler()->rxNotify(req->m_pbyBuffer, req->m_unLength);
	}
	if (req->m_bAlloc) {
		if ((!(m_pobjOwner->getSlaveBuffer())) && (req->m_pbyData)) {
			delete [] req->m_pbyData;
		}
		if (req->m_pbyBuffer) {
			delete [] req->m_pbyBuffer;
		}
		delete req;
	}
	return result;
}

/**
 *
 */
CSpiBus::CSpiBus(SPI_TypeDef *a_pobjDevice, EMode a_nMode, ESsCntrlMode a_nSsCntrlMode,
		CHandler *a_pobjHandler, uint32_t a_unSlaveBuffeSize)
	:	CHandlerBase(~0u, 256u, 8u, 6u, NULL, true), m_cnMode(a_nMode), m_cnSsCntrlMode(a_nSsCntrlMode),
			m_pobjDevice(a_pobjDevice), m_pobjHandle(NULL),
			m_objExecWrite(this), m_objExecWriteRead(this),
			m_pobjHandler(a_pobjHandler), m_pobjSlaveBuffer(NULL)
{
	if (getDevice() == SPI1) {
		m_pobjDriverList[EBus1] = this;
	} else if (getDevice() == SPI2) {
		m_pobjDriverList[EBus2] = this;
	} else if (getDevice() == SPI3) {
		m_pobjDriverList[EBus3] = this;
	} else {
		assert_param(false);
	}
	if (getMode() == EModeSlave) {
		assert_param(a_unSlaveBuffeSize);
		m_pobjSlaveBuffer = new CSlaveBuffer(this, a_unSlaveBuffeSize);
	}
	signalPrepare();
	waitStartUp();
}

/**
 *
 */
CSpiBus::~CSpiBus()
{
}

/**
 *
 */
int CSpiBus::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	m_pobjHandle = reinterpret_cast<SPI_HandleTypeDef *>(::malloc(sizeof(SPI_HandleTypeDef)));
	::memset(m_pobjHandle, 0, sizeof(SPI_HandleTypeDef));

	SPI_HandleTypeDef *h = m_pobjHandle;

	h->Instance = getDevice();
	if (m_cnMode == EModeMaster) {
		h->Init.Mode = SPI_MODE_MASTER;
	} else {
		h->Init.Mode = SPI_MODE_SLAVE;
	}
	h->Init.Direction = SPI_DIRECTION_2LINES;
	h->Init.DataSize = SPI_DATASIZE_8BIT;
	h->Init.CLKPolarity = SPI_POLARITY_LOW;
	h->Init.CLKPhase = SPI_PHASE_1EDGE;
	if (m_cnSsCntrlMode == ESsCntrlModeHardware) {
		if (m_cnMode == EModeMaster) {
			h->Init.NSS = SPI_NSS_HARD_OUTPUT;
		} else {
			h->Init.NSS = SPI_NSS_HARD_INPUT;
		}
	} else /* if (m_cnSsCntrlMode == ESsCntrlModeSoftware) */ {
		h->Init.NSS = SPI_NSS_SOFT;
	}
	h->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	h->Init.FirstBit = SPI_FIRSTBIT_MSB;
	h->Init.TIMode = SPI_TIMODE_DISABLE;
	h->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	h->Init.CRCPolynomial = 7;
	h->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	h->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (::HAL_SPI_Init(h) != HAL_OK) {
		return -1;
	}

	(void) registerExec(ERequestWrite, &m_objExecWrite);
	(void) registerExec(ERequestWriteRead, &m_objExecWriteRead);

	IRQn_Type irq = getIrq();
	/*Configure the IRQ priority see configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY */
	::HAL_NVIC_SetPriority(irq, 5u, 0u);
	/* Enable the global Interrupt */
	::HAL_NVIC_EnableIRQ(irq);

	return CThreadInterfaceBase::onInitialize();
}

/**
 *
 */
int CSpiBus::onTerminate()
{
	if (m_pobjHandle) {
		SPI_HandleTypeDef *h = m_pobjHandle;
		::HAL_SPI_DeInit(h);
		::free(m_pobjHandle);
	}
	return 0;
}

/**
 *
 */
int CSpiBus::transmit(EChannel a_nChannel, uint8_t *a_pbyData,
		uint32_t a_unLength, bool a_bAsync)
{
	CRequest *req = NULL;
	if (!a_bAsync) {
		CRequest reqInst;
		req = &reqInst;
		req->m_nChannel	= a_nChannel;
		req->m_unLength	= a_unLength;
		req->m_pbyData	= a_pbyData;
		req->m_bAlloc	= a_bAsync;
		return requestSync(ERequestWrite, &req, sizeof(req));
	}
	req = new CRequest();
	req->m_nChannel	= a_nChannel;
	req->m_unLength	= a_unLength;
	req->m_pbyData	= new uint8_t[a_unLength];
	req->m_bAlloc	= a_bAsync;
	::memcpy(req->m_pbyData, a_pbyData, a_unLength);
	return requestAsync(ERequestWrite, &req, sizeof(req));
}

/**
 *
 */
int CSpiBus::transmitReceive(EChannel a_nChannel, uint8_t *a_pbyData,
		uint8_t *a_pbyBuffer, uint32_t a_unLength, bool a_bAsync)
{
	CSpiBus::CRequest *req = NULL;
	if (!a_bAsync) {
		CSpiBus::CRequest reqInst;
		req = &reqInst;
		req->m_nChannel	= a_nChannel;
		req->m_unLength	= a_unLength;
		req->m_pbyBuffer= a_pbyBuffer;
		req->m_bAlloc	= a_bAsync;
		req->m_pbyData	= a_pbyData;
		return requestSync(ERequestWriteRead, &req, sizeof(req));
	}
	req = new CSpiBus::CRequest();
	req->m_nChannel	= a_nChannel;
	req->m_unLength	= a_unLength;
	req->m_pbyBuffer= a_pbyBuffer;
	req->m_bAlloc	= a_bAsync;
	req->m_pbyData	= NULL;
	if (a_pbyData) {
		req->m_pbyData	= new uint8_t[a_unLength];
		::memcpy(req->m_pbyData, a_pbyData, a_unLength);
	}
	return requestAsync(ERequestWriteRead, &req, sizeof(req));
}

/**
 *
 */
int CSpiBus::receive(uint8_t *a_pbyBuffer, uint32_t a_unLength)
{
	CRequest *req = new CRequest();
	req->m_nChannel	= EChannelSlave;
	req->m_unLength	= a_unLength;
	req->m_pbyData	= NULL;
	req->m_pbyBuffer= a_pbyBuffer;
	req->m_bAlloc	= true;
	return requestAsync(ERequestWriteRead, &req, sizeof(req));
}

/**
 *
 */
void CSpiBus::interruptHandler(CSpiBus *a_pobjSpi)
{
	::HAL_SPI_IRQHandler(a_pobjSpi->getHandle());
}

/**
 *
 */
CSpiBus *CSpiBus::getInstance(EBus a_nBus)
{
	if (a_nBus >= EBusSupremum) {
		return NULL;
	}
	return m_pobjDriverList[a_nBus];
}

/**
 *
 */
CSpiBus *CSpiBus::getInstance(void *a_pHandle)
{
	SPI_HandleTypeDef *h = reinterpret_cast<SPI_HandleTypeDef *>(a_pHandle);
	for (int i = EBus1; i < EBusSupremum; i++) {
		if (m_pobjDriverList[i]->getHandle() == h) {
			return m_pobjDriverList[i];
		}
	}
	return NULL;
}

/**
 *
 */
bool CSpiBus::pushSlaveBuffer(uint8_t *a_pbyData)
{
	if (!getSlaveBuffer()) {
		return false;
	}
	getSlaveBuffer()->push(a_pbyData);
	return true;
}

/**
 *
 */
IRQn_Type CSpiBus::getIrq() const
{
	IRQn_Type irq = UsageFault_IRQn;
	SPI_TypeDef *dev = getDevice();
	if (dev == SPI1) {
		irq = SPI1_IRQn;
	} else if (dev == SPI2) {
		irq = SPI2_IRQn;
	} else if (dev == SPI3) {
		irq = SPI3_IRQn;
	} else {
		assert_param(false);
	}
	return irq;
}

/**
 *
 */
CSpiBus::CHandler::CHandler()
{
}

/**
 *
 */
CSpiBus::CHandler::~CHandler()
{
}

/**
 *
 */
CUart::CTx::CTx(CUart *a_pobjOwner)
	:	CHandlerBase(~0u, 512u, 384u, 6u, NULL, true), m_pobjOwner(a_pobjOwner)
{
	signalPrepare();
	waitStartUp();
}

/**
 *
 */
CUart::CTx::~CTx()
{
}

/**
 *
 */
int CUart::CTx::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	(void) registerExec(CUart::ERequestWrite, const_cast<CUart::CExecWrite *>(m_pobjOwner->getExecWrite()));
	return CThreadInterfaceBase::onInitialize();
}

/**
 *
 */
int CUart::CTx::onTerminate()
{
	return 0;
}

/**
 *
 */
int CUart::CTx::transmit(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CUart::CRequest reqInst(a_pbyData, a_unLength, false);
	CUart::CRequest *req = &reqInst;
	return requestSync(CUart::ERequestWrite, &req, sizeof(req));
}

/**
 *
 */
int CUart::CTx::transmitAsync(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CUart::CRequest *req = new CUart::CRequest(a_pbyData, a_unLength, true);
	assert_param(req);
	if (!req) {
		return -1;
	}
	return requestAsync(CUart::ERequestWrite, &req, sizeof(req));
}

/**
 *
 */
CUart::CRequest::CRequest(uint8_t *a_pbyData, uint32_t a_unLength, bool a_bAsync)
	:	m_bAsync(a_bAsync), m_unLength(a_unLength), m_pbyData(a_pbyData)
{
	if (!m_bAsync) {
		return;
	}
	if (m_unLength <= 0u) {
		return;
	}
	if (!a_pbyData) {
		return;
	}
	uint32_t l = _RNDUP_REMINDER(m_unLength, PERI_IF_BUF_ALINE);
	m_pbyData = new uint8_t[l];
	assert_param(m_pbyData);
	if (!m_pbyData) {
		return;
	}
	::memset(m_pbyData, 0, l);
	::memcpy(m_pbyData, a_pbyData, m_unLength);
}

/**
 *
 */
CUart::CRequest::~CRequest()
{
	if (!m_bAsync) {
		return;
	}
	if (m_unLength <= 0u) {
		return;
	}
	if (!m_pbyData) {
		return;
	}
	delete [] m_pbyData;
}

/**
 *
 */
int CUart::CExecWrite::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CRequest)) {
	}
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	HAL_StatusTypeDef result = HAL_OK;
	if (!m_pobjOwner->getCmpltEventSemaphore()) {
		do {
			result = ::HAL_UART_Transmit(m_pobjOwner->getHandle(), req->m_pbyData,
					req->m_unLength, (uint32_t)~0u);
		} while (result != HAL_OK);
	} else {
		do {
			result = ::HAL_UART_Transmit_DMA(m_pobjOwner->getHandle(), req->m_pbyData, req->m_unLength);
//			result = ::HAL_UART_Transmit_IT(m_pobjOwner->getHandle(), req->m_pbyData, req->m_unLength);
		} while (result != HAL_OK);
		m_pobjOwner->getCmpltEventSemaphore()->wait();
	}
	if (req->m_bAsync) {
		delete req;
	}
	return result;
}

/**
 *
 */
int CUart::CExecRead::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CInterruptRequest::CRequest *)) {
	}
	CInterruptRequest::CRequest *req =
			reinterpret_cast<CInterruptRequest::CRequest *>(const_cast<void *>(a_lpParam));
	if (m_pobjOwner->getHandler()) {
		CRingBuffComm *rb = reinterpret_cast<CRingBuffComm *>(req->m_pbyData);
		uint8_t *buf;
		uint32_t len;
		bool result = rb->getData(buf, len);
		if (result) {
			m_pobjOwner->getHandler()->rxNotify(buf, len);
			delete [] buf;
		}
	}
	m_pobjOwner->getInterruptRequestManager()->release(req);
	return 0;
}

/**
 *
 */
CUart::CUart(USART_TypeDef *a_pobjDevice, CHandler *a_pobjHandler,
		UART_InitTypeDef *a_pobjInitTypeDef)
	:	CHandlerBase(~0u, 256u, 512u, 5u, NULL, true),
			m_pobjDevice(a_pobjDevice), m_pobjHandle(NULL),
			m_pobjInitTypeDef(NULL), m_objExecWrite(this), m_objExecRead(this),
			m_pobjHandler(a_pobjHandler), m_pobjInterruptRequestManager(NULL),
			m_pobjRingBuffer(NULL), m_pobjCmpltEvent(NULL), m_pobjTx(NULL)
{
	if (getDevice() == USART1) {
		m_pobjDriverList[EChannel1] = this;
	} else if (getDevice() == USART2) {
		m_pobjDriverList[EChannel2] = this;
	} else if (getDevice() == USART3) {
		m_pobjDriverList[EChannel3] = this;
	} else if (getDevice() == UART4) {
		m_pobjDriverList[EChannel4] = this;
	} else {
		return;
	}
	m_pobjTx = new CTx(this);
	m_pobjInterruptRequestManager = new CInterruptRequestManager(ERequestRead,
			sizeof(void *), 64u);
	if (a_pobjInitTypeDef) {
		m_pobjInitTypeDef = reinterpret_cast<UART_InitTypeDef *>(
				::malloc(sizeof(UART_InitTypeDef)));
		::memcpy(m_pobjInitTypeDef, a_pobjInitTypeDef, sizeof(UART_InitTypeDef));
	}
	m_pobjRingBuffer = new CRingBuffComm(getIrq(), 256u);
	m_pobjCmpltEvent = new CSemaphore();
	assert_param(m_pobjTx);
	assert_param(m_pobjInterruptRequestManager);
	assert_param(m_pobjRingBuffer);
	assert_param(m_pobjCmpltEvent);
	signalPrepare();
	waitStartUp();
}

/**
 *
 */
CUart::~CUart()
{
	if (m_pobjTx) {
		delete m_pobjTx;
	}
	if (m_pobjHandle) {
		::free(m_pobjHandle);
	}
	if (getCmpltEventSemaphore()) {
		delete getCmpltEventSemaphore();
	}
	if (getInterruptRequestManager()) {
		delete getInterruptRequestManager();
	}
	if (getRingBuffer()) {
		delete getRingBuffer();
	}
}

/**
 *
 */
int CUart::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	m_pobjHandle = reinterpret_cast<UART_HandleTypeDef *>(
			::malloc(sizeof(UART_HandleTypeDef)));
	if (!m_pobjHandle) {
		return -1;
	}
	::memset(m_pobjHandle, 0, sizeof(UART_HandleTypeDef));

	UART_HandleTypeDef *h = getHandle();

	h->Instance = getDevice();
	if (m_pobjInitTypeDef) {
		::memcpy(&(h->Init), m_pobjInitTypeDef, sizeof(UART_InitTypeDef));
		::free(m_pobjInitTypeDef);
		m_pobjInitTypeDef = NULL;
	} else {
		h->Init.BaudRate = 115200;
		h->Init.WordLength = UART_WORDLENGTH_8B;
		h->Init.StopBits = UART_STOPBITS_1;
		h->Init.Parity = UART_PARITY_NONE;
		h->Init.Mode = UART_MODE_TX_RX;
		h->Init.HwFlowCtl = UART_HWCONTROL_NONE;
		h->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_ENABLE;
		h->Init.OverSampling = UART_OVERSAMPLING_16;
	}
	h->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (::HAL_UART_Init(h) != HAL_OK) {
		return -1;
	}

	IRQn_Type irq = getIrq();
	/*Configure the IRQ priority see configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY */
	::HAL_NVIC_SetPriority(irq, 5u, 0u);
	/* Enable the global Interrupt */
	::HAL_NVIC_EnableIRQ(irq);

	/* Enable the UART Parity Error Interrupt */
	SET_BIT(getDevice()->CR1, USART_CR1_PEIE);
	/* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	SET_BIT(getDevice()->CR3, USART_CR3_EIE);
	/* Enable the UART Data Register not empty Interrupt */
	SET_BIT(getDevice()->CR1, USART_CR1_RXNEIE);

//	(void) registerExec(ERequestWrite, &m_objExecWrite);
	(void) registerExec(ERequestRead, &m_objExecRead);

	return CThreadInterfaceBase::onInitialize();
}

/**
 *
 */
int CUart::onTerminate()
{
	UART_HandleTypeDef *h = m_pobjHandle;
	::HAL_UART_DeInit(h);
	return 0;
}

/**
 *
 */
int CUart::transmit(uint8_t *a_pbyData, uint32_t a_unLength)
{
	return m_pobjTx->transmit(a_pbyData, a_unLength);
}

/**
 *
 */
int CUart::transmitAsync(uint8_t *a_pbyData, uint32_t a_unLength)
{
	return m_pobjTx->transmitAsync(a_pbyData, a_unLength);
}

/**
 *
 */
int CUart::receive(uint8_t *a_pbyBuffer, uint32_t a_unLength, uint32_t a_unTimeout)
{
	return (int) getRingBuffer()->pullWait(a_pbyBuffer, a_unLength, a_unTimeout);
}

/**
 *
 */
void CUart::rxHandler()
{
	UART_HandleTypeDef *huart = getHandle();
	USART_TypeDef *dev = getDevice();

	CInterruptRequest *req = getInterruptRequestManager()->getFree();
	if (!req) {return;}

	for (; ((dev->ISR) & USART_ISR_RXNE);) {
		uint16_t tmp = (dev->RDR) & 0x01ffu;
		if (huart->Init.WordLength == UART_WORDLENGTH_9B) {
			if (huart->Init.Parity == UART_PARITY_NONE) {
				getRingBuffer()->push(reinterpret_cast<uint8_t *>(&tmp), sizeof(uint16_t));
				continue;
			}
			tmp &= 0xffu;
			getRingBuffer()->push(reinterpret_cast<uint8_t *>(&tmp), sizeof(uint8_t));
			continue;
		}
		if (huart->Init.Parity == UART_PARITY_NONE) {
			tmp &= 0xffu;
			getRingBuffer()->push(reinterpret_cast<uint8_t *>(&tmp), sizeof(uint8_t));
			continue;
		}
		tmp &= 0x7fu;
		getRingBuffer()->push(reinterpret_cast<uint8_t *>(&tmp), sizeof(uint8_t));
		continue;
	}
	req->setDataAddress(getRingBuffer());
	req->setLength(sizeof(void *));
	sendMessage(req);
}

/**
 *
 */
void CUart::txCmpltHandler(uint8_t *a_pbyData, uint32_t a_unLength)
{
	getCmpltEventSemaphore()->signal();
}

/**
 *
 */
void CUart::errorHandler()
{
	::printf("l(%4d): %s: ch=%d\n", __LINE__, __PRETTY_FUNCTION__, getChannel());
	::HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	getHandler()->errNotify(getHandle()->ErrorCode);
}

/**
 *
 */
int CUart::getChannel() const
{
	for (int i = 0; i < CUart::EChannelSupremum; i++) {
		CUart *uart = CUart::getInstance(static_cast<CUart::EChannel>(i));
		if (uart == this) {
			return i;
		}
	}
	return -1;
}

/**
 *
 */
void CUart::interruptHandler(CUart *a_pobjUart)
{
	::HAL_UART_IRQHandler(a_pobjUart->getHandle());
}

/**
 *
 */
CUart *CUart::getInstance(EChannel a_nChannel)
{
	return m_pobjDriverList[a_nChannel];
}

/**
 *
 */
IRQn_Type CUart::getIrq() const
{
	IRQn_Type irq = UsageFault_IRQn;
	USART_TypeDef *dev = getDevice();
	if (dev == USART1) {
		irq = USART1_IRQn;
	} else if (dev == USART2) {
		irq = USART2_IRQn;
	} else if (dev == USART3) {
		irq = USART3_IRQn;
	} else if (dev == UART4) {
		irq = UART4_IRQn;
	} else {
		assert_param(false);
	}
	return irq;
}

/**
 *
 */
CUart::CHandler::CHandler()
{
}

/**
 *
 */
CUart::CHandler::~CHandler()
{
}

/**
 *
 */
CUsbCdcAcm::CRequest::CRequest()
{
	::memset(this, 0, sizeof(CRequest));
}

/**
 *
 */
int CUsbCdcAcm::CExecWrite::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CRequest)) {
	}
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	uint8_t result = ::CDC_Transmit_FS(req->m_pbyData, req->m_unLength);
	if (result) {
		delete [] req->m_pbyData;
	}
//	delete req;
	return result;
}

/**
 *
 */
int CUsbCdcAcm::CExecWriteComplete::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CInterruptRequest::CRequest)) {
	}
	CInterruptRequest::CRequest *req =
			reinterpret_cast<CInterruptRequest::CRequest *>(const_cast<void *>(a_lpParam));
	delete [] req->m_pbyData;
	m_pobjOwner->getIntRequestManagerTxCmplt()->release(req);
	return 0;
}

/**
 *
 */
int CUsbCdcAcm::CExecRead::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CInterruptRequest::CRequest *)) {
	}
	CInterruptRequest::CRequest *req =
			reinterpret_cast<CInterruptRequest::CRequest *>(const_cast<void *>(a_lpParam));
	if (m_pobjOwner->getHandler()) {
		CRingBuffComm *rb = reinterpret_cast<CRingBuffComm *>(req->m_pbyData);
		uint8_t *buf;
		uint32_t len;
		bool result = rb->getData(buf, len);
		if (result) {
			m_pobjOwner->getHandler()->rxNotify(buf, len);
			delete [] buf;
		}
	}
	m_pobjOwner->getInterruptRequestManager()->release(req);
	return 0;
}

/**
 *
 */
CUsbCdcAcm::CHandler::CHandler()
{
}

/**
 *
 */
CUsbCdcAcm::CHandler::~CHandler()
{
}

/**
 *
 */
CUsbCdcAcm::~CUsbCdcAcm()
{
	if (getInterruptRequestManager()) {
		delete getInterruptRequestManager();
	}
	if (getRingBuffer()) {
		delete getRingBuffer();
	}
}

/**
 *
 */
int CUsbCdcAcm::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	::MX_USB_DEVICE_Init();
	(void) registerExec(ERequestWrite, &m_objExecWrite);
	(void) registerExec(ERequestWriteComplete, &m_objExecWriteComplete);
	(void) registerExec(ERequestRead, &m_objExecRead);
	return CThreadInterfaceBase::onInitialize();
}

/**
 *
 */
int CUsbCdcAcm::transmit(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CRequest reqInst;
	CRequest *req = &reqInst;
	uint32_t l = _RNDUP_REMINDER(a_unLength, PERI_IF_BUF_ALINE);
	uint8_t *p = new uint8_t[l];
	assert_param(p);
	if (!p) {
		return -1;
	}
	::memset(p, 0, l);
	::memcpy(p, a_pbyData, a_unLength);
	req->m_unLength	= a_unLength;
	req->m_pbyData	= p;
	int result = requestSync(ERequestWrite, &req, sizeof(req));
	if (result != 0) {
		delete [] p;
	}
	return result;
}

/**
 *
 */
int CUsbCdcAcm::receive(uint8_t *a_pbyBuffer, uint32_t a_unLength, uint32_t a_unTimeout)
{
	return (int) getRingBuffer()->pullWait(a_pbyBuffer, a_unLength, a_unTimeout);
}

/**
 *
 */
void CUsbCdcAcm::rxHandler(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CInterruptRequest *req = getInterruptRequestManager()->getFree();
	if (!req) {return;}

	getRingBuffer()->push(a_pbyData, a_unLength);
	req->setDataAddress(getRingBuffer());
	req->setLength(sizeof(void *));
	sendMessage(req);
}

/**
 *
 */
void CUsbCdcAcm::txCmpltHandler(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CInterruptRequest *req = getIntRequestManagerTxCmplt()->getFree();
	if (!req) {return;}

	req->setDataAddress(a_pbyData);
	req->setLength(a_unLength);
	sendMessage(req);
}

/**
 *
 */
CUsbCdcAcm *CUsbCdcAcm::newInstance(CHandler *a_pobjHandler)
{
	if (!CUsbCdcAcm::getInstance()) {
		m_pobjDriver = new CUsbCdcAcm(a_pobjHandler);
		assert_param(m_pobjDriver);
	}
	return CUsbCdcAcm::getInstance();
}

/**
 *
 */
CUsbCdcAcm *CUsbCdcAcm::getInstance()
{
	return m_pobjDriver;
}

/**
 *
 */
CUsbCdcAcm::CUsbCdcAcm(CHandler *a_pobjHandler)
	:	CHandlerBase(~0u, 512u, 8u, 6u, NULL, true), m_pobjHandler(a_pobjHandler),
		m_pobjInterruptRequestManager(NULL), m_pobjIntRequestManagerTxCmplt(NULL),
		m_pobjRingBuffer(NULL), m_objExecWrite(this), m_objExecWriteComplete(this),
		m_objExecRead(this)
{
	m_pobjInterruptRequestManager = new CInterruptRequestManager(ERequestRead, sizeof(void *), 64u);
	m_pobjRingBuffer = new CRingBuffComm(OTG_FS_IRQn, 256u);
	m_pobjIntRequestManagerTxCmplt = new CInterruptRequestManager(ERequestWriteComplete, sizeof(void *), 1u);
	assert_param(m_pobjInterruptRequestManager);
	assert_param(m_pobjRingBuffer);
	assert_param(m_pobjIntRequestManagerTxCmplt);
	signalPrepare();
	waitStartUp();
}

/**
 *
 */
CAnalogInput::CAnalogInput(const ADC_TypeDef *a_cpobjAdc,
		const uint32_t a_cnChannel, const uint32_t a_cnTimeout)
	:	CHandlerBase(a_cnTimeout, 128u, 4u, 7u, NULL, true), m_cnChannel(a_cnChannel)
{
	::memset(&m_objAdcHandleTypeDef, 0, sizeof(m_objAdcHandleTypeDef));
	m_objAdcHandleTypeDef.Instance = const_cast<ADC_TypeDef *>(a_cpobjAdc);
	signalPrepare();
	waitStartUp();
}

/**
 *
 */
CAnalogInput::~CAnalogInput()
{
}

/**
 *
 */
int CAnalogInput::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	ADC_ChannelConfTypeDef cfg;
	HAL_StatusTypeDef rc;

	/**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	*/
	m_objAdcHandleTypeDef.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
	m_objAdcHandleTypeDef.Init.Resolution = ADC_RESOLUTION_12B;
	m_objAdcHandleTypeDef.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	m_objAdcHandleTypeDef.Init.ScanConvMode = ADC_SCAN_DISABLE;
	m_objAdcHandleTypeDef.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	m_objAdcHandleTypeDef.Init.LowPowerAutoWait = DISABLE;
	m_objAdcHandleTypeDef.Init.ContinuousConvMode = DISABLE;
	m_objAdcHandleTypeDef.Init.NbrOfConversion = 1;
	m_objAdcHandleTypeDef.Init.DiscontinuousConvMode = DISABLE;
	m_objAdcHandleTypeDef.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	m_objAdcHandleTypeDef.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	m_objAdcHandleTypeDef.Init.DMAContinuousRequests = DISABLE;
	m_objAdcHandleTypeDef.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	m_objAdcHandleTypeDef.Init.OversamplingMode = DISABLE;
	rc = ::HAL_ADC_Init(&m_objAdcHandleTypeDef);
	if (rc != HAL_OK) {
		return rc;
	}

	/**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	cfg.Channel = m_cnChannel;
	cfg.Rank = ADC_REGULAR_RANK_1;
	cfg.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	cfg.SingleDiff = ADC_SINGLE_ENDED;
	cfg.OffsetNumber = ADC_OFFSET_NONE;
	cfg.Offset = 0;
	rc = ::HAL_ADC_ConfigChannel(&m_objAdcHandleTypeDef, &cfg);
	if (rc != HAL_OK) {
		return rc;
	}

	rc = static_cast<HAL_StatusTypeDef>(read());
	if (rc != HAL_OK) {
		return rc;
	}

	return CThreadInterfaceBase::onInitialize();
}

/**
 *
 */
int CAnalogInput::onTerminate()
{
	return 0;
}

/**
 *
 */
int CAnalogInput::onTimeout()
{
	int rc = read();
	return rc;
}

/**
 *
 */
int CAnalogInput::read()
{
	HAL_StatusTypeDef rc;
	rc = ::HAL_ADC_Start(&m_objAdcHandleTypeDef);
	if (rc != HAL_OK) {
		return rc;
	}
	rc = ::HAL_ADC_PollForConversion(&m_objAdcHandleTypeDef, 10);
	if (rc != HAL_OK) {
		return rc;
	}
	m_unAdcValue = ::HAL_ADC_GetValue(&m_objAdcHandleTypeDef);
	return rc;
}
