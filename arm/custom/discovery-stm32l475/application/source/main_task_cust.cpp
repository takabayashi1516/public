/**
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <stm32l4xx_hal.h>

#include <FreeRTOS.h>
#include <task.h>

#include <usbd_ctlreq.h>

#include <framework.h>
#include <main_task.h>

#define DEFAULT_MAINTASK_TIMEOUT (1000u)

// Ctrl+C
#define ETX (0x03)


static void __attribute__((noreturn)) main_assert(void);

static void MX_TIM3_Init(void);
static void setPwmDuty(uint32_t a_unChannel, uint32_t a_unDuty);

/**
 */
CMain	*CMain::m_pobjMain = NULL;

/**
 */
TIM_HandleTypeDef g_objTimHandleTypeDef_3;

/**
 *
 */
void __attribute__((noreturn)) _exit(int code)
{
	::printf("%s: code=%d\n", __PRETTY_FUNCTION__, code);
	::main_assert();
}

/**
 *
 */
void __attribute__((noreturn)) wrap_abort(void)
{
	::printf("%s ...\n", __PRETTY_FUNCTION__);
	::main_assert();
}

/**
 *
 */
void __attribute__((noreturn)) abort(void)
{
	::printf("%s ...\n", __PRETTY_FUNCTION__);
	::main_assert();
}

/**
 *
 */
static void __attribute__((noreturn)) main_assert(void)
{
	for (unsigned i = 0; i < 100; i++) {
		::HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
//		::HAL_Delay(100);
		for (unsigned j = 0u; j < 1000000u; j++) {
		}
	}
	::NVIC_SystemReset();
	/* no return */
	while (true) {}
}

/**
 *
 */
void EXTI0_IRQHandler(void)
{
	::HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/**
 *
 */
void EXTI15_10_IRQHandler(void)
{
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10)) {
		::HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	}
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11)) {
		::HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
	}
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12)) {
		::HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
	}
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13)) {
		::HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
	}
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14)) {
		::HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
	}
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15)) {
		::HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
	}
	return;
}

/**
 *
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	CMain::getSingleton()->interruptSwitch(GPIO_Pin);
}

/**
 *
 */
int startMain(void)
{
	CMain::getSingleton();
	::vTaskStartScheduler();

	return 0;
}

/**
 *
 */
void CSpiRxHandler::rxNotify(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CMain::CExecSpiReceive::CRequest *req =
			new CMain::CExecSpiReceive::CRequest(m_unBus,
					a_pbyData, a_unLength);
	assert_param(req);
	m_pobjOwner->requestAsync(CMain::ERequestSpiReceive, &req,
			sizeof(req));
	delete [] a_pbyData;
}

/**
 *
 */
void CUartRxHandler::rxNotify(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CMain::CExecUartReceive::CRequest *req =
			new CMain::CExecUartReceive::CRequest(m_unChannel,
					a_pbyData, a_unLength);
	assert_param(req);
	m_pobjOwner->requestAsync(CMain::ERequestUartReceive, &req,
			sizeof(req));
}

/**
 *
 */
void CUartRxHandler::errNotify(uint32_t a_unErrorCode)
{
	::printf("l(%4d): %s: error=0x%08x\n", __LINE__, __PRETTY_FUNCTION__, (unsigned) a_unErrorCode);
}

/**
 *
 */
void CUsbCdcAcmRxHandler::rxNotify(uint8_t *a_pbyData, uint32_t a_unLength)
{
	CMain::CExecCdcAcmReceive::CRequest *req =
			new CMain::CExecCdcAcmReceive::CRequest(a_pbyData, a_unLength);
	assert_param(req);
	m_pobjOwner->requestAsync(CMain::ERequestCdcAcmReceive, &req,
			sizeof(req));
}

/**
 *
 */
CMain *CMain::getSingleton()
{
	if (!m_pobjMain) {
		m_pobjMain = new CMain();
		assert_param(m_pobjMain);
	}
	return m_pobjMain;
}

/**
 *
 */
