
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <lib-tls.h>
#include <lib-timer.h>

unsigned char g_byBuffer[128 * 1024];

static void print_ssl(SSL *s)
{
	if (!s) {
		return;
	}
//	printf("%s: l(%d): ssl=%p: handshake=%p, shutdown=0x%08x, mode=0x%08lx, as_job=%p\n",
//			__FUNCTION__, __LINE__, s, s->handshake_func, s->shutdown, s->mode,
//			/*ASYNC_get_current_job()*/(void *)NULL);
}

class CServerSession;
class CServerListener;

class CServerSession : public CSSocketSession {
private:
	const char		*m_cpszPublicKey;
	const char		*m_cpszPrivateKey;
	const char		*m_cpszCertificate;

public:
	CServerSession(ESecure a_nSecure, HANDLE a_hDesc, EDomain a_nDomain,
				CSSocket *a_pobjListener, const char *a_cpszPublicKey = NULL,
				const char *a_cpszPrivateKey = NULL,
				const char *a_cpszCertificate = NULL)
		:	CSSocketSession(a_nSecure, a_hDesc, a_nDomain, a_pobjListener),
					m_cpszPublicKey(a_cpszPublicKey),
					m_cpszPrivateKey(a_cpszPrivateKey),
					m_cpszCertificate(a_cpszCertificate) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		loadSslConfig();
	}
	virtual ~CServerSession() {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
	}

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
	RESULT onEnableTransmit(CIOEventDispatcher& a_objDispatcher) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		return 0;
	}
	void onReceiveData(CIOEventDispatcher& a_objDispatcher,
			unsigned char *a_pbyAllocateData, int a_nValidLength) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n",
				__FUNCTION__, __LINE__, this, getHandle(), getDomain());
		CDebugUtil::dump(a_pbyAllocateData, a_nValidLength);
		// echo-back
		write(a_pbyAllocateData, a_nValidLength);
		delete [] a_pbyAllocateData;
#if 0
		RESULT result;
		do {
			result = write(g_byBuffer, 1024);
			::printf("%s: l(%d): %s: sent=%d, errno=%d:%s\n", __FILE__, __LINE__, __FUNCTION__, result, errno, ::strerror(errno));
		} while (result == 1024);
#endif
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
		::memset(g_byBuffer, 0x30 + getDomain(), 128);
		RESULT result = write(g_byBuffer, 128);
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d, result=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain(), result);
		return result;
	}
	RESULT onSecureAlert() {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		print_ssl(getSsl());
		return 0;
	}
};

class CServerListener : public CSSocketListener {
private:
	const ESecure	m_cnSecure;
	const char		*m_cszLCert;
	const char		*m_cszLKey;
	const char		*m_cszRCert;
	unsigned short	m_hwPort;

public:
	CServerListener(ESecure a_nSecure, EDomain a_nDomain,
			unsigned short a_hwPort, char *a_szLCert, char *a_szLKey,
			char *a_szRCert)
		:	 CSSocketListener(a_nDomain),
					m_cnSecure(a_nSecure), m_cszLCert(a_szLCert),
					m_cszLKey(a_szLKey), m_cszRCert(a_szRCert),
					m_hwPort(a_hwPort) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d, sec=%d\n",
				__FUNCTION__, __LINE__, this, getHandle(), getDomain(), getSecure());
		bind();
	}
	virtual ~CServerListener() {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
	}

	RESULT onConnect(CIOEventDispatcher& a_objController, CSSocket*& a_pobjSocket) {
		RESULT result = CSSocketListener::onConnect(a_objController, a_pobjSocket);
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d: regstNo=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain(), result);
		return result;
	}

protected:
	CSSocket *newSocket(HANDLE a_hDesc, EDomain a_nDomain) {
		return new CServerSession(m_cnSecure, a_hDesc, a_nDomain, this,
				m_cszLCert, m_cszLKey, m_cszRCert);
	}
	const unsigned short propertyPort() {
		return m_hwPort;
	}

};

