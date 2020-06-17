
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <lib-tls.h>

#define num_of_array(array)	(sizeof(array) / sizeof((array)[0]))

typedef struct tag_NetworkPeerInfo {
	const char				*cpszHostName;
	const char				*cpszIpAddr;
	const unsigned short	chwPort;
	const CSSocket::EDomain	cnDomain;
} NetworkPeerInfo;

char g_szAddress[128];

static NetworkPeerInfo s_nwPeer[] = {
	{"www.google.co.jp",	g_szAddress,	80u,	CSSocket::EDomainDNS},
#ifdef __CYGWIN__
	{NULL,	"192.168.1.10",	30101u,	CSSocket::EDomainIPv4},
	{NULL,	"192.168.1.10",	30201u,	CSSocket::EDomainIPv4},
	{NULL,	"192.168.1.10",	30301u,	CSSocket::EDomainIPv4},
	{NULL,	"192.168.1.10",	30401u,	CSSocket::EDomainIPv4},
	{NULL,	"fe80::52ce:6978:e480:9cc4",	30100u,	CSSocket::EDomainIPv6},
	{NULL,	"fe80::52ce:6978:e480:9cc4",	30200u,	CSSocket::EDomainIPv6},
	{NULL,	"fe80::52ce:6978:e480:9cc4",	30300u,	CSSocket::EDomainIPv6},
	{NULL,	"fe80::52ce:6978:e480:9cc4",	30400u,	CSSocket::EDomainIPv6},
#else
	{NULL,	"fe80::881a:9574:210c:beec",	30100u,	CSSocket::EDomainIPv6},
	{NULL,	"fe80::881a:9574:210c:beec",	30200u,	CSSocket::EDomainIPv6},
	{NULL,	"fe80::881a:9574:210c:beec",	30300u,	CSSocket::EDomainIPv6},
	{NULL,	"fe80::881a:9574:210c:beec",	30400u,	CSSocket::EDomainIPv6},
	{NULL,	"192.168.11.66",	30101u,	CSSocket::EDomainIPv4},
	{NULL,	"192.168.11.66",	30201u,	CSSocket::EDomainIPv4},
	{NULL,	"192.168.11.66",	30301u,	CSSocket::EDomainIPv4},
	{NULL,	"192.168.11.66",	30401u,	CSSocket::EDomainIPv4},
//	{NULL,	"fe80::8d7a:29e8:5971:9d29",	30500u,	CSSocket::EDomainIPv6},
//	{NULL,	"fe80::8d7a:29e8:5971:9d29",	30600u,	CSSocket::EDomainIPv6},
//	{NULL,	"fe80::8d7a:29e8:5971:9d29",	30700u,	CSSocket::EDomainIPv6},
//	{NULL,	"fe80::8d7a:29e8:5971:9d29",	30800u,	CSSocket::EDomainIPv6},
//	{NULL,	"192.168.180.1",	30501u,	CSSocket::EDomainIPv4},
//	{NULL,	"192.168.180.1",	30601u,	CSSocket::EDomainIPv4},
//	{NULL,	"192.168.180.1",	30701u,	CSSocket::EDomainIPv4},
//	{NULL,	"192.168.180.1",	30801u,	CSSocket::EDomainIPv4},
//	{NULL,	"fe80::7c04:c7da:258:d988",	30900u,	CSSocket::EDomainIPv6},
//	{NULL,	"fe80::7c04:c7da:258:d988",	31000u,	CSSocket::EDomainIPv6},
//	{NULL,	"fe80::7c04:c7da:258:d988",	31100u,	CSSocket::EDomainIPv6},
//	{NULL,	"fe80::7c04:c7da:258:d988",	31200u,	CSSocket::EDomainIPv6},
//	{NULL,	"192.168.190.1",	30901u,	CSSocket::EDomainIPv4},
//	{NULL,	"192.168.190.1",	31001u,	CSSocket::EDomainIPv4},
//	{NULL,	"192.168.190.1",	31101u,	CSSocket::EDomainIPv4},
//	{NULL,	"192.168.190.1",	31201u,	CSSocket::EDomainIPv4},
#endif
	{NULL,	"/tmp/test_uds1",		0u,		CSSocket::EDomainIPC},
	{NULL,	"/tmp/test_uds2",		0u,		CSSocket::EDomainIPC}
};

unsigned char g_byBuffer[1024];

#define USE_SIGACT
#include <signal.h>


static void print_ssl(SSL *s)
{
#if 0
	if (!s) {
		return;
	}
	::printf("%s: l(%d): ssl=%p: handshake=%p, shutdown=0x%08x, mode=0x%08lx, as_job=%p\n",
			__FUNCTION__, __LINE__, s, s->handshake_func, s->shutdown, s->mode,
			/*ASYNC_get_current_job()*/(void *)NULL);
#endif
}

