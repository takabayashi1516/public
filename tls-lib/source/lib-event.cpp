//
//
//

#include <stdio.h>

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>

#include <lib-event.h>

// #define DEBUG_TLS_STATUS

/**
*/
void CDebugUtil::dump(unsigned char *data, int size, const char *comment)
{
	if (comment) {
		printf(EDebug, "%s (size=%dbytes)\n", comment, size);
	} else {
		printf(EDebug, "(size=%dbytes)\n", size);
	}
	printf(EDebug, "    | +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f\n");
	printf(EDebug, "-----------------------------------------------------\n");
	int i;
	unsigned char xbuf[16];
	for (i = 0; size > 0; i += sizeof(xbuf), size -= sizeof(xbuf)) {
		::memset(xbuf, 0xcc, sizeof(xbuf));
		::memcpy(xbuf, &data[i], (size >= static_cast<int>(sizeof(xbuf)))?
				sizeof(xbuf) : size);
		printf(EDebug, "%04x| %02x %02x %02x %02x %02x %02x %02x %02x"
				" %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
				xbuf[0], xbuf[1], xbuf[2], xbuf[3], xbuf[4],
				xbuf[5], xbuf[6], xbuf[7], xbuf[8], xbuf[9],
				xbuf[10], xbuf[11], xbuf[12], xbuf[13], xbuf[14], xbuf[15]);
	}
	printf(EDebug, "----------------------------------------------------- (end)\n");
}

/**
*/
void CDebugUtil::printf(ELogLevel level, const char *format, ...)
{
	if (level > LOGLEVEL) {
		return;
	}

	char buf[512];
	switch (level) {
	case EError:
		::strcpy(buf, "[ERR] ");
		break;
	case EWarning:
		::strcpy(buf, "[WRN] ");
		break;
	case EDebug:
	default:
		::strcpy(buf, "[DBG] ");
		break;
	}
	va_list al;
	va_start(al, format);
	::vsprintf(&buf[::strlen(buf)], format, al);
	va_end(al);

	::fprintf(stdout, "%s", buf);
}

/**
*/
CHandle::CHandle()
	:	m_nHandle(EInvalidHandleValue)
{
}

/**
*/
CHandle::~CHandle()
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__);
}

/**
*/
CHandle::HANDLE CHandle::getHandle() const
{
	return m_nHandle;
}

/**
*/
void CHandle::setHandle(HANDLE a_nHandle)
{
	assert(EInvalidHandleValue != a_nHandle);
	m_nHandle = a_nHandle;
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: h=%d\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, getHandle());
}

/**
*/
CIOEventDispatcher *CHandle::getDispatcher() const
{
	return m_pobjDispatcher;
}

/**
*/
void CHandle::setDispatcher(CIOEventDispatcher& a_objDispatcher)
{
	m_pobjDispatcher = &a_objDispatcher;
}

/**
*/
int CHandle::isValidHandle() const
{
	return (getHandle() != EInvalidHandleValue);
}

/**
*/
CHandle::RESULT CHandle::read(unsigned char *a_lpbyBuffer, unsigned a_unLength)
{
	RESULT result = 0;
	if (isValidHandle()) {
		result = ::read(getHandle(), a_lpbyBuffer, a_unLength);
	}
	return result;
}


/**
*/
CHandle::RESULT CHandle::write(unsigned char *a_lpbyBuffer, unsigned a_unLength)
{
	RESULT result = 0;
	if (isValidHandle()) {
		result = ::write(getHandle(), a_lpbyBuffer, a_unLength);
	}
	return result;
}

/**
*/
CHandle::RESULT CHandle::shutdownHandle()
{
	return 0;
}

/**
*/
void CHandle::closeHandle()
{
	if (isValidHandle()) {
		CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: h=%d\n",
				__FILE__, __LINE__, __PRETTY_FUNCTION__, getHandle());
		::close(getHandle());
		m_nHandle = EInvalidHandleValue;
	}
}

/**
*/
CHandle::RESULT CHandle::onInput(CIOEventDispatcher& a_objDispatcher)
{
	return 0;
}

/**
*/
CHandle::EBool CHandle::isWatchDirectionOut() const
{
	return ETrue;
}

/**
*/
CHandle::EBool CHandle::isCreateAuto() const
{
	return EFalse;
}

