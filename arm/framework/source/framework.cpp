/**
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(STM32F401xE)
#include <stm32f4xx.h>
#elif defined(STM32L475xx)
#include <stm32l4xx.h>
#else
#error no support cpu.
#endif

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#include <framework.h>

/**
 *
 */
void	CUtil::dump(const void *a_clpParam, size_t a_nCount)
{
#ifdef	DEBUG_DUMP
	unsigned char *lpbyBuffer = reinterpret_cast<unsigned char *>(
			const_cast<void *>(a_clpParam));
	size_t rest = a_nCount;
	const int columns = 16;
	int index = 0;
	::printf("      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f\n");
	::printf("-----------------------------------------------------\n");
	for (; rest > 0 && a_nCount >= rest;
			rest -= columns, lpbyBuffer += columns, index += columns) {
		::printf("%04x| ", index);
		unsigned char buff[columns];
		::memset(buff, 0, sizeof(buff));
		size_t tmp_size = columns;
		if (rest < columns) {
			tmp_size = rest;
		}
		::memcpy(buff, lpbyBuffer, tmp_size);
		::printf(
				"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				buff[0], buff[1], buff[2], buff[3],
				buff[4], buff[5], buff[6], buff[7],
				buff[8], buff[9], buff[10], buff[11],
				buff[12], buff[13], buff[14], buff[15]);
	}
	::printf(
			"----------------------------------------------------- %u(0x%04x)bytes\n",
			a_nCount, a_nCount);
#endif	// DEBUG_DUMP
}

/**
 *
 */
uint32_t	CUtil::getTickCount()
{
	if (!CContext::inInterrupt()) {
		return ::xTaskGetTickCount();
	}
	return ::xTaskGetTickCountFromISR();
}

/**
 *
 */
CSemaphore::CSemaphore(bool a_bInitSignal)
{
	m_hResource = ::xSemaphoreCreateBinary();
	if (a_bInitSignal) {
		signal();
	}
}

/**
 *
 */
CSemaphore::~CSemaphore()
{
	::vSemaphoreDelete(m_hResource);
}

/**
 *
 */
bool CSemaphore::wait(uint32_t a_unTimeout)
{
	BaseType_t result;
	if (a_unTimeout == ~0u) {
		a_unTimeout = portMAX_DELAY;
	}
	if (!CContext::inInterrupt()) {
		result = ::xSemaphoreTake(m_hResource, a_unTimeout);
	} else {
		result = ::xSemaphoreTakeFromISR(m_hResource, NULL);
	}
	return (result == pdTRUE);
}

/**
 *
 */
void CSemaphore::signal()
{
	if (!CContext::inInterrupt()) {
		::xSemaphoreGive(m_hResource);
	} else {
		::xSemaphoreGiveFromISR(m_hResource, NULL);
	}
}

/**
 *
 */
CThreadInterfaceBase::CLock::CLock(bool a_bInitSignalStatus)
	:	CSemaphore(a_bInitSignalStatus)
{
}

/**
 *
 */
CThreadInterfaceBase::CLock::~CLock()
{
}

/**
 *
 */
void	CThreadInterfaceBase::CLock::lock()
{
	wait(~0u);
}

/**
 *
 */
