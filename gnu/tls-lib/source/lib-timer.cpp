
#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#ifndef __CYGWIN__
#include <sys/timerfd.h>
#endif // __CYGWIN__

#include <lib-timer.h>


/**
*/
CTimer::CTimer(CIOEventDispatcher& a_objDispatcher)
	:	m_bActive(EFalse)
{
#ifndef __CYGWIN__
	int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
#else // defined(__CYGWIN__)
	int fd = EInvalidHandleValue;
#endif // __CYGWIN__
	if (fd < 0) {
		CDebugUtil::printf(CDebugUtil::EError, "%s: l(%d): %s: errno=%d:%s\n",
				__FILE__, __LINE__, __PRETTY_FUNCTION__, errno, ::strerror(errno));
	}
	setHandle(fd);
	assert(isValidHandle());

	a_objDispatcher.add(this);
}

/**
*/
CTimer::~CTimer()
{
	if (getDispatcher()) {
		getDispatcher()->del(this);
	}
	closeHandle();
}

/**
*/
CHandle::RESULT CTimer::get(unsigned& a_unUntilUs)
{
	RESULT result = -1;
#ifndef __CYGWIN__
	struct itimerspec its;
	result = timerfd_gettime(getHandle(), &its);
	if (result < 0) {
		CDebugUtil::printf(CDebugUtil::EError, "%s: l(%d): %s: errno=%d:%s\n",
				__FILE__, __LINE__, __PRETTY_FUNCTION__, errno, ::strerror(errno));
		return result;
	}
	a_unUntilUs = (its.it_value.tv_sec * 1000000);
	a_unUntilUs += (its.it_value.tv_nsec / 1000);
#endif // __CYGWIN__
	return result;
}

/**
*/
CHandle::RESULT CTimer::set(unsigned a_unTimeoutUs, ECycle a_nCycle)
{
	RESULT result = -1;
#ifndef __CYGWIN__
	struct itimerspec its;
	m_bActive = (a_unTimeoutUs)? ETrue : EFalse;
	::memset(&its, 0, sizeof(its));
	its.it_value.tv_sec	= a_unTimeoutUs / 1000000;
	its.it_value.tv_nsec= a_unTimeoutUs % 1000000 * 1000;
	if (a_nCycle == ECyclic) {
		memcpy(&its.it_interval, &its.it_value, sizeof(its.it_interval));
	}
	result = timerfd_settime(getHandle(), 0, &its, NULL);
	if (result < 0) {
		CDebugUtil::printf(CDebugUtil::EError, "%s: l(%d): %s: errno=%d:%s\n",
				__FILE__, __LINE__, __PRETTY_FUNCTION__, errno, ::strerror(errno));
	}
#endif // __CYGWIN__
	return result;
}

/**
*/
CHandle::RESULT CTimer::clear()
{
	return set(0u);
}

/**
*/
CHandle::RESULT CTimer::onInput(CIOEventDispatcher& a_objDispatcher)
{
	unsigned char buf[8];
	RESULT result = CHandle::read(buf, sizeof(buf));
	if (result < 0) {
		CDebugUtil::printf(CDebugUtil::EError, "%s: l(%d): %s: errno=%d:%s\n",
				__FILE__, __LINE__, __PRETTY_FUNCTION__, errno, ::strerror(errno));
		return result;
	}
	onTimeout(m_bActive);
	return result;
}

/**
*/
void CTimer::onTimeout(EBool a_nActive)
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: act=%d\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, a_nActive);
}