CMain::CMain()
	:	CHandlerBase(DEFAULT_MAINTASK_TIMEOUT, 512u, 384u, 6u, NULL, false),
		m_pobjEdgeObserver(NULL),
		m_pobjI2c1(NULL),
		m_pobjI2c2(NULL),
		m_pobjUart2(NULL),
		m_pobjUart4(NULL),
		m_pobjSpi1(NULL),
		m_pobjSpi2(NULL),
		m_pobjSpi3(NULL),
		m_pobjExecZigBeeReceive(NULL),
		m_pobjExecZigBeeTransmitStatus(NULL),
		m_pobjExecZigBeeAtResponse(NULL),
		m_pobjExecUartReceive(NULL),
		m_pobjExecCdcAcmReceive(NULL),
		m_pobjExecSpiReceive(NULL),
		m_pobjExecInputSwitch(NULL),
		m_pobjIntRqMngrSwtch(NULL),
		m_pobjAiLevelSens(NULL),
		m_objObserver(this),
		m_nCommStatus(ECommStatusDisconnected),
		m_bMainteLedStatus(false)
{
	::memset(m_wStatus, 0, sizeof(m_wStatus));
	setStatus(CMD_VAL_IDX_POWER, true);
	setStatus(CMD_VAL_IDX_PERIOD, DEFAULT_MAINTASK_TIMEOUT);
	setObserver(&m_objObserver);
	m_pobjExecZigBeeReceive = new CExecZigBeeReceive(this);
	m_pobjExecZigBeeTransmitStatus = new CExecZigBeeTransmitStatus(this);
	m_pobjExecZigBeeAtResponse = new CExecZigBeeAtResponse(this);
	m_pobjExecUartReceive = new CExecUartReceive(this);
	m_pobjExecCdcAcmReceive = new CExecCdcAcmReceive(this);
	m_pobjExecSpiReceive = new CExecSpiReceive(this);
	m_pobjExecInputSwitch = new CExecInputSwitch(this);
	m_pobjIntRqMngrSwtch = new CInterruptRequestManager(ERequestInputSwitch,
			sizeof(void *), 8u);
	assert_param(m_pobjExecZigBeeReceive);
	assert_param(m_pobjExecZigBeeTransmitStatus);
	assert_param(m_pobjExecZigBeeAtResponse);
	assert_param(m_pobjExecUartReceive);
	assert_param(m_pobjExecCdcAcmReceive);
	assert_param(m_pobjExecSpiReceive);
	assert_param(m_pobjExecInputSwitch);
	assert_param(m_pobjIntRqMngrSwtch);
}

/**
 *
 */
CMain::~CMain()
{
}

/**
 *
 */
int CMain::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	{
	GPIO_InitTypeDef setting;
	// Configure pin in output push/pull mode
	setting.Pin = GPIO_PIN_9;
	setting.Mode = GPIO_MODE_OUTPUT_PP;
	setting.Speed = GPIO_SPEED_FAST;
	setting.Pull = GPIO_PULLUP;
	::HAL_GPIO_Init(GPIOC, &setting);
	::HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	}

	m_pobjEdgeObserver = new CEdgeObserver(this);
	m_pobjZigBee = new CZigBee(m_pobjEdgeObserver);
	m_pobjI2c1 = new CI2cBus(I2C1);
	m_pobjI2c2 = new CI2cBus(I2C2);

	UART_InitTypeDef uartInitType;
	uartInitType.BaudRate = 115200;
	uartInitType.WordLength = UART_WORDLENGTH_8B;
	uartInitType.StopBits = UART_STOPBITS_1;
	uartInitType.Parity = UART_PARITY_NONE;
	uartInitType.Mode = UART_MODE_TX_RX;
//	uartInitType.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
	uartInitType.HwFlowCtl = USART_CR3_RTSE;
