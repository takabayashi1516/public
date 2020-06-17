


#include	<assert.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<pthread.h>
#include	<unistd.h>
#include	<sys/mman.h>
#ifdef	__USE_SYSCALL
#include	<sys/syscall.h>
#endif	// __USE_SYSCALL
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/time.h>

#include	"threadframe.h"


/**
 *
 */
static pid_t gettid(void)
{
#ifdef	__USE_SYSCALL
	return syscall(SYS_gettid);
#else	// !defined(__USE_SYSCALL)
	return 0;
#endif	// __USE_SYSCALL
}

/**
 *
 */
int	CUtil::read(const int a_cnFd, void *a_lpParam, size_t a_nCount)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): fd:=0x%08x, p:=0x%08lx, c:=%ld\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					a_cnFd, reinterpret_cast<unsigned long>(a_lpParam),
							a_nCount);
	ssize_t length = 0;
	unsigned char *lpbyBuffer = reinterpret_cast<unsigned char *>(a_lpParam);
	size_t count = a_nCount;
	while (count > 0) {
		length = ::read(a_cnFd, lpbyBuffer, count);
		if (length < 0)	{
			int result = -errno;
			::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): result:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__,
							__PRETTY_FUNCTION__, result);
			return(result);
		}
		count -= length;
		lpbyBuffer += length;
	}
	dump(a_lpParam, a_nCount);
	return(a_nCount);
}

/**
 *
 */
int	CUtil::write(const int a_cnFd, const void *a_clpParam, size_t a_nCount)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): fd:=0x%08x, p:=0x%08lx, c:=%ld\n",
			::getpid(), ::gettid(), __FILE__, __LINE__,
					__PRETTY_FUNCTION__, a_cnFd, reinterpret_cast<unsigned long>(
							const_cast<void *>(a_clpParam)), a_nCount);
	dump(a_clpParam, a_nCount);
	ssize_t length = 0;
	unsigned char *lpbyBuffer = reinterpret_cast<unsigned char *>(
			const_cast<void *>(a_clpParam));
	size_t count = a_nCount;
	while (count > 0) {
		length = ::write(a_cnFd, lpbyBuffer, count);
		if (length < 0)	{
			int result = -errno;
			::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): result:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
							result);
			return(result);
		}
		count -= length;
		lpbyBuffer += length;
	}
	return(a_nCount);
}

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
		::printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				buff[0], buff[1], buff[2], buff[3],
				buff[4], buff[5], buff[6], buff[7],
				buff[8], buff[9], buff[10], buff[11],
				buff[12], buff[13], buff[14], buff[15]);
	}
	::printf("----------------------------------------------------- %ld(0x%04lx)bytes\n",
			a_nCount, a_nCount);
#endif	// DEBUG_DUMP
}

/**
 *
 */
CThreadInterfaceBase::CLock::CLock() :
	m_objMutex(PTHREAD_MUTEX_INITIALIZER)
{
	::pthread_mutex_init(&m_objMutex, NULL);
}

/**
 *
 */
CThreadInterfaceBase::CLock::~CLock()
{
	::pthread_mutex_destroy(&m_objMutex);
}

/**
 *
 */
void	CThreadInterfaceBase::CLock::lock()
{
	::pthread_mutex_lock(&m_objMutex);
}

/**
 *
 */