class CIpcListener : public CSSocketListener {
private:
	const ESecure	m_cnSecure;
	const char		*m_cszLCert;
	const char		*m_cszLKey;
	const char		*m_cszRCert;

public:
	CIpcListener(char *a_szName, ESecure a_nSecure,
			char *a_szLCert, char *a_szLKey, char *a_szRCert)
		:	CSSocketListener(EDomainIPC), m_cnSecure(a_nSecure),
					m_cszLCert(a_szLCert), m_cszLKey(a_szLKey),
					m_cszRCert(a_szRCert) {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
		bind(a_szName);
	}
	virtual ~CIpcListener() {
		::printf("%s: l(%d): inst=%p, fd=%d, domain=%d\n", __FUNCTION__, __LINE__, this, getHandle(), getDomain());
	}
	RESULT onConnect(CIOEventDispatcher& a_objController, CSSocket*& a_pobjSocket) {
		RESULT result = CSSocketListener::onConnect(a_objController, a_pobjSocket);
		::memset(g_byBuffer, 0x20 + getDomain(), 32);
		a_objController.getObjectFromIndex(a_objController.getRegistObjects() - 1)
				->write(g_byBuffer, 32);
		return result;
	}

protected:
	CSSocket *newSocket(HANDLE a_hDesc, EDomain a_nDomain) {
		return new CServerSession(m_cnSecure, a_hDesc, EDomainIPC, this,
				m_cszLCert, m_cszLKey, m_cszRCert);
	}
};

class CTestTimer : public CTimer {
private:
	int m_nCounter;

public:
	CTestTimer(CIOEventDispatcher& a_objController) : CTimer(a_objController),
			m_nCounter(0) {}
	~CTestTimer() {}
protected:
	virtual void onTimeout(EBool a_nActive) {
		::printf("%s: l(%d): inst=%p, fd=%d\n", __FUNCTION__, __LINE__, this, getHandle());
		if (m_nCounter++ > 10) {
			clear();
		}
	}
};


int main(int argc, char *argv[])
{
	int result;

	::printf("%s: l(%d)\n", __FUNCTION__, __LINE__);

	if (argc <= 1) {
		::printf("%s: l(%d): [elf] [port] [server.crt] [server.key] [client.crt]\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	int portNo = ::atoi(argv[1]);

	CSSocket::initialLibrary();

	::printf("%s: l(%d)\n", __FUNCTION__, __LINE__);

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

	if (argc <= 2) {
		type = CSSocket::ESecureNone;
	}

	char *szLCert = (argc > 2)? argv[2] : NULL;
	char *szLKey  = (argc > 3)? argv[3] : NULL;
	char *szRCert = (argc > 4)? argv[4] : NULL;

	::printf("%s: l(%d): type=%d: %s, %s, %s\n", __FUNCTION__, __LINE__, type, szLCert, szLKey, szRCert);

	CIOEventDispatcher dispather;
	CServerListener& tlsListenerV6 = *new CServerListener(type,
			CSSocket::EDomainIPv6, (unsigned)portNo, szLCert, szLKey, szRCert);
	CServerListener& tlsListenerV4 = *new CServerListener(type,
			CSSocket::EDomainIPv4, (unsigned)portNo + 1, szLCert, szLKey,
			szRCert);

#ifndef __CYGWIN__
	CTestTimer tim(dispather);
	tim.set(5000000, CTimer::ECyclic);
#endif // __CYGWIN__

#ifdef TEST_UDS_ON
	CIpcListener& udsListener1 = *new CIpcListener(const_cast<char *>("/tmp/test_uds1"),
			type, szLCert, szLKey, szRCert);
	CIpcListener& udsListener2 = *new CIpcListener(const_cast<char *>("/tmp/test_uds2"),
			type, szLCert, szLKey, szRCert);
#endif // TEST_UDS_ON

	result = dispather.add(&tlsListenerV6);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);

	result = dispather.add(&tlsListenerV4);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);

#ifdef TEST_UDS_ON
	result = dispather.add(&udsListener1);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);
	result = dispather.add(&udsListener2);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);
#endif // TEST_UDS_ON

//	dump((unsigned char *)vp, 512);
	result = tlsListenerV6.listen(32);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);

	result = tlsListenerV4.listen(32);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);

#ifdef TEST_UDS_ON
	result = udsListener1.listen(32);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);
	result = udsListener2.listen(32);
	::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);
#endif // TEST_UDS_ON

	while (1) {
		::printf("%s: l(%d)\n", __FUNCTION__, __LINE__);
		result = dispather.dispatch(~0u);
		::printf("%s: l(%d): result=%d\n", __FUNCTION__, __LINE__, result);
	}

#ifdef TEST_UDS_ON
	dispather.del(&udsListener2);
	dispather.del(&udsListener1);
#endif // TEST_UDS_ON
	dispather.del(&tlsListenerV4);
	dispather.del(&tlsListenerV6);

	return 0;
}