//	uartInitType.HwFlowCtl = UART_HWCONTROL_NONE;
	uartInitType.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	uartInitType.OverSampling = UART_OVERSAMPLING_16;
	m_pobjUart2 = new CUart(USART2, new CUartRxHandler(2u, this));
	m_pobjUart4 = new CUart(UART4, m_pobjZigBee->getInterfaceListener(),
			&uartInitType);

	m_pobjSpi1 = new CSpiBus(SPI1, CSpiBus::EModeSlave, new CSpiRxHandler(1u, this), sizeof(command_t));
	m_pobjSpi2 = new CSpiBus(SPI2, CSpiBus::EModeMaster);
	m_pobjSpi3 = new CSpiBus(SPI3, CSpiBus::EModeMaster);

	(void) CUsbCdcAcm::newInstance(new CUsbCdcAcmRxHandler(this));
	m_pobjZigBee->setInterface(m_pobjUart4);

	m_pobjAiLevelSens = new CAiLevelSens(this, ADC2, ADC_CHANNEL_1, 500, 100);

	::MX_TIM3_Init();

	(void) registerExec(ERequestZigBeeReceive, m_pobjExecZigBeeReceive);
	(void) registerExec(ERequestZigBeeTransmitStatus, m_pobjExecZigBeeTransmitStatus);
	(void) registerExec(ERequestZigBeeAtResponse, m_pobjExecZigBeeAtResponse);
	(void) registerExec(ERequestUartReceive, m_pobjExecUartReceive);
	(void) registerExec(ERequestCdcAcmReceive, m_pobjExecCdcAcmReceive);
	(void) registerExec(ERequestSpiReceive, m_pobjExecSpiReceive);
	(void) registerExec(ERequestInputSwitch, m_pobjExecInputSwitch);

	::HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	::HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	uint8_t *p = new uint8_t[sizeof(command_t)];
	getSpi(1)->receive(p, sizeof(command_t));

	::HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

	return CThreadInterfaceBase::onInitialize();
}

/**
 *
 */
int	CMain::onTerminate()
{
	delete getAiLevelSens();
	delete getIntRqMngrSwtch();

	delete m_pobjExecInputSwitch;
	delete m_pobjExecSpiReceive;
	delete m_pobjExecCdcAcmReceive;
	delete m_pobjExecUartReceive;
	delete m_pobjExecZigBeeTransmitStatus;
	delete m_pobjExecZigBeeAtResponse;
	delete m_pobjExecZigBeeReceive;

	delete m_pobjZigBee;
	delete m_pobjSpi1;
	delete m_pobjSpi2;
	delete m_pobjSpi3;
	delete m_pobjUart2;
	delete m_pobjUart4;
	delete m_pobjI2c1;
	delete m_pobjI2c2;
	delete m_pobjEdgeObserver;

	return 0;
}

/**
 *
 */
void CMain::interruptSwitch(uint32_t a_unSwitch)
{
	CInterruptRequest *req = getIntRqMngrSwtch()->getFree();
	if (!req) {return;}

	req->setDataAddress(reinterpret_cast<void *>(a_unSwitch));
	req->setLength(sizeof(void *));
	sendMessage(req);
}

/**
 *
 */
CI2cBus	*CMain::getI2c(int bus) const
{
	if (bus == 1) {
		return m_pobjI2c1;
	}
	if (bus == 2) {
		return m_pobjI2c2;
	}
	return NULL;
}

/**
 *
 */
CUart	*CMain::getUart(int ch) const
{
	if (ch == 2) {
		return m_pobjUart2;
	}
	if (ch == 4) {
		return m_pobjUart4;
	}
	return NULL;
}

/**
 *
 */
CSpiBus	*CMain::getSpi(int bus) const
{
	if (bus == 1) {
		return m_pobjSpi1;
	}
	if (bus == 2) {
		return m_pobjSpi2;
	}
	if (bus == 3) {
		return m_pobjSpi3;
	}
	return NULL;
}

/**
 *
 */
CUsbCdcAcm *CMain::getUsbCdcAcm() const
{
	return CUsbCdcAcm::getInstance();
}

/**
 *
 */
CZigBee	*CMain::getZigBee() const
{
	return m_pobjZigBee;
}

/**
 *
 */