void	CThreadInterfaceBase::CLock::unlock()
{
	::pthread_mutex_unlock(&m_objMutex);
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::CRequestMessageBase() :
	m_cnRequest(0),
	m_lpobjReplyMessage(NULL),
	m_lpobjObserver(NULL),
	m_cnLength(0)
{
}
/**
 *
 */
CThreadFrameBase::CRequestMessageBase::CRequestMessageBase(
		const int a_cnRequest) :
	m_cnRequest(a_cnRequest),
	m_lpobjReplyMessage(NULL),
	m_lpobjObserver(NULL),
	m_cnLength(0)
{
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::CRequestMessageBase(
		const int a_cnRequest, const int a_cnLength) :
	m_cnRequest(a_cnRequest),
	m_lpobjReplyMessage(NULL),
	m_lpobjObserver(NULL),
	m_cnLength(a_cnLength)
{
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::CRequestMessageBase(
		CRequestMessageBase& a_objMessageBase) :
	m_cnRequest(a_objMessageBase.m_cnRequest),
	m_lpobjReplyMessage(a_objMessageBase.m_lpobjReplyMessage),
	m_lpobjObserver(NULL),
	m_cnLength(a_objMessageBase.m_cnLength)
{
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase::~CRequestMessageBase()
{
}

/**
 *
 */
CThreadFrameBase::CRequestMessageTerminete::CRequestMessageTerminete() :
	CRequestMessageBase(CThreadFrameBase::ETerminate)
{
	m_lpobjReplyMessage = new CThreadFrame::CReplyMessage();
}

/**
 *
 */
CThreadFrameBase::CRequestMessageTerminete::~CRequestMessageTerminete()
{
	delete m_lpobjReplyMessage;
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
CThreadFrameBase::CReplyMessageBase::CReplyMessageBase()
{
}

/**
 *
 */
CThreadFrameBase::CReplyMessageBase::~CReplyMessageBase()
{
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
		struct timeval *a_lpobjRecvTimeout) :
	m_lpobjThreadInterface(&a_objThreadInterface),
	m_lpobjRecvTimeout(a_lpobjRecvTimeout)
{
	int	nResult = 0;

	if (m_lpobjRecvTimeout) {
		m_objRecvTimeout = *m_lpobjRecvTimeout;
	}

	if (::pipe(m_nArrayPipeFd) < 0)	{
		nResult = -errno;
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s() %d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
#ifdef	assert_perror
		assert_perror(nResult);
#endif	// assert_perror
	}

	nResult = ::pthread_create(&m_objPThread, NULL, mainRoutine, this);
	if (nResult)	{
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s() %d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
#ifdef	assert_perror
		assert_perror(nResult);
#endif	// assert_perror
	}
}

/**
 *
 */
CThreadFrameBase::~CThreadFrameBase()
{
	::close(m_nArrayPipeFd[0]);
	::close(m_nArrayPipeFd[1]);
}

/**
 *
 */
void	*CThreadFrameBase::mainRoutine(void *a_lpParam)
{
	void *pResult = reinterpret_cast<CThreadFrameBase *>(a_lpParam)->main();
	::pthread_exit(pResult);
	return(pResult);
}

/**
 *
 */
void	*CThreadFrameBase::main()
{
	int nResult = m_lpobjThreadInterface->onInitialize();
	if (nResult) {
		return(NULL);
	}

	int nRequest;
	do {
		CRequestMessageBase *lpMsg = receiveMessage(
				(m_lpobjRecvTimeout)? &m_objRecvTimeout : m_lpobjRecvTimeout);
		if (m_lpobjRecvTimeout && m_objRecvTimeout.tv_sec == 0 &&
				m_objRecvTimeout.tv_usec == 0) {
			m_objRecvTimeout = *m_lpobjRecvTimeout;
		}
		if (!lpMsg) {
			::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s()\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
			break;
		}
		nRequest = lpMsg->m_cnRequest;

		switch (nRequest) {
		case ETerminate:
			nResult = m_lpobjThreadInterface->onTerminate();
			if (nResult) {
				::printf("%6d::%6d: %s: l(%4d): %s(): result:=%d.\n",
						::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
								nResult);
				nRequest = 0;	// clear ETerminate
			}
			break;
		case ETick:
			nResult = m_lpobjThreadInterface->onTick();
			delete lpMsg;
			lpMsg = NULL;
			break;
		default:
			::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
							nRequest);
			if (lpMsg->m_lpobjObserver) {
				lpMsg->m_lpobjObserver->onStart(nRequest);
			}
			nResult = m_lpobjThreadInterface->onJudgeStatus(
					lpMsg->m_cnRequest, &lpMsg[1], lpMsg->m_cnLength);
			if (nResult) {
				::printf("%6d::%6d: %s: l(%4d): %s(): result:=%d.\n",
						::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
								nResult);
			} else {
				nResult = m_lpobjThreadInterface->onMessage(
						lpMsg->m_cnRequest, &lpMsg[1], lpMsg->m_cnLength);
			}
			if (lpMsg->m_lpobjObserver) {
				lpMsg->m_lpobjObserver->onFinished(nRequest, nResult);
			}
			break;
		}
		if (lpMsg && lpMsg->m_lpobjReplyMessage) {
			::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, result:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
							nRequest, nResult);
			lpMsg->m_lpobjReplyMessage->reply(nResult);
		}
		if (lpMsg) {
			delete [] reinterpret_cast<unsigned char *>(lpMsg);
		}
	} while (ETerminate != nRequest);

	return(NULL);
}

/**
 *
 */
CThreadFrameBase::CRequestMessageBase	*CThreadFrameBase::receiveMessage(
		struct timeval *a_lpobjTimeout)
{
	int	nResult = 0;
	int	nReadFd = m_nArrayPipeFd[0];

	if (a_lpobjTimeout &&
			(a_lpobjTimeout->tv_usec || a_lpobjTimeout->tv_sec)) {
		fd_set	rfds;
		FD_ZERO(&rfds);
		FD_SET(nReadFd, &rfds);
		nResult = ::select(nReadFd + 1, &rfds, NULL, NULL, a_lpobjTimeout);
		if (nResult < 0)	{
			nResult = -errno;
			::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): result:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
							nResult);
			return(NULL);
		}
		if (nResult == 0)	{
			::printf("%6d::%6d: %s: l(%4d): %s(): time out:=(%ld, %ld)\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
							a_lpobjTimeout->tv_sec, a_lpobjTimeout->tv_usec);
			return(new CMessageTick());
		}
	}

	CRequestMessageBase msg;
	ssize_t	nLength = CUtil::read(nReadFd, reinterpret_cast<void *>(&msg),
			sizeof(CRequestMessageBase));
	if (nLength < 0)	{
		nResult = -errno;
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): result:=%d\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
		return(NULL);
	}
	if (nLength != sizeof(CRequestMessageBase))	{
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): length:=%ld\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nLength);
		return(NULL);
	}
	unsigned char *pbuff =
			new unsigned char[sizeof(CRequestMessageBase) + msg.m_cnLength];
	::memcpy(pbuff, reinterpret_cast<void *>(&msg),
			sizeof(CRequestMessageBase));
	if (msg.m_cnLength > 0) {
		nLength = CUtil::read(nReadFd, &pbuff[sizeof(CRequestMessageBase)],
				msg.m_cnLength);
		if (nLength < 0)	{
			nResult = -errno;
			::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): result:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
							nResult);
			delete [] pbuff;
			return(NULL);
		}
		if (nLength != msg.m_cnLength)	{
			::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): length:=%ld\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
							nLength);
			delete [] pbuff;
			return(NULL);
		}
	}

	return(reinterpret_cast<CRequestMessageBase *>(pbuff));
}

