//
//
//
#ifndef __LIB_TLS_H
#define __LIB_TLS_H


#include <openssl/ssl.h>
#include <lib-event.h>


// #define USE_CERT_VERIFY_CALLBACK

#ifdef __cplusplus

#define CAT_WHERE_STRING(str, type, where) \
	(((where & type)) && ((*str)? ::strcat(str, " | " #where) : \
			::strcat(str, #where)))


/**
*/
class CSSocket : public CHandle {
public:
	enum EDomain {
		EDomainIPv4 = 0,
		EDomainIPv6,
		EDomainIPC,
		EDomainDNS,
		EDomainNone,
		EDomainSupremum = EDomainNone
	};

	enum EUsage {
		EUsageListener = 0,
		EUsageSession,
		EUsageSupremum
	};

	enum ESecure {
		ESecureNone = 0,
		ESecureTls,
		ESecureTls_1,
		ESecureTls_1_1,
		ESecureTls_1_2,
		ESecureSupremum
	};

	enum EStatus {
		EStatusTlsDisconn = 0,
		EStatusTlsHandshake,
		EStatusTlsConn,
		EStatusSupremum
	};

	enum EDirection {
		EDirectionConnect = 0,
		EDirectionAccept,
		EDirectionListen,
		EDirectionSupremum
	};

	enum EDataDirection {
		ERead = 0,
		EWrite
	};

private:
	const int			m_cnVerifyDepth;
	const ESecure		m_cnSecure;
	const EUsage		m_cnUsage;
	const EDomain		m_cnDomain;
	const EDirection	m_cnDirection;
	EStatus				m_nTlsStatus;
	EDomain				m_nDynamicDomain;
	unsigned int		m_unConnTimeoutMs;

protected:
	const CSSocket		*m_cpobjListener;
	EBool				m_nConnIniStatus;
	EBool				m_nWriteWaitStatus;
	SSL_CTX				*m_pobjSslCtx;
	SSL					*m_pobjSsl;

public:
	virtual ~CSSocket();
	const ESecure getSecure() const;
	const EUsage getUsage() const;
	const EDomain getDomain() const;
	const EDirection getDirection() const;
	const CSSocket *getListener() const;

	virtual RESULT read(unsigned char *a_lpbyBuffer, unsigned a_unLength);
	virtual RESULT write(unsigned char *a_lpbyBuffer, unsigned a_unLength);
	virtual RESULT shutdownHandle();
	virtual void closeHandle();
	RESULT setConnectionTimeout(unsigned a_unConnTimeoutMs);

	SSL_CTX *getSslCtx() const;
	SSL *getSsl() const;
	EStatus getTlsStatus() const;
	HANDLE accept();
	RESULT acceptSecure();
	RESULT connect(int a_nPhase = 1);
	RESULT listen(int a_nConnectionMax);

	virtual EBool isWatchDirectionOut() const;
	virtual EBool isCreateAuto() const;

	virtual RESULT onInput(CIOEventDispatcher& a_objDispatcher);
	virtual RESULT onOutput(CIOEventDispatcher& a_objDispatcher);
	virtual RESULT onError(CIOEventDispatcher& a_objDispatcher, int a_nError);
	virtual RESULT onRemoteError(CIOEventDispatcher& a_objDispatcher, int a_nError);

	static void initialLibrary();
	static void freeLibrary();

protected:
	CSSocket(ESecure a_nSecure, EDomain a_nDomain, EUsage a_nUsage);
	CSSocket(ESecure a_nSecure, HANDLE a_hDesc, EDomain a_nDomain, CSSocket *a_pobjListener);

	virtual CSSocket *newSocket(HANDLE a_hDesc, EDomain a_nDomain);
	virtual unsigned char *propertyReceiveBuffer();
	virtual unsigned int propertyReceiveBufferSize();
	virtual const char *propertyCertificateFile();
	virtual const char *propertyPrivateKeyFile();
	virtual const char *propertyVerifyCertificateFile();
	virtual const char *propertyHostName();
	virtual const char *propertyAddress();
	virtual unsigned int propertyAddressSize();
	virtual const unsigned short propertyPort();

	virtual RESULT onConnect(CIOEventDispatcher& a_objDispatcher, CSSocket*& a_pobjSocket);
	virtual RESULT onAccept(CIOEventDispatcher& a_objDispatcher);
	virtual RESULT onReceive(CIOEventDispatcher& a_objDispatcher);
	virtual void onReceiveData(CIOEventDispatcher& a_objDispatcher, unsigned char *a_pbyAllocateData, int a_nValidLength);
	virtual void onReceiveError(CIOEventDispatcher& a_objDispatcher);
	virtual RESULT onEnableTransmit(CIOEventDispatcher& a_objDispatcher);

	virtual RESULT onSecureStartHandShake();
	virtual RESULT onSecureHandShakeDone();
	virtual RESULT onSecureAlert();

	virtual void registSocket(CSSocket *a_pobjSocket);
	virtual void deregistSocket(CSSocket *a_pobjSock);

	const int getVerifyDepth() const;

	void messageCallback(int write_p, int version, int content_type,
			const void *buf, size_t len);
	void infoCallback(int type, int val);

	EDomain getDomainFromHandle();
	HANDLE createHandle();
	HANDLE createHandleAndGetAddress(
			const char *a_szHostName, char *a_szAddress, int a_nSize);

	RESULT isSslShutdownState();
	RESULT isSecureError(RESULT api_err, RESULT err_no, EDataDirection dir);

	RESULT bind(char *a_szName = NULL);

	static void msgCbFunc(int write_p, int version, int content_type,
			const void *buf, size_t len, SSL *ssl, void *arg);
	static void infoCbFunc(const SSL *ssl, int type, int val);
	static int verifyCbFunc(int preverify_ok, X509_STORE_CTX *x509_ctx);

private:
	HANDLE createHandleInternal(EDomain a_nDomain);
};
typedef CSSocket *PSSOCKET;

/**
TODO: del entry
*/
class CSSocketListener : public CSSocket {
private:
	PSSOCKET	*m_pobjSockets;
	unsigned	m_nSockets;

public:
	CSSocketListener(EDomain a_nDomain);
	virtual ~CSSocketListener();
	CSSocket *getSession(unsigned a_nIndex);

protected:
	virtual RESULT onConnect(CIOEventDispatcher& a_objDispatcher, CSSocket*& a_pobjSocket);
	virtual void registSocket(CSSocket *a_pobjSocket);
	virtual void deregistSocket(CSSocket *a_pobjSock);
};

/**
*/
class CSSocketSession : public CSSocket {
private:
	const int	m_cbVerify;

public:
	CSSocketSession(ESecure a_nSecure, EDomain a_nDomain);
	CSSocketSession(ESecure a_nSecure, HANDLE a_hDesc, EDomain a_nDomain,
			CSSocket *a_pobjListener);
	virtual ~CSSocketSession();

protected:
	void loadSslConfig();
};


#endif /* __cplusplus */


#endif /* __LIB_TLS_H */