void	CThreadInterfaceBase::CLock::unlock()
{
	signal();
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::CRequestMessageBase(
		const int a_cnRequest, CResponseMessage *a_lpobjResponse)
	:	m_cnRequest(a_cnRequest),
		m_lpobjResponse(a_lpobjResponse),
		m_lpobjObserver(NULL),
		m_cnLength(0),
		m_lpbyContents(NULL)
{
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::CRequestMessageBase(
		const int a_cnRequest, const void *a_clpContents, const int a_cnLength,
		CResponseMessage *a_lpobjResponse)
	:	m_cnRequest(a_cnRequest),
		m_lpobjResponse(a_lpobjResponse),
		m_lpobjObserver(NULL),
		m_cnLength(a_cnLength),
		m_lpbyContents(reinterpret_cast<uint8_t *>(const_cast<void *>(a_clpContents)))
{
	if (m_cnLength <= 0) {
		return;
	}
	if (m_lpobjResponse) {
		return;
	}
	int l = _RNDUP_REMINDER(m_cnLength, 4);
	m_lpbyContents = new uint8_t[l];
	assert_param(m_lpbyContents);
	if (!m_lpbyContents || !a_clpContents) {
		return;
	}
	::memset(m_lpbyContents, 0, l);
	::memcpy(m_lpbyContents, a_clpContents, m_cnLength);
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::CRequestMessageBase(
		CRequestMessageBase& a_objMessageBase)
	:	m_cnRequest(a_objMessageBase.m_cnRequest),
		m_lpobjResponse(a_objMessageBase.m_lpobjResponse),
		m_lpobjObserver(NULL),
		m_cnLength(a_objMessageBase.m_cnLength),
		m_lpbyContents(a_objMessageBase.m_lpbyContents)
{
	if (m_cnLength <= 0) {
		return;
	}
	if (m_lpobjResponse) {
		return;
	}
	int l = _RNDUP_REMINDER(m_cnLength, 4);
	m_lpbyContents = new uint8_t[l];
	assert_param(m_lpbyContents);
	if (!m_lpbyContents || !(a_objMessageBase.m_lpbyContents)) {
		return;
	}
	::memset(m_lpbyContents, 0, l);
	::memcpy(m_lpbyContents, a_objMessageBase.m_lpbyContents, m_cnLength);
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::~CRequestMessageBase()
{
	if (m_lpobjResponse || !m_lpbyContents) {
		return;
	}
	delete [] m_lpbyContents;
}

/**
 *
 */
bool CThreadFrameBase::CRequestMessageBase::isStatic()
{
	return false;
}

/**
 *
 */
CThreadFrameBase::CRequestMessageTerminete::CRequestMessageTerminete() :
		CRequestMessageBase(CThreadFrameBase::ETerminate,
				new CThreadFrame::CResponseMessage())
{
}

/**
 *
 */
CThreadFrameBase::CRequestMessageTerminete::~CRequestMessageTerminete()
{
	delete m_lpobjResponse;
}

/**
 *
 */
CThreadFrameBase::CMessageTick::CMessageTick() :
		CRequestMessageBase(CThreadFrameBase::ETick)
{
}

/**
 *
 */
CThreadFrameBase::CRequestMessageSetTimeout::CRequestMessageSetTimeout(
		uint32_t a_unTimeout)
	:	CRequestMessageBase(CThreadFrameBase::ESetTimeout),
				m_unTimeout(a_unTimeout)
{
}

/**
 *
 */
CThreadFrameBase::CResponseMessage::CResponseMessage()
{
	m_nXferHandle = ::xQueueCreate(1, sizeof(void *));
}

/**
 *
 */
CThreadFrameBase::CResponseMessage::~CResponseMessage()
{
	::vQueueDelete(getXferHandle(EXferRead));
}

/**
 *
 */
int	CThreadFrameBase::CResponseMessage::reply(void *a_lpParam)
{
	int nResult = 0;
	if (!CContext::inInterrupt()) {
		nResult = ::xQueueSend(getXferHandle(EXferWrite), &a_lpParam, portMAX_DELAY);
	} else {
		nResult = ::xQueueSendFromISR(getXferHandle(EXferWrite), &a_lpParam, NULL);
	}
	return(nResult);
}

/**
 *
 */
void	*CThreadFrameBase::CResponseMessage::wait(uint32_t a_unTimeout/* = ~0u*/)
{
	void *pResult = NULL;
	if (a_unTimeout == ~0u) {
		a_unTimeout = portMAX_DELAY;
	}
	(void) ::xQueueReceive(getXferHandle(EXferRead), &pResult, a_unTimeout);
	return(pResult);
}

/**
 *
 */
int	CThreadFrameBase::CResponseMessage::replyResult(const int a_cnResult)
{
	int *result = new int();
	assert_param(result);
	if (!result) {
		return -1;
	}
	*result = a_cnResult;
	int nResult = reply(result);
	return(nResult);
}

/**
 *
 */
int	CThreadFrameBase::CResponseMessage::waitResult(uint32_t a_unTimeout/* = ~0u*/)
{
	int *result = reinterpret_cast<int *>(wait(a_unTimeout));
	int rc = *result;
	delete result;
	return(rc);
}

/**
 *
 */
void	CThreadFrameBase::setThreadInterfaceBase(
		CThreadInterfaceBase& a_objThreadInterface)
{
	m_lpobjThreadInterface = &a_objThreadInterface;
}

/**
 *
 */
CThreadFrameBase::CThreadFrameBase(CThreadInterfaceBase& a_objThreadInterface,
		uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
		uint32_t a_unQueues, uint32_t a_unPriority)
	: 	m_lpobjThreadInterface(&a_objThreadInterface),
			m_unRecvTimeout(a_unRecvTimeout)
{
	int	nResult = 0;

	m_nXferHandle = ::xQueueCreate(a_unQueues, sizeof(void *));

	::memset(m_byExtra, 0, sizeof(m_byExtra));
	::sprintf(reinterpret_cast<char *>(m_byExtra), "th_%p", this);

	nResult = ::xTaskCreate(mainRoutine, reinterpret_cast<char *>(m_byExtra),
			a_unStagesOfStack, this, a_unPriority, &m_hThread);
	if (nResult)	{
	}
/*	::printf("l(%4d): %s: extra=%s, xfer=%p, h=%p, result=%d\n",
			__LINE__, __PRETTY_FUNCTION__, reinterpret_cast<char *>(m_byExtra),
			m_nXferHandle, m_hThread, nResult); */
}

/**
 *
 */
CThreadFrameBase::~CThreadFrameBase()
{
	::vQueueDelete(getXferHandle(EXferRead));
}

/**
 *
 */
void	CThreadFrameBase::mainRoutine(void *a_lpParam)
{
	(void) reinterpret_cast<CThreadFrameBase *>(a_lpParam)->main();
	return;
}

/**
 *
 */
void	*CThreadFrameBase::main()
{
//	::printf("l(%4d): %s: frame=%p\n", __LINE__, __PRETTY_FUNCTION__, this);
	int nResult = m_lpobjThreadInterface->onInitialize();
	if (nResult) {
		return(NULL);
	}
	uint32_t timeout = m_unRecvTimeout;
	m_unPrevTickCount = CUtil::getTickCount();

	int nRequest;
	do {
		uint32_t tick = 0;
		CRequestMessageBase *lpMsg = receiveMessage((m_unRecvTimeout != ~0u)? timeout : m_unRecvTimeout);
		if (!lpMsg) {
			break;
		}
//		::printf("l(%4d): %s: frame=%p: start\n", __LINE__, __PRETTY_FUNCTION__, this);
		nRequest = lpMsg->m_cnRequest;
		if (lpMsg->m_lpobjObserver) {
			lpMsg->m_lpobjObserver->onPreviousProcess(m_lpobjThreadInterface, nRequest);
		}
		switch (nRequest) {
		case ETerminate:
			nResult = m_lpobjThreadInterface->onTerminate();
			if (nResult) {
				nRequest = 0;	// clear ETerminate
			}
			break;
		case ETick:
			m_unPrevTickCount = CUtil::getTickCount();
			nResult = m_lpobjThreadInterface->onTimeout();
			tick = CUtil::getTickCount();
			timeout = m_unRecvTimeout - (tick - m_unPrevTickCount);
			break;
		case ESetTimeout:
			if (getReceiveTimeout() != reinterpret_cast<CRequestMessageSetTimeout *>(lpMsg)->m_unTimeout) {
				setReceiveTimeout(reinterpret_cast<CRequestMessageSetTimeout *>(lpMsg)->m_unTimeout);
				m_unPrevTickCount = CUtil::getTickCount();
				timeout = m_unRecvTimeout;
				break;
			}
			tick = CUtil::getTickCount();
			timeout = m_unRecvTimeout - (tick - m_unPrevTickCount);
			break;
		default:
			nResult = m_lpobjThreadInterface->onJudgeStatus(
					lpMsg->m_cnRequest, lpMsg->m_lpbyContents, lpMsg->m_cnLength);
			if (nResult == 0) {
				nResult = m_lpobjThreadInterface->onMessage(
						lpMsg->m_cnRequest, lpMsg->m_lpbyContents, lpMsg->m_cnLength);
			}
			tick = CUtil::getTickCount();
			timeout = m_unRecvTimeout - (tick - m_unPrevTickCount);
			break;
		}
		if (lpMsg) {
			if (lpMsg->m_lpobjObserver) {
				lpMsg->m_lpobjObserver->onCompleted(m_lpobjThreadInterface, nRequest, nResult);
			}
			if (lpMsg->m_lpobjResponse) {
				lpMsg->m_lpobjResponse->replyResult(nResult);
//			::printf("l(%4d): %s: frame=%p: end\n", __LINE__, __PRETTY_FUNCTION__, this);
				continue;	/* if exist response then the caller manage buffer. */
			}
			if (!(lpMsg->isStatic())) {
				delete lpMsg;
			}
		}
//		::printf("l(%4d): %s: frame=%p: end\n", __LINE__, __PRETTY_FUNCTION__, this);
	} while (ETerminate != nRequest);

	return(NULL);
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase	*CThreadFrameBase::receiveMessage(
		uint32_t a_unTimeout)
{
	int	nResult = 0;
	HANDLE	h = getXferHandle(EXferRead);
	if (a_unTimeout == ~0u) {
		a_unTimeout = portMAX_DELAY;
	}
	CRequestMessageBase *msg;
	nResult = ::xQueueReceive(h, reinterpret_cast<void *>(&msg), a_unTimeout);
	if (nResult != pdTRUE) {
		CMessageTick *tick = new CMessageTick();
		tick->m_lpobjObserver = m_lpobjThreadInterface->getObserver();
		return(tick);
	}
	return(msg);
}

/**
 *
 */
int	CThreadFrameBase::join(void*& a_lpParam)
{
	return(0);
}

/**
 *
 */
void	CThreadFrame::CObserver::onPreviousProcess(CThreadInterfaceBase *a_lpobjInterface,
		const int a_cnRequest)
{
	/* override to use */
}

/**
 *
 */
void	CThreadFrame::CObserver::onCompleted(CThreadInterfaceBase *a_lpobjInterface,
		const int a_cnRequest, const int a_cnResult)
{
	/* override to use */
}

/**
 *
 */
void	CThreadFrame::CObserver::onCanceled(CThreadInterfaceBase *a_lpobjInterface,
		const int a_cnRequest)
{
	/* override to use */
}

/**
 *
 */
CThreadFrame::CThreadFrame(CThreadInterfaceBase& a_objThreadInterface,
		uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
		uint32_t a_unQueues, uint32_t a_unPriority)
	:	CThreadFrameBase(a_objThreadInterface, a_unRecvTimeout, a_unStagesOfStack,
				a_unQueues, a_unPriority)
{
}

/**
 *
 */
CThreadFrame::~CThreadFrame()
{
}

/**
 *
 */
CThreadFrame	*CThreadFrame::newInstance(
		CThreadInterfaceBase& a_objThreadInterface,
		uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
		uint32_t a_unQueues, uint32_t a_unPriority)
{
	return(new CThreadFrame(a_objThreadInterface, a_unRecvTimeout,
			a_unStagesOfStack, a_unQueues, a_unPriority));
}

/**
 *
 */
void	CThreadFrame::deleteInstance(CThreadFrame *a_objThreadFrame)
{
	delete a_objThreadFrame;
}

/**
 *
 */
int	CThreadInterfaceBase::sendMessage(
		CThreadFrameBase::CRequestMessageBase *a_lpobjMessage)
{
	int nResult = -1;
	if (!CContext::inInterrupt()) {
		nResult = ::xQueueSend((m_lpobjThreadFrame->getXferHandle(
				CThreadFrameBase::EXferWrite)), &a_lpobjMessage, portMAX_DELAY);
		nResult =  (pdTRUE == nResult)? 0 : -1;
		if ((0 == nResult) && a_lpobjMessage->m_lpobjResponse) {
			nResult = a_lpobjMessage->m_lpobjResponse->waitResult();
		}
		return(nResult);
	}
	nResult = ::xQueueSendFromISR((m_lpobjThreadFrame->getXferHandle(
			CThreadFrameBase::EXferWrite)), &a_lpobjMessage, NULL);
	nResult =  (pdTRUE == nResult)? 0 : -1;
	return(nResult);
}

/**
 *
 */
int	CThreadInterfaceBase::sendMessage(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength,
		CThreadFrame::CResponseMessage *a_lpobjResponse,
		CThreadFrameBase::CObserverBase *a_lpobjObserver)
{
	int nResult = 0;
	if (a_lpobjResponse) {
		CThreadFrameBase::CRequestMessageBase msg(a_cnRequest, a_lpParam,
				a_cnLength, a_lpobjResponse);
		msg.m_lpobjObserver = a_lpobjObserver;
		nResult = sendMessage(&msg);
		return(nResult);
	}
	CThreadFrameBase::CRequestMessageBase *pmsg =
			new CThreadFrameBase::CRequestMessageBase(a_cnRequest, a_lpParam,
					a_cnLength, a_lpobjResponse);
	pmsg->m_lpobjObserver = a_lpobjObserver;
	nResult = sendMessage(pmsg);
	return(nResult);
}

/**
 *
 */
CThreadInterfaceBase::CThreadInterfaceBase(uint32_t a_unRecvTimeout,
		uint32_t a_unStagesOfStack, uint32_t a_unQueues, uint32_t a_unPriority,
		bool a_bBlocking)
	:	m_pobjBlockStartup(NULL)
{
	if (a_bBlocking) {
		m_pobjBlockStartup = new CSemaphore();
	}
	m_lpobjThreadFrame = CThreadFrame::newInstance(*this, a_unRecvTimeout,
			a_unStagesOfStack, a_unQueues, a_unPriority);
}

/**
 *
 */
#if 0
CThreadInterfaceBase::CThreadInterfaceBase(CThreadInterfaceBase& a_objThread, bool a_bBlocking)
	:	m_pobjBlockStartup(NULL)
{
	if (a_bBlocking) {
		m_pobjBlockStartup = new CSemaphore();
	}
	m_lpobjThreadFrame = a_objThread.m_lpobjThreadFrame;
	m_lpobjThreadFrame->setThreadInterfaceBase(*this);
}
#endif

/**
 *
 */
CThreadInterfaceBase::~CThreadInterfaceBase()
{
	if (m_pobjBlockStartup) {
		delete m_pobjBlockStartup;
	}
	CThreadFrame::deleteInstance(m_lpobjThreadFrame);
}

/**
 *
 */
void	CThreadInterfaceBase::waitStartUp()
{
	if (m_pobjBlockStartup) {
		m_pobjBlockStartup->wait();
		delete m_pobjBlockStartup;
		m_pobjBlockStartup = NULL;
	}
}

/**
 *
 */
void	CThreadInterfaceBase::signalStartUp()
{
	if (m_pobjBlockStartup) {
		m_pobjBlockStartup->signal();
	}
}

/**
 *
 */
int	CThreadInterfaceBase::terminate()
{
	CThreadFrameBase::CRequestMessageTerminete msg;
	int nResult = sendMessage(&msg);
	if (nResult) {
		return(nResult);
	}
	void *pResult = NULL;
	nResult = m_lpobjThreadFrame->join(pResult);
	return(nResult);
}

/**
 *
 */
int	CThreadInterfaceBase::setTimeout(uint32_t a_unTimeout)
{
	CThreadFrameBase::CRequestMessageSetTimeout *msg =
			new CThreadFrameBase::CRequestMessageSetTimeout(a_unTimeout);
	int nResult = sendMessage(msg);
	return(nResult);
}

/**
 *
 */
int	CThreadInterfaceBase::requestSync(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength,
		CThreadFrame::CObserver *a_lpobjObserver)
{
	CThreadFrame::CResponseMessage rmsg;
	int nResult = sendMessage(a_cnRequest, a_lpParam, a_cnLength,
			&rmsg, (a_lpobjObserver)? a_lpobjObserver : m_lpobjObserver);
	return(nResult);
}

/**
 *
 */
int	CThreadInterfaceBase::requestAsync(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength,
		CThreadFrame::CObserver *a_lpobjObserver)
{
	return(sendMessage(a_cnRequest, a_lpParam, a_cnLength,
			NULL, (a_lpobjObserver)? a_lpobjObserver : m_lpobjObserver));
}

/**
 *
 */
void	CThreadInterfaceBase::setObserver(
		CThreadFrame::CObserver *a_lpobjObserver)
{
	m_lpobjObserver = a_lpobjObserver;
}

/**
 *
 */
int	CThreadInterfaceBase::onInitialize()
{
//	::printf("l(%4d): %s: if=%p, frame=%p\n", __LINE__, __PRETTY_FUNCTION__, this, m_lpobjThreadFrame);
	signalStartUp();
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onTerminate()
{
	/* override to use */
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onTimeout()
{
	/* override to use */
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onMessage(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	/* override to use */
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onJudgeStatus(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	/* override to use */
	return(0);
}

/**
 *
 */
CHandlerBase::CExecMap::CExecMap(const int a_cnRequest) :
		m_cnRequest(a_cnRequest), m_nExecItems(0), m_lpobjExecArray(NULL)
{
}

/**
 *
 */
CHandlerBase::CExecMap::~CExecMap()
{
	if (m_lpobjExecArray) {
		delete [] m_lpobjExecArray;
	}
}

/**
 *
 */
int	CHandlerBase::CExecMap::add(CExecBase *a_lpobjExec)
{
	if (!m_lpobjExecArray) {
		m_lpobjExecArray = new pExecBase_t[m_nExecItems + 1];
		m_lpobjExecArray[m_nExecItems] = a_lpobjExec;
		m_nExecItems++;
	} else {
		pExecBase_t *buff = new pExecBase_t[m_nExecItems];
		::memcpy(buff, m_lpobjExecArray,
				sizeof(CExecBase *) * m_nExecItems);
		delete [] m_lpobjExecArray;
		m_lpobjExecArray = new pExecBase_t[m_nExecItems + 1];
		::memcpy(m_lpobjExecArray, buff,
				sizeof(CExecBase *) * m_nExecItems);
		delete [] buff;
		m_lpobjExecArray[m_nExecItems] = a_lpobjExec;
		m_nExecItems++;
	}
	return(0);
}

/**
 *
 */
int	CHandlerBase::CExecMap::remove(CExecBase *a_lpobjExec)
{
	int nResult = -1;
	for (int i = 0; i < m_nExecItems; i++) {
		if (m_lpobjExecArray[i] == a_lpobjExec) {
			nResult = 0;
			if (--m_nExecItems) {
				pExecBase_t *buff = new pExecBase_t[m_nExecItems];
				::memcpy(buff, m_lpobjExecArray, sizeof(CExecBase *) * i);
				if (i < m_nExecItems) {
					::memcpy(&buff[i], &m_lpobjExecArray[i + 1],
							sizeof(CExecBase *) * (m_nExecItems - i));
				}
				delete [] m_lpobjExecArray;
				m_lpobjExecArray = new pExecBase_t[m_nExecItems];
				::memcpy(m_lpobjExecArray, buff,
						sizeof(CExecBase *) * m_nExecItems);
				delete [] buff;
			} else {
				delete [] m_lpobjExecArray;
				m_lpobjExecArray = NULL;
			}
			break;
		}
	}
	return(nResult);
}

/**
 *
 */
int	CHandlerBase::CExecMap::broadCast(const void *a_lpParam,
		const int a_cnLength)
{
	int nResult = 0;
	for (int i = 0; i < m_nExecItems; i++) {
		int result = m_lpobjExecArray[i]->doExec(
				a_lpParam, a_cnLength);
		if (!result) continue;
		nResult = result;
	}
	return(nResult);
}

/**
 *
 */
CHandlerBase::CHandlerBase(uint32_t a_unRecvTimeout,
		uint32_t a_unStagesOfStack, uint32_t a_unQueues, uint32_t a_unPriority,
		CThreadFrame::CObserver *a_lpobjObserver, bool a_bBlocking)
	:	CThreadInterfaceBase(a_unRecvTimeout, a_unStagesOfStack,
				a_unQueues, a_unPriority, a_bBlocking),
			m_lpobjExecMap(NULL),
			m_nExecMapItems(0)
{
	setObserver(a_lpobjObserver);
}

/**
 *
 */
CHandlerBase::~CHandlerBase()
{
	if (m_lpobjExecMap) {
		for (int i = 0; i < m_nExecMapItems; i++) {
			delete m_lpobjExecMap[i];
		}
		delete [] m_lpobjExecMap;
	}
}

/**
 *
 */
int	CHandlerBase::onMessage(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	int nResult = 0;
	getLock()->lock();
	CExecMap *map = searchExecMap(a_cnRequest);
	if (map) {
		nResult = map->broadCast(a_lpParam, a_cnLength);
	}
	getLock()->unlock();
	return(nResult);
}

/**
 * call only on onInitialize event
 */
int	CHandlerBase::registerExec(const int a_cnRequest,
		CExecBase *a_lpobjExec)
{
	getLock()->lock();
	if (!m_lpobjExecMap) {
		m_lpobjExecMap = new pExecMap_t[m_nExecMapItems + 1];
		m_lpobjExecMap[m_nExecMapItems] = new CExecMap(a_cnRequest);
		m_lpobjExecMap[m_nExecMapItems]->add(a_lpobjExec);
		m_nExecMapItems++;
	} else {
		CExecMap *map = searchExecMap(a_cnRequest);
		if (map) {
			map->add(a_lpobjExec);
		} else {
			pExecMap_t *buff = new pExecMap_t[m_nExecMapItems];
			::memcpy(buff, m_lpobjExecMap,
					sizeof(CExecMap *) * m_nExecMapItems);
			delete [] m_lpobjExecMap;
			m_lpobjExecMap = new pExecMap_t[m_nExecMapItems + 1];
			::memcpy(m_lpobjExecMap, buff,
					sizeof(CExecMap *) * m_nExecMapItems);
			delete [] buff;
			m_lpobjExecMap[m_nExecMapItems] = new CExecMap(a_cnRequest);
			m_lpobjExecMap[m_nExecMapItems]->add(a_lpobjExec);
			m_nExecMapItems++;
		}
	}
	getLock()->unlock();
	return(0);
}

/**
 * call only on onTerminate event
 */
int	CHandlerBase::deregisterExec(const int a_cnRequest,
		CExecBase *a_lpobjExec)
{
	int nResult = -1;
	getLock()->lock();
	CExecMap *map = searchExecMap(a_cnRequest);
	if (map) {
		map->remove(a_lpobjExec);
		nResult = 0;
	}
	getLock()->unlock();
	return(nResult);
}

/**
 *
 */
CHandlerBase::CExecMap	*CHandlerBase::searchExecMap(
		const int a_cnRequest)
{
	if (m_lpobjExecMap) {
		for (int i = 0; i < m_nExecMapItems; i++) {
			if (m_lpobjExecMap[i]->getRequest() == a_cnRequest) {
				return(m_lpobjExecMap[i]);
			}
		}
	}
	return(NULL);
}

/**
 *
 */
CInterruptRequest::CInterruptRequest(const uint32_t a_cunRequestNo,
		const uint32_t a_cunDataLength)
	:	CThreadFrameBase::CRequestMessageBase(a_cunRequestNo, NULL, sizeof(CRequest *)),
		m_cunMaxLength(a_cunDataLength), m_bUsed(false)
{
	m_objRequest.m_unLength = 0u;
	m_objRequest.m_pbyData = new uint8_t[_RNDUP_REMINDER(m_cunMaxLength, 4)];
	m_lpbyContents = reinterpret_cast<uint8_t *>(&m_objRequest);
}

/**
 *
 */
CInterruptRequest::~CInterruptRequest()
{
	delete [] (m_objRequest.m_pbyData);
}

/**
 *
 */
bool CInterruptRequest::isStatic()
{
	return true;
}

/**
 *
 */
CInterruptRequestManager::CInterruptRequestManager(const uint32_t a_cunRequestNo,
		const uint32_t a_cunDataLength,
		const uint32_t a_cnInterruptRequests)
	:	m_pobjIntReqs(NULL), m_cnIntReqs(a_cnInterruptRequests)
{
	m_pobjIntReqs = new pInterruptRequest_t[m_cnIntReqs];
	for (uint32_t i = 0; i < m_cnIntReqs; i++) {
		m_pobjIntReqs[i] = new CInterruptRequest(a_cunRequestNo, a_cunDataLength);
	}
}

/**
 *
 */
CInterruptRequestManager::~CInterruptRequestManager()
{
	for (uint32_t i = 0; i < m_cnIntReqs; i++) {
		delete m_pobjIntReqs[i];
	}
	delete [] m_pobjIntReqs;
}

/**
 *
 */
CInterruptRequest*& CInterruptRequestManager::getFree()
{
	CInterruptRequest*& result = m_pobjIntReqs[0];
	for (uint32_t i = 0; i < m_cnIntReqs; i++) {
		if (!(m_pobjIntReqs[i]->isUsed())) {
			result = m_pobjIntReqs[i];
			result->setUseState();
			return result;
		}
	}
	assert_param(NULL);
	return result;
}

/**
 *
 */
void CInterruptRequestManager::release(CInterruptRequest::CRequest *a_pobjRequest)
{
	for (uint32_t i = 0; i < m_cnIntReqs; i++) {
		if ((m_pobjIntReqs[i]->getRequest() == a_pobjRequest)) {
			m_pobjIntReqs[i]->setFreeState();
			break;
		}
	}
}

/**
 *
 */
CRingBuffer::CRingBuffer(const uint32_t a_cunSize)
	:	m_pobjLock(new CThreadInterfaceBase::CLock()),
			m_pbyWrAddr(NULL), m_pbyRdAddr(NULL),
			m_cpbyBaseAddr(new uint8_t[_RNDUP_REMINDER(a_cunSize, 4)]),
			m_cunSize(a_cunSize), m_pobjSemaphore(NULL)
{
	setWriteAddress(getBaseAddress());
	setReadAddress(getBaseAddress());
	m_pobjSemaphore = new CSemaphore();
}

/**
 *
 */
CRingBuffer::~CRingBuffer()
{
	delete m_pobjSemaphore;
	delete [] m_cpbyBaseAddr;
	delete m_pobjLock;
}

/**
 *
 */
bool CRingBuffer::push(uint8_t *a_pbyData, uint32_t a_unLength)
{
	bool result = false;
	uint8_t *p;
	lock();
	if (a_unLength > getFreeLength()) {
		unLock();
		return result;
	}
	p = getWriteAddress();
	::memcpy(p, a_pbyData, a_unLength);
	p += a_unLength;
	setWriteAddress(p);
	result = true;
	unLock();
	notify();
	return result;
}

/**
 *
 */
bool CRingBuffer::pull(uint8_t *a_pbyBuffer, uint32_t a_unRequireLength,
		uint32_t *a_punProvidedLength)
{
	bool result = false;
	bool iniFlag = false;
	lock();
	if (getValidLength() <= 0u) {
		goto end;
	}
	if (getValidLength() <= a_unRequireLength) {
		*a_punProvidedLength = getValidLength();
		iniFlag = true;
	} else {
		*a_punProvidedLength = a_unRequireLength;
	}
	::memcpy(a_pbyBuffer, getReadAddress(), *a_punProvidedLength);
	if (iniFlag) {
		setWriteAddress(getBaseAddress());
		setReadAddress(getBaseAddress());
	} else {
		setReadAddress(&(getReadAddress()[*a_punProvidedLength]));
	}
	result = true;

end:
	unLock();
	return result;
}

/**
 *
 */
uint32_t CRingBuffer::pullWait(uint8_t *a_pbyBuffer, uint32_t a_unLength,
		uint32_t a_unTimeout)
{
	bool result = false;
	uint32_t rcvLen = 0u;
	uint32_t rcvTotalLen = 0u;
	uint32_t start = CUtil::getTickCount();
	do {
		result = pull(&a_pbyBuffer[rcvLen], a_unLength - rcvLen, &rcvLen);
		if (result) {
			rcvTotalLen += rcvLen;
			if (rcvLen == a_unLength) {
				break;
			}
		}
		if (a_unTimeout != ~0u) {
			a_unTimeout -= (CUtil::getTickCount() - start);
		}
		result = wait(a_unTimeout);
	} while (result);
	return rcvTotalLen;
}

/**
 *
 */
bool CRingBuffer::getData(uint8_t*& a_pbyBuffer, uint32_t& a_unLength)
{
	bool result = false;
	uint32_t l = 0;
	lock();
	if (getValidLength() <= 0) {
		goto end;
	}
	a_unLength = getValidLength();
	l = _RNDUP_REMINDER(a_unLength, 4);
	a_pbyBuffer = new uint8_t[l];
	assert_param(a_pbyBuffer);
	if (!a_pbyBuffer) {
		goto end;
	}
	::memset(a_pbyBuffer, 0, l);
	::memcpy(a_pbyBuffer, getReadAddress(), a_unLength);
	setWriteAddress(getBaseAddress());
	setReadAddress(getBaseAddress());
	result = true;
end:
	unLock();
	return result;
}

/**
 *
 */
void CRingBuffer::lock()
{
	bool isInt = CContext::inInterrupt();
	if (isInt) {
		return;
	}
	onRequireLock();
	m_pobjLock->lock();
}

/**
 *
 */
void CRingBuffer::unLock()
{
	bool isInt = CContext::inInterrupt();
	if (isInt) {
		return;
	}
	m_pobjLock->unlock();
	onRequireUnLock();
}

/**
 *
 */
void CRingBuffer::notify()
{
	m_pobjSemaphore->signal();
}

/**
 *
 */
bool CRingBuffer::wait(uint32_t a_unTimeout)
{
	return m_pobjSemaphore->wait(a_unTimeout);
}

/**
 *
 */
uint32_t CRingBuffer::getFreeLength()
{
	return static_cast<uint32_t>(getEndAddress() - getWriteAddress());
}

/**
 *
 */
uint32_t CRingBuffer::getValidLength()
{
	return static_cast<uint32_t>(getWriteAddress() - getReadAddress());
}

/**
 *
 */
CTripleBuffer::CQuadStateBuffer::CQuadStateBuffer(const uint32_t a_cunSize)
	:	m_cunSize(a_cunSize), m_nState(EQuadStateInvalid),
		m_pbyBuffer(NULL)
{
	m_pbyBuffer = new uint8_t[m_cunSize];
	assert_param(m_pbyBuffer);
}

/**
 *
 */
CTripleBuffer::CQuadStateBuffer::~CQuadStateBuffer()
{
	if (getAddress()) {
		delete [] getAddress();
	}
}

/**
 *
 */
bool CTripleBuffer::CQuadStateBuffer::update(uint8_t *a_pbyBuffer)
{
	if ((getState() > EQuadStateValid)) {
		return false;
	}
	m_nState = EQuadStateValid;
	::memcpy(getAddress(), a_pbyBuffer, m_cunSize);
	return true;
}

/**
 *
 */
bool CTripleBuffer::CQuadStateBuffer::preSelect()
{
	if (getState() != EQuadStateValid) {
		return false;
	}
	m_nState = EQuadStateNextUse;
	return true;
}

/**
 *
 */
bool CTripleBuffer::CQuadStateBuffer::select()
{
	if (getState() < EQuadStateValid) {
		return false;
	}
	m_nState = EQuadStateInUse;
	return true;
}

/**
 *
 */
bool CTripleBuffer::CQuadStateBuffer::deselect()
{
	if (getState() < EQuadStateInUse) {
		return false;
	}
	m_nState = EQuadStateInvalid;
	return true;
}

/**
 *
 */
CTripleBuffer::EQuadState CTripleBuffer::CQuadStateBuffer::getState() const
{
	return m_nState;
}

/**
 *
 */
uint8_t *CTripleBuffer::CQuadStateBuffer::getAddress() const
{
	return m_pbyBuffer;
}

/**
 *
 */
CTripleBuffer::CTripleBuffer(const uint32_t a_cunSize)
{
	m_pobjLock = new CThreadInterfaceBase::CLock();
	assert_param(m_pobjLock);
	for (unsigned i = 0; i < (sizeof(m_pobjQuadStateBuffer) / sizeof(m_pobjQuadStateBuffer[0])); i++) {
		m_pobjQuadStateBuffer[i] = new CQuadStateBuffer(a_cunSize);
		assert_param(m_pobjQuadStateBuffer[i]);
	}
}

/**
 *
 */
CTripleBuffer::~CTripleBuffer()
{
	for (unsigned i = 0; i < (sizeof(m_pobjQuadStateBuffer) / sizeof(m_pobjQuadStateBuffer[0])); i++) {
		if (!m_pobjQuadStateBuffer[i]) {
			continue;
		}
		delete m_pobjQuadStateBuffer[i];
	}
	if (m_pobjLock) {
		delete m_pobjLock;
	}
}

/**
 *
 */
void CTripleBuffer::push(uint8_t *a_pbyData)
{
	onRequireLock();
	for (uint32_t i = 0u; getQuadStateBuffer(i); i++) {
		if (getQuadStateBuffer(i)->update(a_pbyData)) {
			break;
		}
	}
	sort();
	onRequireUnLock();
}

/**
 *
 */
uint8_t *CTripleBuffer::update()
{
	uint8_t *result = NULL;
	onRequireLock();
	if (getQuadStateBuffer(0)->getState() == EQuadStateInvalid) {
		goto err_proc;
	}
	if (getQuadStateBuffer(0)->getState() == EQuadStateValid) {
		getQuadStateBuffer(0)->select();
		goto end_proc;
	}
	if (getQuadStateBuffer(0)->getState() == EQuadStateNextUse) {
		if (getQuadStateBuffer(1)->getState() == EQuadStateValid) {
			getQuadStateBuffer(0)->preSelect();
		}
		getQuadStateBuffer(0)->select();
		goto end_proc;
	}
	if (getQuadStateBuffer(0)->getState() == EQuadStateInUse) {
		if (getQuadStateBuffer(1)->getState() == EQuadStateInvalid) {
			goto end_proc;
		}
		getQuadStateBuffer(0)->deselect();
		getQuadStateBuffer(1)->select();
		if (getQuadStateBuffer(2)->getState() == EQuadStateInvalid) {
			goto end_proc;
		}
		getQuadStateBuffer(2)->preSelect();
		goto end_proc;
	}
end_proc:
	sort();
	result = getQuadStateBuffer(0)->getAddress();
err_proc:
	onRequireUnLock();
	return result;
}

/**
 *
 */
void CTripleBuffer::onRequireLock()
{
	lock();
}

/**
 *
 */
void CTripleBuffer::onRequireUnLock()
{
	unlock();
}

/**
 *
 */
const uint32_t CTripleBuffer::getNumOfBuffer() const
{
	return (uint32_t) ENumOfBuffer;
}

/**
 *
 */
CTripleBuffer::CQuadStateBuffer *CTripleBuffer::getQuadStateBuffer(uint32_t a_unIndex)
{
	if (a_unIndex >= getNumOfBuffer()) {
		return NULL;
	}
	return m_pobjQuadStateBuffer[a_unIndex];
}

/**
 *
 */
void CTripleBuffer::lock()
{
	if (!CContext::inInterrupt()) {
		m_pobjLock->lock();
	}
}

/**
 *
 */
void CTripleBuffer::unlock()
{
	if (!CContext::inInterrupt()) {
		m_pobjLock->unlock();
	}
}

/**
 *
 */
void CTripleBuffer::sort()
{
//	::printf("l(%4d): %s: %d %d %d\n", __LINE__, __PRETTY_FUNCTION__,
//			getQuadStateBuffer(0)->getState(),
//			getQuadStateBuffer(1)->getState(),
//			getQuadStateBuffer(2)->getState());
	::qsort(m_pobjQuadStateBuffer, getNumOfBuffer(), sizeof(CQuadStateBuffer *),
			sortCompare);
//	::printf("l(%4d): %s: %d %d %d\n", __LINE__, __PRETTY_FUNCTION__,
//			getQuadStateBuffer(0)->getState(),
//			getQuadStateBuffer(1)->getState(),
//			getQuadStateBuffer(2)->getState());
}

/**
 *
 */
int CTripleBuffer::sortCompare(const void *a_pParam1, const void *a_pParam2)
{
	CQuadStateBuffer *a = *reinterpret_cast<CQuadStateBuffer **>(const_cast<void *>(a_pParam1));
	CQuadStateBuffer *b = *reinterpret_cast<CQuadStateBuffer **>(const_cast<void *>(a_pParam2));
	if ((a->getState()) > (b->getState())) {
//		::printf("l(%4d): %s: %d %d\n", __LINE__, __PRETTY_FUNCTION__,
//				a->getState(), b->getState());
		return -1;
	}
	if ((a->getState()) < (b->getState())) {
//		::printf("l(%4d): %s: %d %d\n", __LINE__, __PRETTY_FUNCTION__,
//				a->getState(), b->getState());
		return 1;
	}
//	::printf("l(%4d): %s: %d %d\n", __LINE__, __PRETTY_FUNCTION__,
//			a->getState(), b->getState());
	return 0;
}

/**
 *
 */
bool CContext::inInterrupt()
{
	return !!__get_IPSR();
}