int	CMain::onTimeout()
{
//	::HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
	switch (getCommStatus()) {
	case ECommStatusDisconnected:
		::setPwmDuty(TIM_CHANNEL_2, 400);
		break;
	case ECommStatusMaintenance:
		::setPwmDuty(TIM_CHANNEL_1, (m_bMainteLedStatus)? 0 : 600);
		m_bMainteLedStatus = !m_bMainteLedStatus;
		break;
	default:
		break;
	}

	postDataToEdge();

	switch (getCommStatus()) {
	case ECommStatusDisconnected:
		::setPwmDuty(TIM_CHANNEL_2, 0);
		break;
	default:
		break;
	}
	return 0;
}

/**
 *
 */
void CMain::postDataToEdge()
{
	command_t cmd;
	cmd.type = CMD_TYPE_GET;
	cmd.pattern = BIT(CMD_VAL_IDX_POWER) |
			BIT(CMD_VAL_IDX_PERIOD) | BIT(CMD_VAL_IDX_SWITCH) |
			BIT(CMD_VAL_IDX_ADC(0)) | BIT(CMD_VAL_IDX_PWM(0));
	::memcpy(&(cmd.value), &m_wStatus, sizeof(cmd.value));
	command_t res;
	makeResponse(&res, &cmd, true);
	for (int i = 0; i < EInterfaceSupremum; i++) {
		sendCommand(&res, static_cast<EInterface>(i));
	}
}

/**
 *
 */
void CMain::makeResponse(command_t *a_pobjResponse, command_t *a_pobjCommand, bool a_bAuto/* = false*/)
{
	uint32_t rc = 0;

	::printf("l(%4d): %s: auto=%d\n", __LINE__, __PRETTY_FUNCTION__, a_bAuto);

	::memset(a_pobjResponse, 0, sizeof(*a_pobjResponse));
	a_pobjResponse->type = a_pobjCommand->type | ((a_bAuto)? BIT(CMD_TYPE_BIT_DIR) : BIT(CMD_TYPE_BIT_RES));
	if (!a_bAuto) {
		a_pobjResponse->pattern |= BIT(CMD_VAL_IDX_RESULT);
	}

	if (a_pobjCommand->pattern & BIT(CMD_VAL_IDX_POWER)) {
		if (a_pobjCommand->type & CMD_TYPE_SET) {
			setStatus(CMD_VAL_IDX_POWER, a_pobjCommand->value[CMD_VAL_IDX_POWER]);
			if (!getStatus(CMD_VAL_IDX_POWER)) {
				::NVIC_SystemReset();
				// no return
			}
		}
		if (a_pobjCommand->type & CMD_TYPE_GET) {
			a_pobjResponse->pattern |= BIT(CMD_VAL_IDX_POWER);
		}
	}

	if (a_pobjCommand->pattern & BIT(CMD_VAL_IDX_PERIOD)) {
		if (a_pobjCommand->type & CMD_TYPE_SET) {
			setStatus(CMD_VAL_IDX_PERIOD, a_pobjCommand->value[CMD_VAL_IDX_PERIOD]);
			setTimeout(getStatus(CMD_VAL_IDX_PERIOD));
		}
		if (a_pobjCommand->type & CMD_TYPE_GET) {
			a_pobjResponse->pattern |= BIT(CMD_VAL_IDX_PERIOD);
		}
	}

	if (a_pobjCommand->pattern & BIT(CMD_VAL_IDX_SWITCH)) {
		if (a_pobjCommand->type & CMD_TYPE_SET) {
			rc = -1;
		}
		if (a_pobjCommand->type & CMD_TYPE_GET) {
			a_pobjResponse->pattern |= BIT(CMD_VAL_IDX_SWITCH);
		}
	}

	for (int n = 0; n < CMD_VAL_IDX_ADC_BITS; n++) {
		if (a_pobjCommand->pattern & BIT(CMD_VAL_IDX_ADC(n))) {
			if (a_pobjCommand->type & CMD_TYPE_SET) {
//				setStatus(CMD_VAL_IDX_ADC(n), a_pobjCommand->value[CMD_VAL_IDX_ADC(n)]);
				rc = -1;
			}
			if (a_pobjCommand->type & CMD_TYPE_GET) {
				a_pobjResponse->pattern |= BIT(CMD_VAL_IDX_ADC(n));
			}
		}
	}

	for (uint32_t n = 0; n < CMD_VAL_IDX_PWM_BITS; n++) {
		const uint32_t channels[CMD_VAL_IDX_PWM_BITS] = {
			TIM_CHANNEL_1,
			TIM_CHANNEL_2,
			~0u,
			~0u
		};
		if ((a_pobjCommand->pattern & BIT(CMD_VAL_IDX_PWM(n))) && (channels[n] != ~0u)) {
			if (a_pobjCommand->type & CMD_TYPE_SET) {
				setStatus(CMD_VAL_IDX_PWM(n), a_pobjCommand->value[CMD_VAL_IDX_PWM(n)]);
				::setPwmDuty(channels[n], getStatus(CMD_VAL_IDX_PWM(n)));
			}
			if (a_pobjCommand->type & CMD_TYPE_GET) {
				a_pobjResponse->pattern |= BIT(CMD_VAL_IDX_PWM(n));
			}
		}
	}

	setStatus(CMD_VAL_IDX_RESULT, rc);
	::memcpy(a_pobjResponse->value, m_wStatus, sizeof(m_wStatus));

	CUtil::dump(reinterpret_cast<uint8_t *>(a_pobjResponse), sizeof(*a_pobjResponse));
}