/**
 *
 */
int	CThreadFrameBase::join(void*& a_lpParam)
{
	return(::pthread_join(m_objPThread, &a_lpParam));
}


/**
 *
 */
CThreadFrame::CReplyMessage::CReplyMessage()
{
	int	nResult = 0;
	if (::pipe(m_nArrayPipeFd) < 0)	{
		nResult = -errno;
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s() %d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
#ifdef	assert_perror
		assert_perror(nResult);
#endif	// assert_perror
	}
}

/**
 *
 */
CThreadFrame::CReplyMessage::~CReplyMessage()
{
	::close(m_nArrayPipeFd[0]);
	::close(m_nArrayPipeFd[1]);
}

/**
 *
 */
int	CThreadFrame::CReplyMessage::reply(const int a_cnResult)
{
	int nResult = 0;
	ssize_t nSize = CUtil::write(m_nArrayPipeFd[1], &a_cnResult, sizeof(int));
	if (nSize < 0)	{
		nResult = -errno;
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s() %d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
	}
	return(nResult);
}

/**
 *
 */
int	CThreadFrame::CReplyMessage::wait()
{
	int nResult = 0;
	int	nReadFd = m_nArrayPipeFd[0];
	int nLength = CUtil::read(nReadFd, &nResult, sizeof(nResult));
	if (nLength < 0)	{
		nResult = -errno;
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): result:=%d\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
		return(nResult);
	}
	if (nLength != sizeof(int))	{
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): length:=%d\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nLength);
		return(nResult);
	}
	return(nResult);
}

/**
 *
 */
void	CThreadFrame::CObserver::onStart(const int a_cnRequest)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);
}

/**
 *
 */
void	CThreadFrame::CObserver::onFinished(const int a_cnRequest,
		const int a_cnResult)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, result:=%d\n", ::getpid(),
			::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest, a_cnResult);
}

/**
 *
 */
void	CThreadFrame::CObserver::onCanceled(const int a_cnRequest)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);
}

/**
 *
 */
CThreadFrame::CThreadFrame(CThreadInterfaceBase& a_objThreadInterface,
		struct timeval *a_lpobjRecvTimeout) :
	CThreadFrameBase(a_objThreadInterface, a_lpobjRecvTimeout)
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
		struct timeval *a_lpobjRecvTimeout)
{
	return(new CThreadFrame(a_objThreadInterface, a_lpobjRecvTimeout));
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
		CThreadFrameBase::CRequestMessageBase& a_objMessage)
{
	int nResult = 0;
	int *pipeFd = m_lpobjThreadFrame->getArrayPipeFd();
	m_objLock.lock();
	ssize_t nSize = CUtil::write(pipeFd[1], &a_objMessage,
			sizeof(CThreadFrameBase::CRequestMessageBase)
					+ a_objMessage.m_cnLength);
	m_objLock.unlock();
	if (nSize < 0)	{
		nResult = -errno;
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s() %d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
	}

	if (0 == nResult && a_objMessage.m_lpobjReplyMessage) {
		nResult = a_objMessage.m_lpobjReplyMessage->wait();
	}
	return(nResult);
}

