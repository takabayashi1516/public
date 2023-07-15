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
#include <command.h>

#define DEFAULT_MAINTASK_TIMEOUT (2000u)
#define ETX (0x03)

// #define __TEST_EXTRA

#ifdef __TEST_EXTRA
class CTestObserver : public CThreadFrame::CObserver {
public:
	///
	CTestObserver(CSpiBus *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
	///
	void onPreviousProcess(CThreadInterfaceBase *a_lpobjInterface,
			const int a_cnRequest) {
		::printf("l(%4d): %s: inst=%p: spi mode=%d, req=%d\n", __LINE__, __PRETTY_FUNCTION__,
				m_pobjOwner, m_pobjOwner->getMode(), a_cnRequest);
	}
	///
	void onCompleted(CThreadInterfaceBase *a_lpobjInterface,
			const int a_cnRequest, const int a_cnResult) {
		::printf("l(%4d): %s: inst=%p: spi mode=%d, req=%d, result=%d\n", __LINE__, __PRETTY_FUNCTION__,
				m_pobjOwner, m_pobjOwner->getMode(), a_cnRequest, a_cnResult);
	}
	///
	void onCanceled(CThreadInterfaceBase *a_lpobjInterface,
			const int a_cnRequest) {}
private:
	///
	CSpiBus *m_pobjOwner;
};
static void __testExtra(void)
{
	static command_t cmd = {
		0x0001,
		0xffff,
		{
			0x11223344,
			0x55667788,
			0x11223344,
			0x55667788,
			0x11223344,
			0x55667788,
			0x11223344
		}
	};
	command_t res;
	int result;

	::memset(&res, 0, sizeof(res));
	::printf("l(%4d): %s: spi ***\n", __LINE__, __PRETTY_FUNCTION__);
	result = CMain::getSingleton()->getSpi(2)->transmitReceive(CSpiBus::EChannel0,
			(uint8_t *)&cmd, (uint8_t *)&res, sizeof(cmd), false);
	::printf("l(%4d): %s: spi result=%d\n", __LINE__, __PRETTY_FUNCTION__, result);
	CUtil::dump((uint8_t *)&res, sizeof(res));

	if (res.type) {
		::memcpy(&cmd, &res, sizeof(command_t));
	}

#if 0
	result = CMain::getSingleton()->getSpi(1)->transmitReceive(CSpiBus::EChannel0,
			(uint8_t *)&cmd, (uint8_t *)&res, sizeof(res));
	::printf("l(%4d): %s: spi result=%d\n", __LINE__, __PRETTY_FUNCTION__, result);
	CUtil::dump((uint8_t *)&res, sizeof(res));

	::memset(&res, 0, sizeof(res));
	result = CMain::getSingleton()->getUart(2)->transmit(
			(uint8_t *)&cmd, sizeof(cmd));
	::printf("l(%4d): %s: uart result=%d\n", __LINE__, __PRETTY_FUNCTION__, result);
	result = CMain::getSingleton()->getUart(2)->receive((uint8_t *)&res, sizeof(res), 300);
	::printf("l(%4d): %s: spi result=%d\n", __LINE__, __PRETTY_FUNCTION__, result);
	CUtil::dump((uint8_t *)&res, sizeof(res));

	::memset(&res, 0, sizeof(res));
	::printf("l(%4d): %s: spi ***\n", __LINE__, __PRETTY_FUNCTION__);
	result = CMain::getSingleton()->getSpi(2)->transmitReceive(CSpiBus::EChannel0,
			NULL, (uint8_t *)&res, sizeof(res));
	::printf("l(%4d): %s: spi result=%d\n", __LINE__, __PRETTY_FUNCTION__, result);
	CUtil::dump((uint8_t *)&res, sizeof(res));
#endif
}
#endif /* __TEST_EXTRA */

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
	:	CHandlerBase(DEFAULT_MAINTASK_TIMEOUT, 128u, 384u, 6u, NULL, false),
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
	::memset(m_wDataToHost, 0, sizeof(m_wDataToHost));
	::memset(m_wDataFromHost, 0, sizeof(m_wDataFromHost));
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
	signalPrepare();
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
	::HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
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
	m_pobjUart2 = new CUart(USART2, /*new CUartRxHandler(2u, this)*/NULL);
	m_pobjUart4 = new CUart(UART4, m_pobjZigBee->getInterfaceListener(),
			&uartInitType);