/**
 *
 */
void CMain::sendCommand(command_t *a_pobjCommand, EInterface a_nInterface)
{
	::printf("l(%4d): %s: if=%u: pattern=%04x, type=%04x\n", __LINE__,
			__PRETTY_FUNCTION__, a_nInterface, a_pobjCommand->pattern,
			a_pobjCommand->type);
	switch (a_nInterface) {
	case EInterfaceSpi:
		getSpi(1)->pushSlaveBuffer(reinterpret_cast<uint8_t *>(a_pobjCommand));
		getSpi(1)->getSlaveBuffer()->update();
		break;
	case EInterfaceZigBee:
		getZigBee()->transmitToCoodinater(1u, reinterpret_cast<uint8_t *>(a_pobjCommand), sizeof(*a_pobjCommand));
		break;
	case EInterfaceCdc:
		getUsbCdcAcm()->transmit(reinterpret_cast<uint8_t *>(a_pobjCommand), sizeof(*a_pobjCommand));
		break;
	default:
		break;
	}
}

/**
 *
 */
int CMain::CExecZigBeeReceive::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	m_pobjOwner->setCommStatus(CMain::ECommStatusConnected);
	command_t *cmd = reinterpret_cast<command_t *>(req->m_pbyData);

	::printf("l(%4d): %s: len=%lu\n", __LINE__, __PRETTY_FUNCTION__, req->m_wLength);
	CUtil::dump(req->m_pbyData, req->m_wLength);

	command_t res;
	m_pobjOwner->makeResponse(&res, cmd);
	m_pobjOwner->sendCommand(&res, EInterfaceZigBee);
	delete req;
	return 0;
}

/**
 *
 */
int CMain::CExecZigBeeTransmitStatus::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	::printf("l(%4d): %s: frame_id=%02x, delivery_status=%02x, discovery_status=%02x\n",
			__LINE__, __PRETTY_FUNCTION__, req->m_byFrameId, req->m_byDeliveryStatus,
			req->m_byDiscoveryStatus);
	m_pobjOwner->setCommStatus((req->m_byDeliveryStatus == 0)?
			CMain::ECommStatusConnected : CMain::ECommStatusDisconnected);
	switch (m_pobjOwner->getCommStatus()) {
	case CMain::ECommStatusDisconnected:
		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 0);
		m_pobjOwner->setTimeout(DEFAULT_MAINTASK_TIMEOUT);
		m_pobjOwner->setStatus(CMD_VAL_IDX_PERIOD, DEFAULT_MAINTASK_TIMEOUT);
		break;
	default:
		break;
	}
	delete req;
	return 0;
}

/**
 *
 */