/**
 *
 */
int	CThreadInterfaceBase::sendMessage(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength,
		CThreadFrame::CReplyMessage *a_lpobjReplyMessage,
		CThreadFrameBase::CObserverBase *a_lpobjObserver)
{
	CThreadFrameBase::CRequestMessageBase msg(a_cnRequest, a_cnLength);
	msg.m_lpobjReplyMessage	= a_lpobjReplyMessage;
	msg.m_lpobjObserver		= a_lpobjObserver;
	unsigned int buff_size = sizeof(msg) + msg.m_cnLength;
	unsigned char *pbuff = new unsigned char[buff_size];
	::memset(pbuff, 0, buff_size);
	::memcpy(pbuff, &msg, sizeof(msg));
	if (a_lpParam && (a_cnLength > 0)) {
		::memcpy(&pbuff[sizeof(msg)], a_lpParam, a_cnLength);
	}
	CThreadFrameBase::CRequestMessageBase *lpmsg =
			reinterpret_cast<CThreadFrameBase::CRequestMessageBase *>(pbuff);
	int nResult = sendMessage(*lpmsg);
	if (nResult)	{
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s() %d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
	}
	delete [] pbuff;
	return(nResult);
}

/**
 *
 */
CThreadInterfaceBase::CThreadInterfaceBase(struct timeval *a_lpobjRecvTimeout)
{
	m_lpobjThreadFrame = CThreadFrame::newInstance(*this, a_lpobjRecvTimeout);
}

/**
 *
 */
CThreadInterfaceBase::CThreadInterfaceBase(CThreadInterfaceBase& a_objThread)
{
	m_lpobjThreadFrame = a_objThread.m_lpobjThreadFrame;
	m_lpobjThreadFrame->setThreadInterfaceBase(*this);
}

/**
 *
 */
CThreadInterfaceBase::~CThreadInterfaceBase()
{
	CThreadFrame::deleteInstance(m_lpobjThreadFrame);
}

/**
 *
 */
int	CThreadInterfaceBase::terminate()
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__, reinterpret_cast<unsigned long>(
					::__builtin_return_address(0)));
	CThreadFrameBase::CRequestMessageTerminete msg;
	int nResult = sendMessage(msg);
	if (nResult) {
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s() %d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, nResult);
		return(nResult);
	}
	void *pResult = NULL;
	nResult = m_lpobjThreadFrame->join(pResult);
	return(nResult);
}

/**
 *
 */
int	CThreadInterfaceBase::requestSync(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: r=%d, p=0x%08lx, l=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							a_cnRequest, reinterpret_cast<unsigned long>(
									const_cast<void *>(a_lpParam)), a_cnLength);
	CThreadFrame::CReplyMessage rmsg;
	int nResult = sendMessage(a_cnRequest, a_lpParam, a_cnLength,
			&rmsg, m_lpobjObserver);
	return(nResult);
}

/**
 *
 */
int	CThreadInterfaceBase::requestAsync(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: r=%d, p=0x%08lx, l=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							a_cnRequest, reinterpret_cast<unsigned long>(
									const_cast<void *>(a_lpParam)), a_cnLength);
	return(sendMessage(a_cnRequest, a_lpParam, a_cnLength,
			NULL, m_lpobjObserver));
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
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onTerminate()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onTick()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onMessage(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, param:=0x%08lx, len:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					a_cnRequest, reinterpret_cast<unsigned long>(a_lpParam),
							a_cnLength);
	return(0);
}

/**
 *
 */
int	CThreadInterfaceBase::onJudgeStatus(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, param:=0x%08lx, len:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					a_cnRequest, reinterpret_cast<unsigned long>(a_lpParam),
							a_cnLength);
	return(0);
}

/**
 *
 */
CThreadInterface4C::CObserver::CObserver(
		CThreadInterface4C *a_lpobjThreadInterface,
		struct tag_threadframe_observer *a_lpObserver) :
	m_lpobjThreadInterface(a_lpobjThreadInterface),
	m_lpObserver(a_lpObserver)
{
}