	m_pobjSpi1 = new CSpiBus(SPI1, CSpiBus::EModeSlave, CSpiBus::ESsCntrlModeHardware,
			new CSpiRxHandler(1u, this), sizeof(command_t));
	m_pobjSpi2 = new CSpiBus(SPI2, CSpiBus::EModeMaster);
	m_pobjSpi3 = new CSpiBus(SPI3, CSpiBus::EModeMaster);
#ifdef __TEST_EXTRA
	CTestObserver *obsrv = new CTestObserver(getSpi(1));
	getSpi(1)->setObserver(obsrv);
#endif /* __TEST_EXTRA */
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

	uint8_t *p = new uint8_t[sizeof(command_t)];
	getSpi(1)->receive(p, sizeof(command_t));

#ifdef __TEST_EXTRA
	uint8_t adr[] = {0x0F, 0x00};
	int rc = getI2c(2)->receive(0xbe >> 1, adr, 1, &adr[1], 1);
	::printf("data=%02x, rc=%d\n", adr[1], rc);
	adr[1] = 0x80;
	rc = getI2c(2)->transmit(0xbe >> 1, adr, 1);
	::printf("rc=%d\n", rc);
	adr[1] = 0xcc;
	rc = getI2c(2)->receive(0xbe >> 1, adr, 1, &adr[1], 1);
	::printf("data=%02x, rc=%d\n", adr[1], rc);
#endif /* __TEST_EXTRA */

	::HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);

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
	::HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
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

#ifdef __TEST_EXTRA
	__testExtra();
#endif /* __TEST_EXTRA */
	return 0;
}

/**
 *
 */
void CMain::postDataToEdge()
{
#if 1
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);
#endif
	uint32_t val = getAiLevelSens()->getValue();
	makeDataToEdge(GET_IDX_LUMINANCE, val);

	command_t res;
	res.type = COMMAND_TYPE_GET;
	res.pattern = BIT(GET_IDX_RESULT) | BIT(GET_IDX_TEMPERETURE) |
			BIT(GET_IDX_PRESSURE) | BIT(GET_IDX_HUMIDITY) |
			BIT(GET_IDX_LUMINANCE);
	::memcpy(&(res.value), &m_wDataToHost, sizeof(res.value));
	getSpi(1)->pushSlaveBuffer(reinterpret_cast<uint8_t *>(&res));
	getSpi(1)->getSlaveBuffer()->update();
	getZigBee()->transmitToCoodinater(1u, (uint8_t*)&res, sizeof(res));
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

	if (cmd->type == COMMAND_TYPE_NOTIF) {
		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 0);
		if (cmd->pattern & BIT(NOTIF_IDX_POWEROFF)) {
			::NVIC_SystemReset();
			/* no return */
		}
		m_pobjOwner->setCommStatus(CMain::ECommStatusDisconnected);
		m_pobjOwner->setTimeout(DEFAULT_MAINTASK_TIMEOUT);
		delete req;
		return 0;
	}

	if (cmd->pattern & BIT(SET_IDX_PERIOD)) {
		m_pobjOwner->setDataFromHost(SET_IDX_PERIOD, cmd->value[SET_IDX_PERIOD]);
		m_pobjOwner->setTimeout(m_pobjOwner->getDataFromHost(SET_IDX_PERIOD));
	}
	if (cmd->pattern & BIT(SET_IDX_PWM1)) {
		m_pobjOwner->setDataFromHost(SET_IDX_PWM1, cmd->value[SET_IDX_PWM1]);
		::setPwmDuty(TIM_CHANNEL_1, m_pobjOwner->getDataFromHost(SET_IDX_PWM1));
	}
	if (cmd->pattern & BIT(SET_IDX_PWM2)) {
		m_pobjOwner->setDataFromHost(SET_IDX_PWM2, cmd->value[SET_IDX_PWM2]);
		::setPwmDuty(TIM_CHANNEL_2, m_pobjOwner->getDataFromHost(SET_IDX_PWM2));
	}

	if (cmd->pattern & BIT(SET_IDX_REQUEST)) {
		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 0);
		m_pobjOwner->postDataToEdge();
	}

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

	char buf[128];
	::sprintf(buf, "frame_id=%02x, status=%02x, length=%d\r\n",
			req->m_byFrameId, req->m_byStatus, req->m_hwLength);
	while (m_pobjOwner->getUsbCdcAcm()->transmit(reinterpret_cast<uint8_t *>(buf),
			::strlen(buf)) != 0) {}
	::memset(buf, 0, sizeof(buf));
	if (req->m_byStatus == 0u) {
		char *p = buf;
		for (uint16_t i = 0u; i < req->m_hwLength; i++, p += 2) {
			::sprintf(p, "%02x", req->m_pbyValue[i]);
		}
	}
	::strcat(buf, "\r\n$ ");
	while (m_pobjOwner->getUsbCdcAcm()->transmit(reinterpret_cast<uint8_t *>(buf),
			::strlen(buf)) != 0) {}
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

