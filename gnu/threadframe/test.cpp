


#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/mman.h>
#ifdef	__USE_SYSCALL
#include	<sys/syscall.h>
#endif	// __USE_SYSCALL
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<sys/socket.h>
#include	<sys/un.h>

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
class CThreadInterfaceTest : public CThreadInterfaceBase {
	/**
	 *
	 */
	class CObserver : public CThreadFrame::CObserver {
	public:
		/// 
		CObserver() {}
		/// 
		void	onStart(const int a_cnRequest) {
			::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __func__, a_cnRequest);
		}
		/// 
		void	onFinished(const int a_cnRequest, const int a_cnResult) {
			::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __func__, a_cnRequest);
		}
		/// 
		void	onCanceled(const int a_cnRequest) {
			::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d\n",
					::getpid(), ::gettid(), __FILE__, __LINE__, __func__, a_cnRequest);
		}
	};

public:
	/// 
	CThreadInterfaceTest(struct timeval *a_lpobjRecvTimeout) :
			CThreadInterfaceBase(a_lpobjRecvTimeout),
			m_nTerminateResult(0) {
		setObserver(new CObserver());
	}
	/// 
	~CThreadInterfaceTest() {
		delete m_lpobjObserver;
	}
	/// 
	int	onInitialize() {
		::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __func__);
		return(0);
	}
	/// 
	int	onTerminate() {
		::printf("%6d::%6d: %s: l(%4d): %s(): result:=%d\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __func__,
						m_nTerminateResult);
		return(m_nTerminateResult);
	}
	/// 
	int	onTick() {
		::printf("%6d::%6d: %s: l(%4d): %s()\n", ::getpid(),
				::gettid(), __FILE__, __LINE__, __func__);
		return(0);
	}
	/// 
	int	onMessage(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength) {
		::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, param:=0x%08lx, len:=%d\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __func__,
						a_cnRequest, (unsigned long)a_lpParam, a_cnLength);
		return(0);
	}
	/// 
	int	onJudgeStatus(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength) {
		::printf("%6d::%6d: %s: l(%4d): %s(): req:=%d, param:=0x%08lx, len:=%d\n",
				::getpid(), ::gettid(), __FILE__, __LINE__, __func__,
						a_cnRequest, (unsigned long)a_lpParam, a_cnLength);
		return(0);
	}

public:
	int	m_nTerminateResult;
};


int	main(int argc, char *argv[])
{
	int result = 0;
	struct timeval *ptime = NULL;
	struct timeval tim;

	if (argc > 1) {
		tim.tv_sec	= ::atoi(argv[1]);
		tim.tv_usec	= 0;
		ptime = &tim;
	}
	if (argc > 2) {
		tim.tv_usec	= ::atoi(argv[2]);
	}
	CThreadInterfaceTest *interface = new CThreadInterfaceTest(ptime);

	::printf("%s: l(%4d): %s(): interface:=0x%08lx.\n",
			__FILE__, __LINE__, __func__, reinterpret_cast<unsigned long>(interface));

	while (1)	{
		::printf("press any decimal digit...\n");
		int req = 0;
		::scanf("%d", &req);
		if (req == -1)	{
			::printf("press any decimal digit(terminate result)...\n");
			::scanf("%d", &(interface->m_nTerminateResult));
			result = interface->terminate();
			::printf("%s: l(%4d): %s(): result:=%d.\n",
					__FILE__, __LINE__, __func__, result);
			if (!result)	{
				break;
			}
			continue;
		}

		::printf("press any string...\n");
		char strbuff[1024];
		::scanf("%s", strbuff);

		if (req > 1000) {
			result = interface->requestAsync(req, strbuff, ::strlen(strbuff));
			::printf("%s: l(%4d): %s(): req:=%d, result:=%d (%s).\n",
					__FILE__, __LINE__, __func__, req, result, strbuff);
		} else {
			result = interface->requestSync(req, strbuff, ::strlen(strbuff));
			::printf("%s: l(%4d): %s(): req:=%d, result:=%d (%s).\n",
					__FILE__, __LINE__, __func__, req, result, strbuff);
		}
	}

	delete interface;

	::printf("%s: l(%4d): %s()\n", __FILE__, __LINE__, __func__);

	return(result);
}