/**
*/
CHandle::RESULT CHandle::onOutput(CIOEventDispatcher& a_objDispatcher)
{
	return 0;
}

/**
*/
CHandle::RESULT CHandle::onError(CIOEventDispatcher& a_objDispatcher, int a_nError)
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p: errno=%d:%s\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this, a_nError, ::strerror(a_nError));
	return 0;
}

/**
*/
CHandle::RESULT CHandle::onRemoteError(CIOEventDispatcher& a_objDispatcher, int a_nError)
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p: errno=%d:%s\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this, a_nError, ::strerror(a_nError));
	return 0;
}

/**
*/
CIOEventDispatcher::CIOEventDispatcher()
	:	m_pobjHandleArray(NULL), m_unRegistNumber(0u)
#if defined(__USE_SYSCALL_EPOLL)
	,	m_hHandle(0u), m_pobjEvent(NULL)
#elif defined(__USE_SYSCALL_POLL)
	,	m_pobjEvent(NULL)
#else	// select
#endif	// __USE_SYSCALL_
{
#if defined(__USE_SYSCALL_EPOLL)
	m_hHandle = ::epoll_create(1);
#elif defined(__USE_SYSCALL_POLL)
#else	// select
	FD_ZERO(&m_objReadFds);
	FD_ZERO(&m_objWriteFds);
	FD_ZERO(&m_objExceptFds);
#endif	// __USE_SYSCALL_
}

/**
*/
CIOEventDispatcher::~CIOEventDispatcher()
{
	while (removeIndex(0u) > 0) {}

#if defined(__USE_SYSCALL_EPOLL)
	::close(m_hHandle);
	if (m_pobjEvent) {
		delete [] m_pobjEvent;
	}
#elif defined(__USE_SYSCALL_POLL)
	if (m_pobjEvent) {
		delete [] m_pobjEvent;
	}
#else	// select
#endif	// __USE_SYSCALL_
}

/**
*/
int CIOEventDispatcher::add(CHandle *a_pobjHandle)
{
	int nEvents = (a_pobjHandle->isWatchDirectionOut())?
			(EEventRead | EEventWrite | EEventRomoteError) :
			(EEventRead | EEventRomoteError);

	if (!a_pobjHandle) {
		return (getRegistObjects() - 1);
	}
	if (!(a_pobjHandle->isValidHandle())) {
		return (getRegistObjects() - 1);
	}

	a_pobjHandle->setDispatcher(*this);

#if defined(__USE_SYSCALL_EPOLL)
	POLLEVENT ev;
	ev.data.ptr = a_pobjHandle;
	ev.events = (EEventMask & nEvents);
	::epoll_ctl(m_hHandle, EPOLL_CTL_ADD, a_pobjHandle->getHandle(), &ev);
#elif defined(__USE_SYSCALL_POLL)
#else	// select
	if (nEvents & EEventRead) {
		FD_SET(a_pobjHandle->getHandle(), &m_objReadFds);
	}
	if (nEvents & EEventWrite) {
		FD_SET(a_pobjHandle->getHandle(), &m_objWriteFds);
	}
//	if (nEvents & EEventExcept) {
	FD_SET(a_pobjHandle->getHandle(), &m_objExceptFds);
//	}
#endif	// __USE_SYSCALL_
	if (!m_pobjHandleArray) {
		m_unRegistNumber = 1u;
		m_pobjHandleArray = new P_CHandle_t[getRegistObjects()];
		m_pobjHandleArray[0] = a_pobjHandle;
#if defined(__USE_SYSCALL_EPOLL)
		m_pobjEvent = new POLLEVENT[getRegistObjects()];
#elif defined(__USE_SYSCALL_POLL)
		m_pobjEvent = new POLLEVENT[getRegistObjects()];
		m_pobjEvent[getRegistObjects() - 1].fd = a_pobjHandle->getHandle();
		m_pobjEvent[getRegistObjects() - 1].events = (EEventMask & nEvents);
		m_pobjEvent[getRegistObjects() - 1].revents = 0;
#else	// select
#endif	// __USE_SYSCALL_
		return (getRegistObjects() - 1);
	}
	P_CHandle_t *buff;
	buff = new P_CHandle_t[getRegistObjects()];
	::memcpy(buff, m_pobjHandleArray, sizeof(P_CHandle_t) * getRegistObjects());
	delete [] m_pobjHandleArray;
#if defined(__USE_SYSCALL_EPOLL)
#elif defined(__USE_SYSCALL_POLL)
	POLLEVENT *poll = new POLLEVENT[getRegistObjects()];
	::memcpy(poll, m_pobjEvent, sizeof(POLLEVENT) * getRegistObjects());
	delete [] m_pobjEvent;
#else	// select
#endif	// __USE_SYSCALL_
	m_unRegistNumber++;
	m_pobjHandleArray = new P_CHandle_t[getRegistObjects()];
	::memcpy(m_pobjHandleArray, buff, sizeof(P_CHandle_t) * getRegistObjects());
	delete [] buff;
	m_pobjHandleArray[getRegistObjects() - 1] = a_pobjHandle;
#if defined(__USE_SYSCALL_EPOLL)
	delete [] m_pobjEvent;
	m_pobjEvent = new POLLEVENT[getRegistObjects()];
#elif defined(__USE_SYSCALL_POLL)
	m_pobjEvent = new POLLEVENT[getRegistObjects()];
	::memcpy(m_pobjEvent, poll, sizeof(POLLEVENT) * getRegistObjects());
	delete [] poll;
	m_pobjEvent[getRegistObjects() - 1].fd = a_pobjHandle->getHandle();
	m_pobjEvent[getRegistObjects() - 1].events = (EEventMask & nEvents);
	m_pobjEvent[getRegistObjects() - 1].revents = (EEventMask & nEvents);
#else	// select
#endif	// __USE_SYSCALL_
	return (getRegistObjects() - 1);
}