class CClientSession : public CSSocketSession {
private:
	CIOEventDispatcher *m_pobjWatcher;

	const char		*m_pszHostName;
	const char		*m_pszAddress;
	unsigned short	m_hwPort;
	const char		*m_cpszPublicKey;
	const char		*m_cpszPrivateKey;
	const char		*m_cpszCertificate;

protected:
	unsigned char *propertyReceiveBuffer() {
		return g_byBuffer;
	}
	unsigned int propertyReceiveBufferSize() {
		return sizeof(g_byBuffer);
	}
	const char *propertyCertificateFile() {
		return m_cpszPublicKey;
	}
	const char *propertyPrivateKeyFile() {
		return m_cpszPrivateKey;
	}
	const char *propertyVerifyCertificateFile() {
		return m_cpszCertificate;
	}
	const char *propertyHostName() {
		return m_pszHostName;
	}
	const char *propertyAddress() {
		return m_pszAddress;
	}
	unsigned int propertyAddressSize() {
		return sizeof(g_szAddress);
	}
	const unsigned short propertyPort() {
		return m_hwPort;
	}

public:
	CClientSession(CIOEventDispatcher *a_pobjWatcher, ESecure a_nSecure,
				EDomain a_nDomain, const char *a_pszHostName,
				const char *a_pszAddress, unsigned short a_hwPort,
				const char *a_cpszPublicKey, const char *a_cpszPrivateKey,
				const char *a_cpszCertificate)
		:	CSSocketSession(a_nSecure, a_nDomain),
					m_pobjWatcher(a_pobjWatcher),
					m_pszHostName(a_pszHostName),
					m_pszAddress(a_pszAddress),
					m_hwPort(a_hwPort),
					m_cpszPublicKey(a_cpszPublicKey),
					m_cpszPrivateKey(a_cpszPrivateKey),
					m_cpszCertificate(a_cpszCertificate) {
		loadSslConfig();
	}
	virtual ~CClientSession() {}

	RESULT onAccept(CIOEventDispatcher& a_objController) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		int result = CSSocket::onAccept(a_objController);
		if (getDomain() == EDomainIPC) {
			::memset(g_byBuffer, 0x20 + getDomain(), 32);
			write(g_byBuffer, 32);
		}
		return result;
	}
	RESULT onEnableTransmit(CIOEventDispatcher& a_objDispatcher) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		return 0;
	}
	void onReceiveData(CIOEventDispatcher& a_objDispatcher,
			unsigned char *a_pbyAllocateData, int a_nValidLength) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		CDebugUtil::dump(a_pbyAllocateData, a_nValidLength);
		delete [] a_pbyAllocateData;
	}
	void onReceiveError(CIOEventDispatcher& a_objDispatcher) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
	}
	RESULT onSecureStartHandShake() {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		print_ssl(getSsl());
		return 0;
	}
	RESULT onSecureHandShakeDone() {
		print_ssl(getSsl());
		::memset(g_byBuffer, 0x10 + getDomain(), 64);
		RESULT result = write(g_byBuffer, 64);
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d, result=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain(), result);
#if 0
			do {
				result = write(g_byBuffer, 1024);
				::printf("%s: l(%d): %s: sent=%d, errno=%d:%s\n", __FILE__, __LINE__, __FUNCTION__, result, errno, ::strerror(errno));
			} while (result == 1024);
#endif
		return result;
	}
	RESULT onSecureAlert() {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		print_ssl(getSsl());
		return 0;
	}
};

/*
SIGHUP	 1	Term	制御端末(controlling terminal)のハングアップ検出または制御しているプロセスの死
SIGINT	 2	Term	キーボードからの割り込み (Interrupt)
SIGQUIT	 3	Core	キーボードによる中止 (Quit)
SIGILL	 4	Core	不正な命令
SIGABRT	 6	Core	abort(3) からの中断 (Abort) シグナル
SIGFPE	 8	Core	浮動小数点例外
SIGKILL	 9	Term	Kill シグナル
SIGSEGV	11	Core	不正なメモリー参照
SIGPIPE	13	Term	パイプ破壊:読み手の無いパイプへの書き出し
SIGALRM	14	Term	alarm(2) からのタイマーシグナル
SIGTERM	15	Term	終了 (termination) シグナル
SIGCHLD	20,17,18	Ign	子プロセスの一時停止 (stop) または終了
SIGCONT	19,18,25	Cont	一時停止 (stop) からの再開
SIGSTOP	17,19,23	Stop	プロセスの一時停止 (stop)
SIGTSTP	18,20,24	Stop	端末より入力された一時停止 (stop)
SIGTTIN	21,21,26	Stop	バックグランドプロセスの端末入力
SIGTTOU	22,22,27	Stop	バックグランドプロセスの端末出力
*/