#ifndef __TEST_EXTRA
	if (cmd->type == COMMAND_TYPE_NOTIF) {
		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 0);
		m_pobjOwner->setTimeout(DEFAULT_MAINTASK_TIMEOUT);
		delete req;
		return 0;
	}

	if (cmd->pattern & BIT(SET_IDX_PERIOD)) {
		m_pobjOwner->setDataFromHost(SET_IDX_PERIOD, cmd->value[SET_IDX_PERIOD]);
		m_pobjOwner->setTimeout(m_pobjOwner->getDataFromHost(SET_IDX_PERIOD));
	}
	if (cmd->pattern & BIT(SET_IDX_PWM1)) {
		m_pobjOwner->setDataFromHost(SET_IDX_PWM1, cmd->value[SET_IDX_PWM1]);
		::setPwmDuty(TIM_CHANNEL_1, m_pobjOwner->getDataFromHost(SET_IDX_PWM1));
	}
	if (cmd->pattern & BIT(SET_IDX_PWM2)) {
		m_pobjOwner->setDataFromHost(SET_IDX_PWM2, cmd->value[SET_IDX_PWM2]);
		::setPwmDuty(TIM_CHANNEL_2, m_pobjOwner->getDataFromHost(SET_IDX_PWM2));
	}

	if (cmd->pattern & BIT(SET_IDX_REQUEST)) {
		::setPwmDuty(TIM_CHANNEL_1, 0);
		::setPwmDuty(TIM_CHANNEL_2, 0);
		m_pobjOwner->postDataToEdge();
	}
#else /* defined(__TEST_EXTRA) */
//	m_pobjOwner->getSpi(req->m_unBus)->transmit(CSpiBus::EChannel0, req->m_pbyData, req->m_wLength, true);
	cmd->type++;
	cmd->pattern--;
	m_pobjOwner->getSpi(req->m_unBus)->pushSlaveBuffer(req->m_pbyData);
	m_pobjOwner->getSpi(req->m_unBus)->getSlaveBuffer()->update();
#endif /* __TEST_EXTRA */

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

#if 0
	::setPwmDuty(TIM_CHANNEL_1, 0);
	::setPwmDuty(TIM_CHANNEL_2, 0);
	m_pobjOwner->setTimeout(DEFAULT_MAINTASK_TIMEOUT);
	if (m_pobjOwner->getZigBee()->getMainteInterface()) {
		m_pobjOwner->getZigBee()->setMainteMode(NULL);
		return 0;
	}
	m_pobjOwner->postDataToEdge();
#endif
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
//	::HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
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
//	::HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
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
	// TODO: change adc value to lux.
	::HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, (getValue() < m_cnAdcValueThreshold)?
			GPIO_PIN_SET : GPIO_PIN_RESET);
	return rc;
}