/**
*/
int CIOEventDispatcher::update(CHandle *a_pobjHandle)
{
	if (!m_pobjHandleArray) {
		return -1;
	}

	if (!a_pobjHandle) {
		return (-1) * getRegistObjects();
	}

	int nEvents = (a_pobjHandle->isWatchDirectionOut())?
			(EEventRead | EEventWrite | EEventRomoteError) :
			(EEventRead | EEventRomoteError);

#if defined(__USE_SYSCALL_EPOLL)
	POLLEVENT ev;
	ev.data.ptr = a_pobjHandle;
	ev.events = (EEventMask & nEvents);
	::epoll_ctl(m_hHandle, EPOLL_CTL_MOD, a_pobjHandle->getHandle(), &ev);
#elif defined(__USE_SYSCALL_POLL)
#else	// select
	FD_CLR(a_pobjHandle->getHandle(), &m_objReadFds);
	FD_CLR(a_pobjHandle->getHandle(), &m_objWriteFds);
	FD_CLR(a_pobjHandle->getHandle(), &m_objExceptFds);
	if (nEvents & EEventRead) {
		FD_SET(a_pobjHandle->getHandle(), &m_objReadFds);
	}
	if (nEvents & EEventWrite) {
		FD_SET(a_pobjHandle->getHandle(), &m_objWriteFds);
	}
	FD_SET(a_pobjHandle->getHandle(), &m_objExceptFds);
#endif	// __USE_SYSCALL_

	int result = -1;
	for (unsigned i = 0; i < getRegistObjects(); i++) {
		if (m_pobjHandleArray[i] == a_pobjHandle) {
#if defined(__USE_SYSCALL_EPOLL)
#elif defined(__USE_SYSCALL_POLL)
			m_pobjEvent[i].fd = a_pobjHandle->getHandle();
			m_pobjEvent[i].events = (EEventMask & nEvents);
			m_pobjEvent[i].revents = 0;
			result = 0;
			return result;
#else	// select
#endif	// __USE_SYSCALL_
		}
	}

	return result;
}

/**
*/
int CIOEventDispatcher::del(CHandle *a_pobjHandle)
{
	unsigned index = getObjectIndex(a_pobjHandle);
	if (index == ~0u) {
		return getRegistObjects();
	}
	int result = removeIndex(index);
	return result;
}

