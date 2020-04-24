//
//
//
#ifndef __LIB_EVENT_H
#define __LIB_EVENT_H


#if defined(__USE_SYSCALL_EPOLL)
#include <sys/epoll.h>
#elif defined(__USE_SYSCALL_POLL)
#include <signal.h>
#include <poll.h>
#else
#include <sys/select.h>
#endif

// #define USE_CERT_VERIFY_CALLBACK

#ifndef LOGLEVEL
#define LOGLEVEL (-1)
#endif // LOGLEVEL

#ifdef __cplusplus


class CIOEventDispatcher;

/**
*/
class CDebugUtil {
public:
	enum ELogLevel {
		EError,
		EWarning,
		EDebug
	};
public:
	static void dump(unsigned char *data, int size, const char *comment = NULL);
	static void printf(ELogLevel level, const char *fomat, ...);
};

/**
*/
class CHandle {
public:
	typedef int HANDLE;
	typedef int RESULT;

	enum {
		EInvalidHandleValue = -1
	};

	enum EBool {
		EFalse = 0,
		ETrue
	};

private:
	HANDLE	m_nHandle;
	CIOEventDispatcher *m_pobjDispatcher;

protected:
	void setHandle(HANDLE a_nHandle);
	CIOEventDispatcher *getDispatcher() const;

public:
	CHandle();
	virtual ~CHandle();

	HANDLE getHandle() const;
	int isValidHandle() const;
	void setDispatcher(CIOEventDispatcher& a_objDispatcher);

	virtual RESULT read(unsigned char *a_lpbyBuffer, unsigned a_unLength);
	virtual RESULT write(unsigned char *a_lpbyBuffer, unsigned a_unLength);
	virtual RESULT shutdownHandle();
	virtual void closeHandle();

	virtual EBool isWatchDirectionOut() const;
	virtual EBool isCreateAuto() const;

	virtual RESULT onInput(CIOEventDispatcher& a_objDispatcher);
	virtual RESULT onOutput(CIOEventDispatcher& a_objDispatcher);
	virtual RESULT onError(CIOEventDispatcher& a_objDispatcher, int a_nError);
	virtual RESULT onRemoteError(CIOEventDispatcher& a_objDispatcher, int a_nError);
};

/**
*/
class CIOEventDispatcher {
public:
#if defined(__USE_SYSCALL_EPOLL)
	typedef struct epoll_event POLLEVENT;
#elif defined(__USE_SYSCALL_POLL)
	typedef struct pollfd POLLEVENT;
#endif	// __USE_SYSCALL_

public:
	enum EEvents {
#if defined(__USE_SYSCALL_EPOLL)
		EEventRead			= EPOLLIN,
		EEventWrite			= EPOLLOUT,
		EEventRomoteError	= EPOLLRDHUP,
#elif defined(__USE_SYSCALL_POLL)
		EEventRead			= POLLIN,
		EEventWrite			= POLLOUT,
#if defined(_GNU_SOURCE)
		EEventRomoteError	= POLLRDHUP,
#else  // !defined(_GNU_SOURCE)
		EEventRomoteError	= 0/*POLLRDHUP*/,
#endif // _GNU_SOURCE
#else	// select
		EEventRead			= 1 << 0,
		EEventWrite			= 1 << 1,
		EEventRomoteError	= 0,
#endif	// __USE_SYSCALL_
		EEventMask		= (EEventRead | EEventWrite | EEventRomoteError),
		EEventExcept	= ~EEventMask
	};

private:
	typedef CHandle *P_CHandle_t;
	P_CHandle_t			*m_pobjHandleArray;
	unsigned int		m_unRegistNumber;

#if defined(__USE_SYSCALL_EPOLL)
	CHandle::HANDLE		m_hHandle;
	POLLEVENT			*m_pobjEvent;
#elif defined(__USE_SYSCALL_POLL)
	POLLEVENT			*m_pobjEvent;
#else	// select
	fd_set				m_objReadFds;
	fd_set				m_objWriteFds;
	fd_set				m_objExceptFds;
#endif	// __USE_SYSCALL_

protected:
	int removeIndex(unsigned int a_unIndex);

public:
	CIOEventDispatcher();
	virtual ~CIOEventDispatcher();

	int add(CHandle *a_pobjHandle);
	int update(CHandle *a_pobjHandle);
	int del(CHandle *a_pobjHandle);

	int dispatch(unsigned long a_dwTimeout);

	CHandle *getObjectFromIndex(unsigned int a_unIndex) const;
	CHandle *getObject(CHandle::HANDLE a_nHandle) const;
	unsigned int getObjectIndex(CHandle *a_pobjHandle) const;
	unsigned int getRegistObjects() const;
};

#endif /* __cplusplus */


#endif /* __LIB_EVENT_H */