int CMain::CExecZigBeeAtResponse::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	::printf("l(%4d): %s: frame-id=%02x, status=%02x, len=%d\n",
			__LINE__, __PRETTY_FUNCTION__, req->m_byFrameId, req->m_byStatus, req->m_hwLength);
	CUtil::dump(req->m_pbyValue, req->m_hwLength);

	const int buf_size = 128;
	char *buf = new char[buf_size];
	::sprintf(buf, "frame_id=%02x, status=%02x, length=%d\r\n",
			req->m_byFrameId, req->m_byStatus, req->m_hwLength);
	while (m_pobjOwner->getUsbCdcAcm()->transmit(reinterpret_cast<uint8_t *>(buf),
			::strlen(buf)) != 0) {}
	::memset(buf, 0, buf_size);
	if (req->m_byStatus == 0u) {
		char *p = buf;
		for (uint16_t i = 0u; i < req->m_hwLength; i++, p += 2) {
			::sprintf(p, "%02x", req->m_pbyValue[i]);
		}
	}
	::strcat(buf, "\r\n$ ");
	while (m_pobjOwner->getUsbCdcAcm()->transmit(reinterpret_cast<uint8_t *>(buf),
			::strlen(buf)) != 0) {}
	delete buf;
	delete req;
	return 0;
}

/**
 *
 */
int CMain::CExecSpiReceive::doExec(const void *a_lpParam, const int a_cnLength)
{
	uint8_t *p = new uint8_t[sizeof(command_t)];

	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	m_pobjOwner->getSpi(req->m_unBus)->receive(p, sizeof(command_t));

	::printf("l(%4d): %s: len=%lu\n", __LINE__, __PRETTY_FUNCTION__, req->m_wLength);
	CUtil::dump(req->m_pbyData, req->m_wLength);

	command_t *cmd = reinterpret_cast<command_t *>(req->m_pbyData);

	command_t res;
	m_pobjOwner->makeResponse(&res, cmd);
	m_pobjOwner->sendCommand(&res, EInterfaceSpi);

	delete req;
	return 0;
}

/**
 *
 */
int CMain::CExecUartReceive::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	if (!(m_pobjOwner->getZigBee()->getMainteInterface())) {
		m_pobjOwner->setTimeout(DEFAULT_MAINTASK_TIMEOUT);
		m_pobjOwner->setStatus(CMD_VAL_IDX_PERIOD, DEFAULT_MAINTASK_TIMEOUT);
		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 0);
		m_pobjOwner->setCommStatus(CMain::ECommStatusMaintenance);
		m_pobjOwner->getZigBee()->setMainteMode(
				m_pobjOwner->getUart(req->m_unChannel));
	}
	m_pobjOwner->getZigBee()->transmitRaw(req->m_pbyData, req->m_wLength);
	delete req;
	return 0;
}

/**
 *
 */
int CMain::CExecCdcAcmReceive::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
	for (uint32_t i = 0u; i < req->m_wLength; i++) {
		if ((req->m_pbyData)[i] == ETX) {
			::NVIC_SystemReset();
			/* no return */
		}
	}
	m_pobjOwner->getUsbCdcAcm()->transmit(req->m_pbyData, req->m_wLength);
	::memcpy(&m_byBuffer[m_hwPoolLength], req->m_pbyData, req->m_wLength);
	m_hwPoolLength += req->m_wLength;
	if ((m_byBuffer[m_hwPoolLength - 1] == '\r') ||
			(m_byBuffer[m_hwPoolLength - 1] == '\n')) {
		m_byBuffer[m_hwPoolLength - 1] = '\0';
		m_hwPoolLength--;
		if (m_hwPoolLength > 0u) {
			m_pobjOwner->getZigBee()->atCommand(0x0a, m_byBuffer, m_hwPoolLength);
			m_hwPoolLength = 0;
		}
	}
	delete req;
	return 0;
}

/**
 *
 */