/**
*/
int CIOEventDispatcher::removeIndex(unsigned int a_unIndex)
{
	if (a_unIndex >= getRegistObjects()) {
		return (-1) * getRegistObjects();
	}

	P_CHandle_t targetInst = m_pobjHandleArray[a_unIndex];

#if defined(__USE_SYSCALL_EPOLL)
	::epoll_ctl(m_hHandle, EPOLL_CTL_DEL, m_pobjHandleArray[a_unIndex]
			->getHandle(), NULL);
#elif defined(__USE_SYSCALL_POLL)
#else	// select
	FD_CLR(m_pobjHandleArray[a_unIndex]->getHandle(), &m_objReadFds);
	FD_CLR(m_pobjHandleArray[a_unIndex]->getHandle(), &m_objWriteFds);
	FD_CLR(m_pobjHandleArray[a_unIndex]->getHandle(), &m_objExceptFds);
#endif	// __USE_SYSCALL_

	if (getRegistObjects() == 1) {
		delete [] m_pobjHandleArray;
		m_pobjHandleArray = NULL;
		m_unRegistNumber = 0u;
#if defined(__USE_SYSCALL_EPOLL)
#elif defined(__USE_SYSCALL_POLL)
		delete [] m_pobjEvent;
		m_pobjEvent = NULL;
#else	// select
#endif	// __USE_SYSCALL_
		if (targetInst->isCreateAuto()) {
			delete targetInst;
		}
		return getRegistObjects();
	}

	P_CHandle_t *buff;
	buff = new P_CHandle_t[getRegistObjects()];
	::memcpy(buff, m_pobjHandleArray, sizeof(P_CHandle_t) * getRegistObjects());
	delete [] m_pobjHandleArray;
#if defined(__USE_SYSCALL_EPOLL)
#elif defined(__USE_SYSCALL_POLL)
	POLLEVENT *poll = new POLLEVENT[getRegistObjects()];
	::memcpy(poll, m_pobjEvent, sizeof(POLLEVENT) * getRegistObjects());
	delete [] m_pobjEvent;
#else	// select
#endif	// __USE_SYSCALL_
	m_unRegistNumber--;
	m_pobjHandleArray = new P_CHandle_t[getRegistObjects()];
	if (a_unIndex > 0) {
		::memcpy(m_pobjHandleArray, buff, sizeof(P_CHandle_t) * a_unIndex);
	}
	if (a_unIndex < getRegistObjects()) {
		::memcpy(&m_pobjHandleArray[a_unIndex], &buff[a_unIndex + 1],
				sizeof(P_CHandle_t) * (getRegistObjects() - a_unIndex));
	}
	delete [] buff;
#if defined(__USE_SYSCALL_EPOLL)
#elif defined(__USE_SYSCALL_POLL)
	m_pobjEvent = new POLLEVENT[getRegistObjects()];
	if (a_unIndex > 0) {
		::memcpy(m_pobjEvent, poll, sizeof(POLLEVENT) * a_unIndex);
	}
	if (a_unIndex < getRegistObjects()) {
		::memcpy(&m_pobjEvent[a_unIndex], &poll[a_unIndex + 1],
				sizeof(POLLEVENT) * (getRegistObjects() - a_unIndex));
	}
	delete [] poll;
#else	// select
#endif	// __USE_SYSCALL_

	if (targetInst->isCreateAuto()) {
		delete targetInst;
	}
	return getRegistObjects();
}

/**
*/
CHandle *CIOEventDispatcher::getObjectFromIndex(unsigned int a_unIndex) const
{
	if (a_unIndex >= getRegistObjects()) {
		return NULL;
	}
	return m_pobjHandleArray[a_unIndex];
}

/**
*/
CHandle *CIOEventDispatcher::getObject(CHandle::HANDLE a_nHandle) const
{
	for (unsigned i = 0; i < getRegistObjects(); i++) {
		CHandle *obj = getObjectFromIndex(i);
		if (!obj) {
			return NULL;
		}
		if (a_nHandle == obj->getHandle()) {
			return obj;
		}
	}
	return NULL;
}

/**
*/
unsigned int CIOEventDispatcher::getObjectIndex(
		CHandle *a_pobjHandle) const
{
	for (unsigned i = 0; i < getRegistObjects(); i++) {
		if (a_pobjHandle == getObjectFromIndex(i)) {
			return i;
		}
	}
	return ~0u;
}

/**
*/
unsigned int CIOEventDispatcher::getRegistObjects() const
{
	return m_unRegistNumber;
}