/**
 *
 */
void	CThreadInterface4C::CObserver::onStart(const int a_cnRequest)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);
	if (!m_lpObserver) {
		::printf("%6d::%6d: %s: l(%4d): %s(): Observer is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	if (!m_lpObserver->proc_start) {
		::printf("%6d::%6d: %s: l(%4d): %s(): proc_start is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	m_lpObserver->proc_start(m_lpobjThreadInterface,
			a_cnRequest);
}

/**
 *
 */
void	CThreadInterface4C::CObserver::onFinished(const int a_cnRequest,
		const int a_cnResult)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);
	if (!m_lpObserver) {
		::printf("%6d::%6d: %s: l(%4d): %s(): Observer is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	if (!m_lpObserver->proc_finished) {
		::printf("%6d::%6d: %s: l(%4d): %s(): proc_finished is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	m_lpObserver->proc_finished(m_lpobjThreadInterface,
			a_cnRequest, a_cnResult);
}

/**
 *
 */
void	CThreadInterface4C::CObserver::onCanceled(const int a_cnRequest)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);
	if (!m_lpObserver) {
		::printf("%6d::%6d: %s: l(%4d): %s(): Observer is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	if (!m_lpObserver->proc_canceled) {
		::printf("%6d::%6d: %s: l(%4d): %s(): proc_canceled is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	m_lpObserver->proc_canceled(
			m_lpobjThreadInterface, a_cnRequest);
}

/**
 *
 */
CThreadInterface4C::CThreadInterface4C(
		struct tag_threadframe_callbacks& a_objCallbacks,
		struct tag_threadframe_observer *a_lpObserver,
		struct timeval *a_lpobjRecvTimeout) :
	CThreadInterfaceBase(a_lpobjRecvTimeout),
	m_objCallbacks(a_objCallbacks),
	m_lpObserver(a_lpObserver)
{
	setObserver(new CThreadInterface4C::CObserver(this, m_lpObserver));
}

/**
 *
 */
CThreadInterface4C::~CThreadInterface4C()
{
	if (m_lpobjObserver) {
		delete m_lpobjObserver;
	}
}

/**
 *
 */
int	CThreadInterface4C::onInitialize()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	if (m_objCallbacks.notify_initialize)
		return(m_objCallbacks.notify_initialize(this));
	return(0);
}

/**
 *
 */
int	CThreadInterface4C::onTerminate()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	if (m_objCallbacks.notify_terminate)
		return(m_objCallbacks.notify_terminate(this));
	return(0);
}

/**
 *
 */
int	CThreadInterface4C::onTick()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	if (m_objCallbacks.notify_tick)
		return(m_objCallbacks.notify_tick(this));
	return(0);
}

/**
 *
 */
int	CThreadInterface4C::onMessage(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, param:=0x%08lx, len:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					a_cnRequest, reinterpret_cast<unsigned long>(a_lpParam),
							a_cnLength);
	if (m_objCallbacks.notify_message)
		return(m_objCallbacks.notify_message(this, a_cnRequest,
				a_lpParam, a_cnLength));
	return(0);
}

/**
 *
 */
int	CThreadInterface4C::onJudgeStatus(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, param:=0x%08lx, len:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					a_cnRequest, reinterpret_cast<unsigned long>(a_lpParam),
							a_cnLength);
	if (m_objCallbacks.notify_judgestatus)
		return(m_objCallbacks.notify_judgestatus(this, a_cnRequest,
				a_lpParam, a_cnLength));
	return(0);
}

/**
 *
 */
CHandlerBase::CActionMap::CActionMap(const int a_cnRequest) :
		m_cnRequest(a_cnRequest), m_nActionItems(0), m_lpobjActionArray(NULL)
{
}

/**
 *
 */
CHandlerBase::CActionMap::~CActionMap()
{
	if (m_lpobjActionArray) {
		delete [] m_lpobjActionArray;
	}
}

/**
 *
 */
int	CHandlerBase::CActionMap::add(CActionBase *a_lpobjAction)
{
	m_objLock.lock();
	if (!m_lpobjActionArray) {
		m_lpobjActionArray = new tag_LPACTIONBASE[m_nActionItems + 1];
		m_lpobjActionArray[m_nActionItems] = a_lpobjAction;
		m_nActionItems++;
	} else {
		tag_LPACTIONBASE *buff = new tag_LPACTIONBASE[m_nActionItems];
		::memcpy(buff, m_lpobjActionArray,
				sizeof(CActionBase *) * m_nActionItems);
		delete [] m_lpobjActionArray;
		m_lpobjActionArray = new tag_LPACTIONBASE[m_nActionItems + 1];
		::memcpy(m_lpobjActionArray, buff,
				sizeof(CActionBase *) * m_nActionItems);
		delete [] buff;
		m_lpobjActionArray[m_nActionItems] = a_lpobjAction;
		m_nActionItems++;
	}
	m_objLock.unlock();
	return(0);
}