int CMain::CExecInputSwitch::doExec(const void *a_lpParam, const int a_cnLength)
{
	if (a_cnLength != sizeof(CInterruptRequest::CRequest)) {
	}
	CInterruptRequest::CRequest *req = reinterpret_cast<
			CInterruptRequest::CRequest *>(const_cast<void *>(a_lpParam));
	uint32_t sw = reinterpret_cast<uint32_t>(req->m_pbyData);
	m_pobjOwner->getIntRqMngrSwtch()->release(req);
	::printf("l(%4d): %s: switch=%08lx\n", __LINE__, __PRETTY_FUNCTION__, sw);

	// get port status
	GPIO_PinState sts = ::HAL_GPIO_ReadPin(GPIOC, sw);

	m_pobjOwner->setTimeout(DEFAULT_MAINTASK_TIMEOUT);
	m_pobjOwner->setStatus(CMD_VAL_IDX_PERIOD, DEFAULT_MAINTASK_TIMEOUT);

	if (GPIO_PIN_13 == sw) {
		uint32_t upd = (sts == GPIO_PIN_RESET)? (m_pobjOwner->getStatus(CMD_VAL_IDX_SWITCH) | sw) :
				(m_pobjOwner->getStatus(CMD_VAL_IDX_SWITCH) & (~sw));
		m_pobjOwner->setStatus(CMD_VAL_IDX_SWITCH, upd);

		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 20);

		::HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (sts == GPIO_PIN_SET)?
				GPIO_PIN_RESET : GPIO_PIN_SET);
	}
	if (GPIO_PIN_0 == sw) {
		uint32_t upd = (sts == GPIO_PIN_RESET)? (m_pobjOwner->getStatus(CMD_VAL_IDX_SWITCH) | sw) :
				(m_pobjOwner->getStatus(CMD_VAL_IDX_SWITCH) & (~sw));
		m_pobjOwner->setStatus(CMD_VAL_IDX_SWITCH, upd);

		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 10);

		if (m_pobjOwner->getZigBee()->getMainteInterface()) {
			m_pobjOwner->getZigBee()->setMainteMode(NULL);
			return 0;
		}
	}
	m_pobjOwner->postDataToEdge();

	if (GPIO_PIN_13 == sw) {
		// draft
		uint32_t upd = (m_pobjOwner->getStatus(CMD_VAL_IDX_SWITCH) & (~sw));
		m_pobjOwner->setStatus(CMD_VAL_IDX_SWITCH, upd);
	}

	return 0;
}

/**
 *
 */
CMain::CObserver::CObserver(CMain *a_pobjOwner): m_pobjOwner(a_pobjOwner)
{
}

/**
 *
 */
void CMain::CObserver::onPreviousProcess(
		CThreadInterfaceBase *a_lpobjInterface,
		const int a_cnRequest)
{
	::HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
	switch (m_pobjOwner->getCommStatus()) {
	case CMain::ECommStatusDisconnected:
		::setPwmDuty(TIM_CHANNEL_2, 400);
		break;
	default:
		break;
	}
}

/**
 *
 */
void CMain::CObserver::onCompleted(CThreadInterfaceBase *a_lpobjInterface,
		const int a_cnRequest, const int a_cnResult)
{
	::HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
	switch (m_pobjOwner->getCommStatus()) {
	case CMain::ECommStatusDisconnected:
		::setPwmDuty(TIM_CHANNEL_2, 0);
		break;
	default:
		break;
	}
}

/**
 *
 */
void CMain::CObserver::onCanceled(CThreadInterfaceBase *a_lpobjInterface,
		const int a_cnRequest)
{
}

/**
 *
 */
void CEdgeObserver::onReceive(uint8_t a_byDstAddr64[8], uint16_t a_hwDstAddr16,
		uint8_t *a_pbyData, uint16_t a_hwLength)
{
	CMain::CExecZigBeeReceive::CRequest *req =
			new CMain::CExecZigBeeReceive::CRequest(a_pbyData, a_hwLength);
	CUtil::dump(req->m_pbyData, req->m_wLength);
	getOwner()->requestAsync(CMain::ERequestZigBeeReceive, &req, sizeof(req));
}

/**
 *
 */
void CEdgeObserver::onTransmitResult(uint8_t a_byFrameId,
		uint8_t a_byDeliveryStatus, uint8_t a_byDiscoveryStatus)
{
	CMain::CExecZigBeeTransmitStatus::CRequest *req =
			new CMain::CExecZigBeeTransmitStatus::CRequest(a_byFrameId,
					a_byDeliveryStatus, a_byDiscoveryStatus);
	getOwner()->requestAsync(CMain::ERequestZigBeeTransmitStatus, &req, sizeof(req));
}