/**
*/
int CIOEventDispatcher::dispatch(unsigned long a_dwTimeout)
{
	CHandle::RESULT result = 0;
	int event_num = 0;

	if (getRegistObjects() <= 0u) {
		return result;
	}

#if defined(__USE_SYSCALL_EPOLL)
	event_num = ::epoll_wait(m_hHandle, m_pobjEvent,
			getRegistObjects(), a_dwTimeout);
	if (event_num < 0) {
		return event_num;
	}

	for (int i = 0; i < event_num; i++) {
		unsigned unEvents = m_pobjEvent[i].events;
		CHandle *s = reinterpret_cast<CHandle *>(m_pobjEvent[i].data.ptr);

		if ((unEvents & (EPOLLERR | EPOLLHUP))) {
			result = s->onError(*this, errno);
			continue;
		}

		if ((unEvents & EPOLLRDHUP)) {
			result = s->onRemoteError(*this, errno);
			continue;
		}

		if (unEvents & EPOLLIN) {
			result = s->onInput(*this);
			if (result < 0) {
				continue;
			}
		}

		if (unEvents & EPOLLOUT) {
			result = s->onOutput(*this);
		}
	}
#elif defined(__USE_SYSCALL_POLL)
	unsigned regNum = getRegistObjects();
	POLLEVENT *pFds = new POLLEVENT[regNum];
	::memcpy(pFds, m_pobjEvent, sizeof(POLLEVENT) * regNum);
	/* --- poll system call --- */
	event_num = ::poll(pFds, regNum, a_dwTimeout);
	if (event_num < 0) {
		delete [] pFds;
		return event_num;
	}

	for (unsigned i = 0; i < regNum; i++) {
		unsigned unEvents = pFds[i].revents;
		CHandle *s = getObject(pFds[i].fd);
		if (!s) {continue;}
		if (!unEvents) {continue;}

		if ((unEvents & (POLLERR | POLLHUP | POLLNVAL))) {
			result = s->onError(*this, errno);
			continue;
		}

#if defined(_GNU_SOURCE)
		if ((unEvents & POLLRDHUP)) {
			result = s->onRemoteError(*this, errno);
			continue;
		}
#endif // _GNU_SOURCE

		if (unEvents & POLLIN) {
			result = s->onInput(*this);
			if (result < 0) {
				continue;
			}
		}

		if (unEvents & POLLOUT) {
			result = s->onOutput(*this);
		}
	}

	delete [] pFds;
#else	/* !defined(__USE_SYSCALL_POLL) && !defined(__USE_SYSCALL_EPOLL) */
	/* --- select system call --- */
	CHandle::HANDLE hMax = 0;
	struct timeval to;
	if (a_dwTimeout != ~0ul) {
		to.tv_sec	= a_dwTimeout / 1000;
		to.tv_usec	= (a_dwTimeout % 1000) * 1000;
	} else {
		to.tv_sec	= ~0u;
		to.tv_usec	= ~0u;
	}

	unsigned regNum = getRegistObjects();

	fd_set readfds, writefds, exceptfds;
	readfds		= m_objReadFds;
	writefds	= m_objWriteFds;
	exceptfds	= m_objExceptFds;

	for (unsigned i = 0; i < regNum; i++) {
		CHandle *s = getObjectFromIndex(i);
		if (!s) {continue;}
		if (!s->isValidHandle()) {continue;}
		if (hMax < s->getHandle()) {
			hMax = s->getHandle();
		}
	}

	event_num = ::select(hMax + 1, &readfds, &writefds, &exceptfds,
			(a_dwTimeout == 0ul)? NULL : &to);
	if (event_num < 0) {
		return event_num;
	}

	for (unsigned i = 0; i < regNum; i++) {
		CHandle *s = getObjectFromIndex(i);
		if (!s) {continue;}
		if (!s->isValidHandle()) {continue;}

		if (FD_ISSET(s->getHandle(), &exceptfds)) {
			result = s->onError(*this, errno);
			continue;
		}

		if (FD_ISSET(s->getHandle(), &readfds)) {
			result = s->onInput(*this);
			if (result < 0) {
				continue;
			}
		}

		if (FD_ISSET(s->getHandle(), &writefds)) {
			result = s->onOutput(*this);
		}
	}
#endif	/* __USE_SYSCALL_POLL */

	return result;
}