static void sigaction(int signo, siginfo_t *info, void *ctx)
{
	::printf("%s: signo=%d, si_signo=%d, si_errno=%d, si_code=%d, si_fd=%d\n",
			__FUNCTION__,
			signo,
			info->si_signo,
			info->si_errno,
			info->si_code,
			/*info->si_fd*/0);
}

int main(int argc, char *argv[])
{
	int result;
	int i;

	struct sigaction act;
	::memset(&act, 0, sizeof(act));
	act.sa_sigaction= sigaction;
	act.sa_flags	= SA_SIGINFO;


#ifdef USE_SIGACT
	sigaction(SIGPIPE, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
//	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGILL, &act, NULL);
	sigaction(SIGABRT, &act, NULL);
	sigaction(SIGFPE, &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	sigaction(SIGALRM, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);
	sigaction(SIGCONT, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);
	sigaction(SIGTTIN, &act, NULL);
	sigaction(SIGTTOU, &act, NULL);
#endif // USE_SIGACT

	CSSocket::initialLibrary();

	CSSocket::ESecure type;

#if defined(TEST_TLS)
	type = CSSocket::ESecureTls;
#elif defined(TEST_TLS_1)
	type = CSSocket::ESecureTls_1;
#elif defined(TEST_TLS_1_1)
	type = CSSocket::ESecureTls_1_1;
#elif defined(TEST_TLS_1_2)
	type = CSSocket::ESecureTls_1_2;
#else
	type = CSSocket::ESecureNone;
#endif

	if (argc <= 1) {
		type = CSSocket::ESecureNone;
	}

	::printf("%s: l(%d): type=%d\n", __FUNCTION__, __LINE__, type);

	CIOEventDispatcher dispather;

	char *szLCert = (argc > 1)? argv[1] : NULL;
	char *szLKey  = (argc > 2)? argv[2] : NULL;
	char *szRCert = (argc > 3)? argv[3] : NULL;

	::printf("%s: l(%d): %s, %s, %s\n", __FUNCTION__, __LINE__, szLCert, szLKey, szRCert);

	CClientSession *s[num_of_array(s_nwPeer)];
	for (i = 0; i < (int)num_of_array(s_nwPeer); i++) {
		::printf("%s: l(%d): regist=%s/%d %s\n", __FUNCTION__, __LINE__,
				s_nwPeer[i].cpszIpAddr, s_nwPeer[i].chwPort, s_nwPeer[i].cpszHostName);
		s[i] = new CClientSession(&dispather, type, s_nwPeer[i].cnDomain,
				s_nwPeer[i].cpszHostName, s_nwPeer[i].cpszIpAddr, s_nwPeer[i].chwPort,
				szLCert, szLKey, szRCert);
		::printf("%s: l(%d): inst=%p, fd=%d\n", __FUNCTION__, __LINE__, s[i], s[i]->getHandle());

		result = s[i]->setConnectionTimeout(1000u);
		::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);

		dispather.add(s[i]);
		result = s[i]->connect();
		::printf("%s: l(%d): peer(%s/%d) %s: result=%d\n",
				__FUNCTION__, __LINE__, s_nwPeer[i].cpszIpAddr,
				s_nwPeer[i].chwPort, s_nwPeer[i].cpszHostName, result);
	}

	for (;;) {
//		::printf("%s: l(%d)\n", __FUNCTION__, __LINE__);
		result = dispather.dispatch(-1);
//		::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);
//		if (!result) {
//			continue;
//		}
//		::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);

		for (i = 0; i < (int)num_of_array(s_nwPeer); i++) {
			if (dispather.getObjectIndex(s[i]) != ~0u) {
				continue;
			}
			delete s[i];
			s[i] = new CClientSession(&dispather, type, s_nwPeer[i].cnDomain,
					s_nwPeer[i].cpszHostName, s_nwPeer[i].cpszIpAddr, s_nwPeer[i].chwPort,
					szLCert, szLKey, szRCert);
			::printf("%s: l(%d): inst=%p, fd=%d\n", __FUNCTION__, __LINE__, s[i], s[i]->getHandle());

			result = s[i]->setConnectionTimeout(1000u);
			::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);

			dispather.add(s[i]);
			result = s[i]->connect();
			::printf("%s: l(%d): peer(%s/%d): result=%d\n",
					__FUNCTION__, __LINE__, s_nwPeer[i].cpszIpAddr,
					s_nwPeer[i].chwPort, result);
		}
	}

	return 0;
}