/**
 *
 */
void CEdgeObserver::onAtResponse(uint8_t a_byFrameId, uint8_t a_byStatus,
				uint8_t *a_pbyValue, uint16_t a_hwLength)
{
	CMain::CExecZigBeeAtResponse::CRequest *req =
			new CMain::CExecZigBeeAtResponse::CRequest(a_byFrameId,
					a_byStatus, a_hwLength, a_pbyValue);
	getOwner()->requestAsync(CMain::ERequestZigBeeAtResponse, &req, sizeof(req));
}

/**
 *
 */
void CEdgeObserver::onCommand(uint8_t *a_pbyCommand, uint16_t a_hwLength)
{
	::printf("l(%4d): %s: frame-type=%02x, len=%u\n",
			__LINE__, __PRETTY_FUNCTION__, *a_pbyCommand, a_hwLength);
}

/**
 *
 */
static void MX_TIM3_Init(void)
{
	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	g_objTimHandleTypeDef_3.Instance = TIM3;
	g_objTimHandleTypeDef_3.Init.Prescaler = 0;
	g_objTimHandleTypeDef_3.Init.CounterMode = TIM_COUNTERMODE_UP;
	g_objTimHandleTypeDef_3.Init.Period = 1024;
	g_objTimHandleTypeDef_3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	g_objTimHandleTypeDef_3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (::HAL_TIM_PWM_Init(&g_objTimHandleTypeDef_3) != HAL_OK) {
		::Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (::HAL_TIMEx_MasterConfigSynchronization(&g_objTimHandleTypeDef_3, &sMasterConfig) != HAL_OK) {
		::Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (::HAL_TIM_PWM_ConfigChannel(&g_objTimHandleTypeDef_3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
		::Error_Handler();
	}
	if (::HAL_TIM_PWM_ConfigChannel(&g_objTimHandleTypeDef_3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
		::Error_Handler();
	}
	if (::HAL_TIM_PWM_ConfigChannel(&g_objTimHandleTypeDef_3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
		::Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
	::HAL_TIM_MspPostInit(&g_objTimHandleTypeDef_3);
}

/**
 *
 */
static void setPwmDuty(uint32_t a_unChannel, uint32_t a_unDuty)
{
	if (::HAL_TIM_PWM_Stop(&g_objTimHandleTypeDef_3, a_unChannel) != HAL_OK) {
	}

	TIM_OC_InitTypeDef sConfigOC;
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = a_unDuty;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (::HAL_TIM_PWM_ConfigChannel(&g_objTimHandleTypeDef_3, &sConfigOC,
			a_unChannel) != HAL_OK) {
	}

	if (a_unDuty == 0u) {
		return;
	}

	if (::HAL_TIM_PWM_Start(&g_objTimHandleTypeDef_3, a_unChannel) != HAL_OK) {
	}
}

/**
 *
 */
CAiLevelSens::CAiLevelSens(CMain *a_objOwner, const ADC_TypeDef *a_cpobjAdc,
			const uint32_t a_cnChannel, const uint32_t a_cnTimeout,
			const uint32_t a_cnAdcValueThreshold)
	:	CAnalogInput(a_cpobjAdc, a_cnChannel, a_cnTimeout),
			m_objOwner(a_objOwner), m_cnAdcValueThreshold(a_cnAdcValueThreshold)
{
}

/**
 *
 */
int CAiLevelSens::onTimeout()
{
	int rc = CAnalogInput::onTimeout();
	::printf("l(%4d): %s: rc=%d, val=%lu\n", __LINE__, __PRETTY_FUNCTION__, rc, getValue());
	m_objOwner->setStatus(CMD_VAL_IDX_ADC(0), getValue());
	// TODO: change adc value to lux.
	::HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, (getValue() < m_cnAdcValueThreshold)?
			GPIO_PIN_SET : GPIO_PIN_RESET);
	return rc;
}