/**
 *
 */
int	CHandlerBase::CActionMap::remove(CActionBase *a_lpobjAction)
{
	int nResult = -ENOENT;
	m_objLock.lock();
	for (int i = 0; i < m_nActionItems; i++) {
		if (m_lpobjActionArray[i] == a_lpobjAction) {
			nResult = 0;
			if (--m_nActionItems) {
				tag_LPACTIONBASE *buff = new tag_LPACTIONBASE[m_nActionItems];
				::memcpy(buff, m_lpobjActionArray, sizeof(CActionBase *) * i);
				if (i < m_nActionItems) {
					::memcpy(&buff[i], &m_lpobjActionArray[i + 1],
							sizeof(CActionBase *) * (m_nActionItems - i));
				}
				delete [] m_lpobjActionArray;
				m_lpobjActionArray = new tag_LPACTIONBASE[m_nActionItems];
				::memcpy(m_lpobjActionArray, buff,
						sizeof(CActionBase *) * m_nActionItems);
				delete [] buff;
			} else {
				delete [] m_lpobjActionArray;
				m_lpobjActionArray = NULL;
			}
			break;
		}
	}
	m_objLock.unlock();
	return(nResult);
}

/**
 *
 */
int	CHandlerBase::CActionMap::broadCast(const void *a_lpParam,
		const int a_cnLength)
{
	int nResult = 0;
	m_objLock.lock();
	for (int i = 0; i < m_nActionItems; i++) {
		int result = m_lpobjActionArray[i]->doAction(
				a_lpParam, a_cnLength);
		if (!result) continue;
		nResult = result;
	}
	m_objLock.unlock();
	return(nResult);
}

/**
 *
 */
CHandlerBase::CHandlerBase(struct timeval *a_lpobjRecvTimeout,
		CThreadFrame::CObserver *a_lpobjObserver) :
	CThreadInterfaceBase(a_lpobjRecvTimeout),
	m_lpobjActionMap(NULL),
	m_nActionMapItems(0)
{
	setObserver(a_lpobjObserver);
}

/**
 *
 */
CHandlerBase::~CHandlerBase()
{
	if (m_lpobjActionMap) {
		for (int i = 0; i < m_nActionMapItems; i++) {
			delete m_lpobjActionMap[i];
		}
		delete [] m_lpobjActionMap;
	}
}

/**
 *
 */
int	CHandlerBase::onMessage(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, param:=0x%08lx, len:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					a_cnRequest, reinterpret_cast<unsigned long>(a_lpParam),
							a_cnLength);
	int nResult = 0;
	CActionMap *map = searchActionMap(a_cnRequest);
	if (map) {
		nResult = map->broadCast(a_lpParam, a_cnLength);
	}
	return(nResult);
}

/**
 *
 */
int	CHandlerBase::registerAction(const int a_cnRequest,
		CActionBase *a_lpobjAction)
{
//	m_objLock.lock();
	if (!m_lpobjActionMap) {
		m_lpobjActionMap = new tag_LPACTIONMAP[m_nActionMapItems + 1];
		m_lpobjActionMap[m_nActionMapItems] = new CActionMap(a_cnRequest);
		m_lpobjActionMap[m_nActionMapItems]->add(a_lpobjAction);
		m_nActionMapItems++;
	} else {
		CActionMap *map = searchActionMap(a_cnRequest);
		if (map) {
			map->add(a_lpobjAction);
		} else {
			tag_LPACTIONMAP *buff = new tag_LPACTIONMAP[m_nActionMapItems];
			::memcpy(buff, m_lpobjActionMap,
					sizeof(CActionMap *) * m_nActionMapItems);
			delete [] m_lpobjActionMap;
			m_lpobjActionMap = new tag_LPACTIONMAP[m_nActionMapItems + 1];
			::memcpy(m_lpobjActionMap, buff,
					sizeof(CActionMap *) * m_nActionMapItems);
			delete [] buff;
			m_lpobjActionMap[m_nActionMapItems] = new CActionMap(a_cnRequest);
			m_lpobjActionMap[m_nActionMapItems]->add(a_lpobjAction);
			m_nActionMapItems++;
		}
	}
//	m_objLock.unlock();
	return(0);
}

/**
 *
 */
int	CHandlerBase::deregisterAction(const int a_cnRequest,
		CActionBase *a_lpobjAction)
{
	int nResult = -ENOENT;
//	m_objLock.lock();
	CActionMap *map = searchActionMap(a_cnRequest);
	if (map) {
		map->remove(a_lpobjAction);
		nResult = 0;
	}
//	m_objLock.unlock();
	return(nResult);
}

/**
 *
 */
CHandlerBase::CActionMap	*CHandlerBase::searchActionMap(
		const int a_cnRequest)
{
	if (m_lpobjActionMap) {
		for (int i = 0; i < m_nActionMapItems; i++) {
			if (m_lpobjActionMap[i]->getRequest() == a_cnRequest) {
				return(m_lpobjActionMap[i]);
			}
		}
	}
	return(NULL);
}

/**
 *
 */
CHandler4C::CObserver::CObserver(CHandler4C *a_lpobjOwner,
		struct tag_threadframe_observer *a_lpObserver) :
	m_lpobjOwner(a_lpobjOwner),
	m_lpObserver(a_lpObserver)
{
}

/**
 *
 */
void	CHandler4C::CObserver::onStart(const int a_cnRequest)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);

	if (!m_lpObserver) {
		::fprintf(stderr,
				"%6d::%6d: %s: l(%4d): %s(): Observer is NULL.\n",
						::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}

	if (!m_lpObserver->proc_start) {
		::printf("%6d::%6d: %s: l(%4d): %s(): proc_start is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	m_lpObserver->proc_start(m_lpobjOwner,
			a_cnRequest);
}

/**
 *
 */
void	CHandler4C::CObserver::onFinished(const int a_cnRequest,
		const int a_cnResult)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);

	if (!m_lpObserver) {
		::fprintf(stderr,
				"%6d::%6d: %s: l(%4d): %s(): Observer is NULL.\n",
						::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}

	if (!m_lpObserver->proc_finished) {
		::printf("%6d::%6d: %s: l(%4d): %s(): proc_finished is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	m_lpObserver->proc_finished(m_lpobjOwner,
			a_cnRequest, a_cnResult);
}

/**
 *
 */
void	CHandler4C::CObserver::onCanceled(const int a_cnRequest)
{
	::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__, a_cnRequest);

	if (!m_lpObserver) {
		::fprintf(stderr,
				"%6d::%6d: %s: l(%4d): %s(): Observer is NULL.\n",
						::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}

	if (!m_lpObserver->proc_canceled) {
		::printf("%6d::%6d: %s: l(%4d): %s(): proc_canceled is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return;
	}
	m_lpObserver->proc_canceled(m_lpobjOwner,
			a_cnRequest);
}

/**
 *
 */
CHandler4C::CAction::CAction(CHandler4C *a_lpobjOwner,
		struct tag_threadframe_action *a_lpobjAction) :
	m_lpobjOwner(a_lpobjOwner),
	m_lpobjAction(a_lpobjAction)
{
}

/**
 *
 */
int CHandler4C::CAction::doAction(const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);

	if (!m_lpobjAction) {
		::printf("%6d::%6d: %s: l(%4d): %s(): ActionObject is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return(0);
	}

	if (!m_lpobjAction->do_action) {
		::printf("%6d::%6d: %s: l(%4d): %s(): do_action is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return(0);
	}

	return(m_lpobjAction->do_action(m_lpobjOwner, a_lpParam, a_cnLength,
			m_lpobjAction));
}

/**
 *
 */
CHandler4C::CHandler4C(struct tag_threadframe_callbacks& a_objCallbacks,
		struct tag_threadframe_observer *a_lpObserver,
				struct timeval *a_lpobjRecvTimeout) :
	CHandlerBase(a_lpobjRecvTimeout, NULL),
	m_objCallbacks(a_objCallbacks)
{
	setObserver(new CObserver(this, a_lpObserver));
}

/**
 *
 */
int	CHandler4C::onInitialize()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	if (m_objCallbacks.notify_initialize)
		return(m_objCallbacks.notify_initialize(this));
	return(0);
}

/**
 *
 */
int	CHandler4C::onTerminate()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	if (m_objCallbacks.notify_terminate)
		return(m_objCallbacks.notify_terminate(this));
	return(0);
}

/**
 *
 */
int	CHandler4C::onTick()
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	if (m_objCallbacks.notify_tick)
		return(m_objCallbacks.notify_tick(this));
	return(0);
}

/**
 *
 */
int	CHandler4C::onJudgeStatus(const int a_cnRequest,
		const void *a_lpParam, const int a_cnLength)
{
	::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(), ::gettid(),
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
	if (m_objCallbacks.notify_judgestatus)
		return(m_objCallbacks.notify_judgestatus(this, a_cnRequest,
				a_lpParam, a_cnLength));
	return(0);
}

/**
 *
 */
int	CHandler4C::registerAction4C(
		struct tag_threadframe_action *a_lpobjAction)
{
	a_lpobjAction->handle = new CHandler4C::CAction(this, a_lpobjAction);
	return(registerAction(a_lpobjAction->request,
			reinterpret_cast<CAction *>(a_lpobjAction->handle)));
}

/**
 *
 */
int	CHandler4C::deregisterAction4C(
		struct tag_threadframe_action *a_lpobjAction)
{
	return(deregisterAction(a_lpobjAction->request,
			reinterpret_cast<CAction *>(a_lpobjAction->handle)));
}


/**
 *
 */
void	*cre_threadframe(struct tag_threadframe_callbacks *callbacks,
		struct tag_threadframe_observer *observer, struct timeval *timeout)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: c=0x%08lx, o=0x%08lx\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							reinterpret_cast<unsigned long>(callbacks),
									reinterpret_cast<unsigned long>(observer));
	return(new CThreadInterface4C(*callbacks, observer, timeout));
}

/**
 *
 */
void	*del_threadframe(void *handle)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: h=0x%08lx\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							reinterpret_cast<unsigned long>(handle));
	delete reinterpret_cast<CThreadInterface4C *>(handle);
}

/**
 *
 */
void	*cre_threadhandler(struct tag_threadframe_callbacks *callbacks,
		struct tag_threadframe_observer *observer, struct timeval *timeout)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: c=0x%08lx, o=0x%08lx\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							reinterpret_cast<unsigned long>(callbacks),
									reinterpret_cast<unsigned long>(observer));
	return(new CHandler4C(*callbacks, observer, timeout));
}

/**
 *
 */
void	*del_threadhandler(void *handle)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: h=0x%08lx\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							reinterpret_cast<unsigned long>(handle));
	delete reinterpret_cast<CHandler4C *>(handle);
}

/**
 *
 */
int	term_threadframe(void *handle)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: h=0x%08lx\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							reinterpret_cast<unsigned long>(handle));
	int result = -EINVAL;
	if (!handle) {
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): handle is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return(result);
	}
	result = reinterpret_cast<CThreadInterfaceBase *>(handle)->terminate();
	return(result);
}

/**
 *
 */
int	req_sync_threadframe(void *handle, const int req, const void *param,
		const int length)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: h=0x%08lx, r=%d, p=0x%08lx, l=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							reinterpret_cast<unsigned long>(handle), req,
									reinterpret_cast<unsigned long>(param),
									length);
	int result = -EINVAL;
	if (!handle) {
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): handle is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return(result);
	}
	result = reinterpret_cast<CThreadInterfaceBase *>(handle)
			->requestSync(req, param, length);
	return(result);
}

/**
 *
 */
int	req_async_threadframe(void *handle, const int req, const void *param,
		const int length)
{
	::printf("%6d::%6d: %s: l(%4d): %s() by 0x%08lx: h=0x%08lx, r=%d, p=0x%08lx, l=%d\n",
			::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__,
					reinterpret_cast<unsigned long>(::__builtin_return_address(0)),
							reinterpret_cast<unsigned long>(handle), req,
									reinterpret_cast<unsigned long>(param),
											length);
	int result = -EINVAL;
	if (!handle) {
		::fprintf(stderr,
				"%6d::%6d: %s: l(%4d): %s(): handle is NULL.\n",
						::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return(result);
	}
	result = reinterpret_cast<CThreadInterfaceBase *>(handle)
			->requestAsync(req, param, length);
	return(result);
}

/**
 *
 */
int	register_action(void *handle, struct tag_threadframe_action *action)
{
	int result = -EINVAL;
	if (!handle) {
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): handle is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return(result);
	}
	result = reinterpret_cast<CHandler4C *>(handle)
			->registerAction4C(action);
	return(result);
}

/**
 *
 */
int	deregister_action(void *handle, struct tag_threadframe_action *action)
{
	int result = -EINVAL;
	if (!handle) {
		::fprintf(stderr, "%6d::%6d: %s: l(%4d): %s(): handle is NULL.\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
		return(result);
	}
	result = reinterpret_cast<CHandler4C *>(handle)
			->deregisterAction4C(action);
	return(result);
}



