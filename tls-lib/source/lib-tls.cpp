
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>

#include <lib-tls.h>

// #define DEBUG_TLS_STATUS

/**
*/
CSSocket::CSSocket(ESecure a_nSecure, EDomain a_nDomain,
		EUsage a_nUsage)
	:	m_cnVerifyDepth(9), m_cnSecure(a_nSecure), m_cnUsage(a_nUsage),
		m_cnDomain(a_nDomain), m_cnDirection((m_cnUsage == EUsageListener)?
				EDirectionListen : EDirectionConnect),
		m_nTlsStatus(EStatusTlsDisconn), m_nDynamicDomain(EDomainNone),
		m_unConnTimeoutMs(~0u), m_cpobjListener(NULL),
		m_nConnIniStatus((m_cnDirection == EDirectionConnect)? ETrue : EFalse),
		m_nWriteWaitStatus(EFalse), m_pobjSslCtx(NULL), m_pobjSsl(NULL)
{
	assert(m_cnDomain != EDomainNone);
	if (m_cnDomain == EDomainDNS) {
		return;
	}
	/*m_nHandle = */createHandle();
	assert(isValidHandle());
}

/**
*/
CSSocket::CSSocket(ESecure a_nSecure, HANDLE a_hDesc, EDomain a_nDomain,
		CSSocket *a_pobjListener)
	:	m_cnVerifyDepth(9), m_cnSecure(a_nSecure), m_cnUsage(EUsageSession),
		m_cnDomain(a_nDomain), m_cnDirection(EDirectionAccept),
		m_nTlsStatus(EStatusTlsDisconn), m_nDynamicDomain(EDomainNone),
		m_unConnTimeoutMs(~0u), m_cpobjListener(a_pobjListener),
		m_nConnIniStatus(EFalse), m_nWriteWaitStatus(EFalse),
		m_pobjSslCtx(NULL), m_pobjSsl(NULL)
{
	assert(m_cnDomain != EDomainNone);
	assert(m_cnDomain != EDomainDNS);
	setHandle(a_hDesc);
	int flg = ::fcntl(getHandle(), F_GETFL, 0);
	flg |= O_NONBLOCK;
	::fcntl(getHandle(), F_SETFL, flg);
}

/**
*/
CSSocket::~CSSocket()
{
	if (getSsl()) {
		::SSL_free(getSsl());
		m_pobjSsl = NULL;
	}
	if (getSslCtx()) {
		::SSL_CTX_free(getSslCtx());
		m_pobjSslCtx = NULL;
	}

	if (m_cpobjListener) {
		m_cpobjListener->deregistSocket(const_cast<CSSocket *>(this));
	}

	shutdownHandle();
	closeHandle();
}

CSSocket *CSSocket::newSocket(
		HANDLE a_hDesc, EDomain a_nDomain)
{
	assert(false);
	return NULL;
}

/**
*/
unsigned char *CSSocket::propertyReceiveBuffer()
{
	assert(false);
	return NULL;
}

/**
*/
unsigned int CSSocket::propertyReceiveBufferSize()
{
	assert(false);
	return 0;
}

/**
*/
const char *CSSocket::propertyCertificateFile()
{
	return NULL;
}

/**
*/
const char *CSSocket::propertyPrivateKeyFile()
{
	return NULL;
}

/**
*/
const char *CSSocket::propertyVerifyCertificateFile()
{
	return NULL;
}

/**
*/
const char *CSSocket::propertyHostName()
{
	return NULL;
}

/**
*/
const char *CSSocket::propertyAddress()
{
	return NULL;
}

/**
*/
unsigned int CSSocket::propertyAddressSize()
{
	return 0;
}

/**
*/
const unsigned short CSSocket::propertyPort()
{
	return 0u;
}

/**
*/
const CSSocket::ESecure CSSocket::getSecure() const
{
	return m_cnSecure;
}

/**
*/
const CSSocket::EUsage CSSocket::getUsage() const
{
	return m_cnUsage;
}

/**
*/
const CSSocket::EDomain CSSocket::getDomain() const
{
	return (m_cnDomain != EDomainDNS)? m_cnDomain : m_nDynamicDomain;
}

/**
*/
const CSSocket::EDirection CSSocket::getDirection() const
{
	return m_cnDirection;
}

/**
*/
const CSSocket *CSSocket::getListener() const
{
	return m_cpobjListener;
}

/**
*/
const int CSSocket::getVerifyDepth() const
{
	return m_cnVerifyDepth;
}

/**
*/
CHandle::EBool CSSocket::isWatchDirectionOut() const
{
	if (m_nWriteWaitStatus == ETrue) {
		return ETrue;
	}
	if (getDirection() != EDirectionConnect) {
		return EFalse;	// only IN
	}
	if (getSecure() == ESecureNone) {
		return m_nConnIniStatus;	// on connect: OUT, on other: IN
	}
	if (getTlsStatus() != EStatusTlsDisconn) {
		return EFalse;	// TLS handshake or connect: IN
	}
	return ETrue;	// TLS disconnect: OUT
}

/**
*/
CHandle::EBool CSSocket::isCreateAuto() const
{
	return (m_cpobjListener)? ETrue : EFalse;
}

/**
*/
CHandle::RESULT CSSocket::read(unsigned char *a_lpbyBuffer,
		unsigned a_unLength)
{
	RESULT result = 0;

	if (!isValidHandle()) {return -1;}
	if (getSecure() == ESecureNone) {
		result = CHandle::read(a_lpbyBuffer, a_unLength);
		if (result < 0) {
			if (errno == EAGAIN) {
				result = 0;
			}
		}
		return result;
	}

	if (!getSsl()) {return -1;}
	result = ::SSL_read(getSsl(), a_lpbyBuffer, a_unLength);
	RESULT ssl_result = result;
	if (result <= 0) {
		int err_no = errno;
		switch (err_no) {
		case EAGAIN:
			result = 0;
		case 0:
			if (isSslShutdownState()) {
				result = -1;
			}
			if (isSecureError(ssl_result, err_no, ERead)) {
				result = -1;
			}
			break;
		default:
			result = -1;
			break;
		}
	}

	return result;
}

/**
*/
CHandle::RESULT CSSocket::write(unsigned char *a_lpbyBuffer,
		unsigned a_unLength)
{
	RESULT result = 0;

	if (!isValidHandle()) {return -1;}

	m_nWriteWaitStatus = EFalse;
	if (getSecure() == ESecureNone) {
		result = CHandle::write(a_lpbyBuffer, a_unLength);
		if (result < 0) {
			if (errno == EAGAIN) {
				result = 0;
				m_nWriteWaitStatus = ETrue;
				if (getDispatcher()) {
					getDispatcher()->update(this);
				}
				return result;
			}
		}
	} else {
		if (!getSsl()) {return -1;}
		result = ::SSL_write(getSsl(), a_lpbyBuffer, a_unLength);
		RESULT ssl_result = result;
		if (result <= 0) {
			int err_no = errno;
			switch (err_no) {
			case EAGAIN:
				result = 0;
				m_nWriteWaitStatus = ETrue;
				if (getDispatcher()) {
					getDispatcher()->update(this);
				}
			case 0:
				if (isSslShutdownState()) {
					result = -1;
				}
				if (isSecureError(ssl_result, err_no, EWrite)) {
					result = -1;
				}
				break;
			default:
				result = -1;
				break;
			}
		}
	}

	return result;
}

/**
*/
CHandle::RESULT CSSocket::shutdownHandle()
{
	RESULT result = 0;

	if (!isValidHandle()) {return -1;}

	if (getSecure() == ESecureNone) {
		result = ::shutdown(getHandle(), SHUT_RDWR);
	} else {
		if (!getSsl()) {return -1;}
		result = ::SSL_shutdown(getSsl());
	}

	return result;
}

/**
*/
void CSSocket::closeHandle()
{
	if (getSecure() != ESecureNone) {
		if (!isValidHandle()) {return;}
		::shutdown(getHandle(), SHUT_RDWR);
	}
	CHandle::closeHandle();
}

/**
*/
CHandle::RESULT CSSocket::setConnectionTimeout(
		unsigned a_unConnTimeoutMs)
{
	RESULT result;
	struct timeval tv;
	tv.tv_sec = a_unConnTimeoutMs / 1000;
	tv.tv_usec= a_unConnTimeoutMs % 1000 * 1000;
	if ((result = ::setsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO,
			&tv, sizeof(tv))) < 0) {
		return result;
	}
	if ((result = ::setsockopt(getHandle(), SOL_SOCKET, SO_SNDTIMEO,
			&tv, sizeof(tv))) < 0) {
		return result;
	}
	m_unConnTimeoutMs = a_unConnTimeoutMs;
	return result;
}

/**
*/
CHandle::RESULT CSSocket::isSslShutdownState()
{
	RESULT result = 0;
	if (!getSsl()) {return result;}
	result = ::SSL_get_shutdown(getSsl());
	result &= (SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
	result = (!(!result));
	return (result);
}

/**
*/
CHandle::RESULT CSSocket::isSecureError(RESULT api_err, RESULT err_no, EDataDirection dir)
{
	RESULT result = 1;
	if (!getSsl()) {return result;}
	result = ::SSL_get_error(getSsl(), api_err);

	CDebugUtil::printf(CDebugUtil::EDebug,
			"%s: l(%d): %s: api_err=%d, errno=%d:%s, ssl_error=%d, ssl_shutdown=%x\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, api_err, err_no, ::strerror(err_no),
			result, ::SSL_get_shutdown(getSsl()));

	switch (result) {
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_NONE:
		result = 0;
		break;
	case SSL_ERROR_SYSCALL:
	default:
		break;
	}
	return (result);
}

/**
*/
void CSSocket::initialLibrary()
{
	::SSL_library_init();
}

/**
*/
void CSSocket::freeLibrary()
{
}

/**
*/
SSL_CTX *CSSocket::getSslCtx() const
{
	return m_pobjSslCtx;
}

/**
*/
SSL *CSSocket::getSsl() const
{
	return m_pobjSsl;
}

/**
*/
CSSocket::EStatus CSSocket::getTlsStatus() const
{
	return m_nTlsStatus;
}

/**
*/
void CSSocket::messageCallback(int write_p, int version, int content_type,
		const void *buf, size_t len)
{
#ifdef DEBUG_TLS_STATUS
	CDebugUtil::printf(CDebugUtil::EDebug,
			"%s: l(%d): %s: w/r=%d, ver=0x%04x, ct=%d\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, write_p, version, content_type);
#endif // DEBUG_TLS_STATUS
}

/**
*/
void CSSocket::infoCallback(int type, int val)
{
	if (type & SSL_CB_HANDSHAKE_START) {
		m_nTlsStatus = EStatusTlsHandshake;
		onSecureStartHandShake();
	}
	if (type & SSL_CB_HANDSHAKE_DONE) {
		m_nTlsStatus = EStatusTlsConn;
		onSecureHandShakeDone();
	}
	if (type & SSL_CB_ALERT) {
		m_nTlsStatus = EStatusTlsDisconn;
		onSecureAlert();
	}
}

/**
*/
CSSocket::EDomain CSSocket::getDomainFromHandle()
{
#ifdef SO_DOMAIN
	int domain;
	socklen_t len = sizeof(domain);
	EDomain type;
	if (::getsockopt(getHandle(), SOL_SOCKET, SO_DOMAIN, &domain, &len)) {
		assert(false);
	}
	switch (domain) {
	case AF_INET:
		type = EDomainIPv4;
		break;
	case AF_INET6:
		type = EDomainIPv6;
		break;
	case AF_UNIX:
		type = EDomainIPC;
		break;
	default:
		type = EDomainNone;
		break;
	}
	return type;
#else  // !defined(SO_DOMAIN)
	return getDomain();
#endif // SO_DOMAIN
}

/**
*/
CHandle::HANDLE CSSocket::createHandleInternal(EDomain a_nDomain)
{
	assert(a_nDomain != EDomainNone);
	assert(a_nDomain != EDomainDNS);

	HANDLE h;
	int domain = AF_INET;
	switch (a_nDomain) {
	case EDomainIPC:
		domain = AF_UNIX;
		break;
	case EDomainIPv6:
		domain = AF_INET6;
		break;
	case EDomainIPv4:
		break;
	default:
		assert(false);
		break;
	}
	h = ::socket(domain, SOCK_STREAM, 0);
	setHandle(h);

	int flg = ::fcntl(getHandle(), F_GETFL, 0);
	flg |= O_NONBLOCK;
	::fcntl(getHandle(), F_SETFL, flg);

	return h;
}

/**
*/
CHandle::HANDLE CSSocket::createHandle()
{
	assert(m_cnDomain != EDomainNone);
	assert(m_cnDomain != EDomainDNS);

	return createHandleInternal(m_cnDomain);
}


/**
*/
CHandle::HANDLE CSSocket::createHandleAndGetAddress(
		const char *a_szHostName, char *a_szAddress, int a_nSize)
{
	assert(m_cnDomain != EDomainNone);
	assert(m_cnDomain != EDomainIPv4);
	assert(m_cnDomain != EDomainIPv6);
	assert(m_cnDomain != EDomainIPC);

	struct addrinfo hints;
	struct addrinfo *result = NULL, *rp = NULL;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	int err;
	if ((err = ::getaddrinfo(a_szHostName, NULL, &hints, &result))) {
		CDebugUtil::printf(CDebugUtil::EWarning,
				"%s: l(%d): %s: err=%s: errno=%s: %s\n",
				__FILE__, __LINE__, __PRETTY_FUNCTION__,
				::gai_strerror(err), ::strerror(errno), a_szHostName);
//		assert(false);
		m_nDynamicDomain = EDomainIPv4;
		return EInvalidHandleValue;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family == AF_INET) {
			uint32_t u_addr = ((struct sockaddr_in *)(rp->ai_addr))->sin_addr.s_addr;
			m_nDynamicDomain = EDomainIPv4;
			::inet_ntop(AF_INET, &u_addr, a_szAddress, (socklen_t)a_nSize);
			CDebugUtil::printf(CDebugUtil::EDebug,
					"%s: l(%d): %s: %s -> %s, buf_size=%d\n",
					__FILE__, __LINE__, __PRETTY_FUNCTION__, a_szHostName, a_szAddress, a_nSize);
		} else if (rp->ai_family == AF_INET6) {
			uint16_t hw_addr[sizeof(((struct sockaddr_in6 *)(rp->ai_addr))->sin6_addr.s6_addr)];
			::memcpy(hw_addr, ((struct sockaddr_in6 *)(rp->ai_addr))->sin6_addr.s6_addr, sizeof(hw_addr));
			m_nDynamicDomain = EDomainIPv6;
			::inet_ntop(AF_INET6, hw_addr, a_szAddress, (socklen_t)a_nSize);
			CDebugUtil::printf(CDebugUtil::EDebug,
					"%s: l(%d): %s: %s -> %s , buf_size=%d(addr-size=%d)\n",
					__FILE__, __LINE__, __PRETTY_FUNCTION__, a_szHostName, a_szAddress, a_nSize,
					sizeof(((struct sockaddr_in6 *)(rp->ai_addr))->sin6_addr.s6_addr));
			break;
		} else {
			assert(false);
		}
	}

	::freeaddrinfo(result);

	return createHandleInternal(m_nDynamicDomain);
}

/**
*/
void CSSocket::msgCbFunc(int write_p, int version, int content_type,
		const void *buf, size_t len, SSL *ssl, void *arg)
{
	reinterpret_cast<CSSocket *>(arg)->messageCallback(write_p, version,
			content_type, buf, len);
}

/**
*/
void CSSocket::infoCbFunc(const SSL *ssl, int type, int val)
{
#ifdef DEBUG_TLS_STATUS
	char szWhere[128];
	::memset(szWhere, 0, sizeof(szWhere));

	CAT_WHERE_STRING(szWhere, type, SSL_CB_LOOP);
	CAT_WHERE_STRING(szWhere, type, SSL_CB_EXIT);
	CAT_WHERE_STRING(szWhere, type, SSL_CB_READ);
	CAT_WHERE_STRING(szWhere, type, SSL_CB_WRITE);
	CAT_WHERE_STRING(szWhere, type, SSL_CB_ALERT);
	CAT_WHERE_STRING(szWhere, type, SSL_ST_ACCEPT);
	CAT_WHERE_STRING(szWhere, type, SSL_ST_CONNECT);
	CAT_WHERE_STRING(szWhere, type, SSL_CB_HANDSHAKE_START);
	CAT_WHERE_STRING(szWhere, type, SSL_CB_HANDSHAKE_DONE);

	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: (%s):%s\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, szWhere, ::SSL_alert_type_string(val));
#endif // DEBUG_TLS_STATUS

#ifndef __OPENSSL_CAPSULE
	void *ptr = ssl->ctx->msg_callback_arg;
#else /* defined(__OPENSSL_CAPSULE) */
	void *ptr = ::SSL_get_default_passwd_cb_userdata(const_cast<SSL *>(ssl));
#endif /* __OPENSSL_CAPSULE */
	if (ptr) {
		reinterpret_cast<CSSocket *>(ptr)->infoCallback(type, val);
	}
}

/**
*/
int CSSocket::verifyCbFunc(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
	int result = preverify_ok;
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onConnect(CIOEventDispatcher& a_objDispatcher, CSSocket*& a_pobjSocket)
{
	HANDLE h = accept();
	a_pobjSocket = newSocket(h, getDomain());
	registSocket(a_pobjSocket);
	RESULT result = a_objDispatcher.add(a_pobjSocket);
	a_pobjSocket->acceptSecure();
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onAccept(CIOEventDispatcher& a_objDispatcher)
{
	RESULT result = 0;
	m_nConnIniStatus = EFalse;
	connect(2);
	a_objDispatcher.update(this);
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onReceive(CIOEventDispatcher& a_objDispatcher)
{
	RESULT result_read;
	RESULT result = 0;
	unsigned char *p;
	int rx_len;
	do {
		rx_len = 2048;
		p = new uint8_t[rx_len];
		result_read = read(p, rx_len);
		if (result_read < 0) {
			CDebugUtil::printf(CDebugUtil::EError, "%s: l(%d): %s: fd=%d, err=%s\n",
					__FILE__, __LINE__, __PRETTY_FUNCTION__, getHandle(), strerror(errno));
			delete [] p;
			onReceiveError(a_objDispatcher);
			shutdownHandle();
			a_objDispatcher.del(this);
			return result_read;
		}
		result += result_read;
		if (result_read == 0) {
			break;
		}
		onReceiveData(a_objDispatcher, reinterpret_cast<unsigned char *>(p), result_read);
	} while (result_read > 0);
	return result;
}

/**
*/
void CSSocket::onReceiveData(CIOEventDispatcher& a_objDispatcher,
		unsigned char *a_pbyAllocateData, int a_nValidLength)
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this);
	CDebugUtil::dump(a_pbyAllocateData, a_nValidLength, __PRETTY_FUNCTION__);
	delete [] a_pbyAllocateData;
}

/**
*/
void CSSocket::onReceiveError(CIOEventDispatcher& a_objDispatcher)
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this);
}

/**
*/
CHandle::RESULT CSSocket::onEnableTransmit(CIOEventDispatcher& a_objDispatcher)
{
	RESULT result = 0;
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this);
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onSecureStartHandShake()
{
	RESULT result = 0;
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this);
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onSecureHandShakeDone()
{
	RESULT result = 0;
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this);
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onSecureAlert()
{
	RESULT result = 0;
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this);
	return result;
}

/**
*/
void CSSocket::registSocket(CSSocket *a_pobjSocket)
{
	// do nothing.
}

/**
*/
void CSSocket::deregistSocket(CSSocket *a_pobjSock)
{
	// do nothing.
}

/**
*/
CSSocketListener::CSSocketListener(EDomain a_nDomain)
	:	CSSocket(ESecureNone, a_nDomain, EUsageListener),
		m_pobjSockets(NULL), m_nSockets(0u)
{
}

/**
*/
CSSocketListener::~CSSocketListener()
{
	if (m_pobjSockets) {
		delete [] m_pobjSockets;
	}
}

/**
*/
CHandle::RESULT CSSocketListener::onConnect(CIOEventDispatcher& a_objDispatcher, CSSocket*& a_pobjSocket)
{
	RESULT result = CSSocket::onConnect(a_objDispatcher, a_pobjSocket);
	return result;
}

CSSocket *CSSocketListener::getSession(unsigned a_nIndex)
{
	if (a_nIndex >= m_nSockets) {
		return NULL;
	}
	return m_pobjSockets[a_nIndex];
}

/**
*/
void CSSocketListener::registSocket(CSSocket *a_pobjSocket)
{
	if (!m_pobjSockets) {
		m_pobjSockets = new PSSOCKET[m_nSockets + 1];
	} else {
		PSSOCKET *tmp = m_pobjSockets;
		m_pobjSockets = new PSSOCKET[m_nSockets + 1];
		::memcpy(m_pobjSockets, tmp, sizeof(PSSOCKET) * m_nSockets);
		delete [] tmp;
	}
	m_pobjSockets[m_nSockets] = a_pobjSocket;
	m_nSockets++;
}

/**
*/
void CSSocketListener::deregistSocket(CSSocket *a_pobjSock)
{
	if (!m_pobjSockets) {
		return;
	}
	m_nSockets--;
	if (m_nSockets < 1) {
		delete [] m_pobjSockets;
		m_pobjSockets = NULL;
		return;
	}

	PSSOCKET *tmp = m_pobjSockets;
	m_pobjSockets = new PSSOCKET[m_nSockets];

	unsigned i, j;
	for (i = 0, j = 0; i <= m_nSockets; i++) {
		if (tmp[i] == a_pobjSock) {
			continue;
		}
		m_pobjSockets[j] = tmp[i];
		j++;
	}

	delete [] tmp;
}

/**
*/
CHandle::RESULT CSSocket::bind(char *a_szName)
{
	RESULT result = 0;
	socklen_t len;

	switch (getDomain()) {
	case EDomainIPv4:
	{
		struct sockaddr_in addr;
		::memset(&addr, 0, sizeof(addr));
		addr.sin_port = htons(propertyPort());
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		len = sizeof(struct sockaddr_in);
		result = ::bind(getHandle(), reinterpret_cast<struct sockaddr *>(&addr), len);
		break;
	}
	case EDomainIPv6:
	{
		struct sockaddr_in6 addr;
		::memset(&addr, 0, sizeof(addr));
		addr.sin6_port = htons(propertyPort());
		addr.sin6_family = AF_INET6;
		::memset(addr.sin6_addr.s6_addr, 0, sizeof(addr.sin6_addr.s6_addr));
		len = sizeof(struct sockaddr_in6);
		result = ::bind(getHandle(), reinterpret_cast<struct sockaddr *>(&addr), len);
		break;
	}
	case EDomainIPC:
	{
		struct sockaddr_un addr;
		::memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		::strcpy(addr.sun_path, a_szName);
		len = sizeof(struct sockaddr_un);
		::unlink(a_szName);
		result = ::bind(getHandle(), reinterpret_cast<struct sockaddr *>(&addr), len);
		break;
	}
	default:
		break;
	}

	return result;
}

/**
*/
CHandle::RESULT CSSocket::listen(int a_nConnectionMax)
{
	RESULT result = 0;
	result = ::listen(getHandle(), a_nConnectionMax);
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onInput(CIOEventDispatcher& a_objDispatcher)
{
	RESULT result = -1;
	if (getUsage() == EUsageListener) {
		CSSocket* psock;
		result = onConnect(a_objDispatcher, psock);
		return result;
	}

	result = onReceive(a_objDispatcher);
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onOutput(CIOEventDispatcher& a_objDispatcher)
{
	RESULT result;
	int optval;
	socklen_t optlen = sizeof(optval);
	result = ::getsockopt(getHandle(), SOL_SOCKET, SO_ERROR,
			&optval, &optlen);
	if (optval != 0) {
		result = -1;
		return result;
	}

	if (m_nWriteWaitStatus == ETrue) {
		m_nWriteWaitStatus = EFalse;
		if (getDispatcher()) {
			getDispatcher()->update(this);
		}
		result = onEnableTransmit(a_objDispatcher);
		return result;
	}
	result = onAccept(a_objDispatcher);
	return result;
}

/**
*/
CHandle::RESULT CSSocket::onError(CIOEventDispatcher& a_objDispatcher, int a_nError)
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p: errno=%d:%s\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this, a_nError, ::strerror(a_nError));
	a_objDispatcher.del(this);
	return -1;
}

/**
*/
CHandle::RESULT CSSocket::onRemoteError(CIOEventDispatcher& a_objDispatcher, int a_nError)
{
	CDebugUtil::printf(CDebugUtil::EDebug, "%s: l(%d): %s: inst=%p: errno=%d:%s\n",
			__FILE__, __LINE__, __PRETTY_FUNCTION__, this, a_nError, ::strerror(a_nError));
	a_objDispatcher.del(this);
	return -1;
}

/**
*/
CHandle::HANDLE CSSocket::accept()
{
	HANDLE result = 0;
	switch (getDomain()) {
	case EDomainIPv4:
	{
		struct sockaddr_in addr;
		socklen_t size = sizeof(addr);
		result = ::accept(getHandle(), (struct sockaddr *)&addr, &size);
		break;
	}
	case EDomainIPv6:
	{
		struct sockaddr_in addr;
		socklen_t size = sizeof(addr);
		result = ::accept(getHandle(), (struct sockaddr *)&addr, &size);
		break;
	}
	case EDomainIPC:
	{
		struct sockaddr_un addr;
		socklen_t size = sizeof(addr);
		result = ::accept(getHandle(),
				reinterpret_cast<struct sockaddr *>(&addr), &size);
		break;
	}
	default:
		assert(false);
		break;
	}
	return result;
}

/**
*/
CSSocketSession::CSSocketSession(ESecure a_nSecure, EDomain a_nDomain)
	:	CSSocket(a_nSecure, a_nDomain, EUsageSession),
				m_cbVerify(SSL_VERIFY_PEER)
{
//	loadSslConfig();
}

/**
*/
CSSocketSession::CSSocketSession(ESecure a_nSecure,
		HANDLE a_hDesc, EDomain a_nDomain, CSSocket *a_pobjListener)
	:	CSSocket(a_nSecure, a_hDesc, a_nDomain, a_pobjListener),
			m_cbVerify(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
{
//	loadSslConfig();
}

/**
*/
CSSocketSession::~CSSocketSession()
{
}

/**
*/
CHandle::RESULT CSSocket::connect(int a_nPhase)
{
	RESULT result = 0;
	assert((a_nPhase >= 1) && (a_nPhase <= 2));
	if (a_nPhase == 1) {
		m_nConnIniStatus = ETrue;
		switch (getDomain()) {
		case EDomainIPv4:
		{
			struct sockaddr_in addr;
			::memset(&addr, 0, sizeof(addr));
			addr.sin_port = htons(propertyPort());
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = inet_addr(propertyAddress());
			result = ::connect(getHandle(), reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
			break;
		}
		case EDomainIPv6:
		{
			struct sockaddr_in6 addr;
			::memset(&addr, 0, sizeof(addr));
			addr.sin6_port = htons(propertyPort());
			addr.sin6_family = AF_INET6;
			::inet_pton(AF_INET6, propertyAddress(), addr.sin6_addr.s6_addr);
			result = ::connect(getHandle(), reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
			break;
		}
		case EDomainIPC:
		{
			struct sockaddr_un addr;
			::memset(&addr, 0, sizeof(addr));
			addr.sun_family = AF_UNIX;
			::strcpy(addr.sun_path, propertyAddress());
			result = ::connect(getHandle(), reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
			break;
		}
		default:
			break;
		}

		if (result == -1) {
			if (errno == EINPROGRESS) {
				result = 0;
			}
			return result;
		}
	}

	if (getSecure() == ESecureNone) {
		return result;
	}

	result = ::SSL_connect(getSsl());
	return result;
}

/**
*/
CHandle::RESULT CSSocket::acceptSecure()
{
	RESULT result = 0;
	if (getDirection() != EDirectionAccept) {
		assert(false);
	}
	if (getSecure() != ESecureNone) {
		result = ::SSL_accept(getSsl());
	}
	return result;
}

/**
*/
void CSSocketSession::loadSslConfig()
{
	if (getDomain() == EDomainNone) {
		createHandleAndGetAddress(propertyHostName(),
				const_cast<char *>(propertyAddress()),
				propertyAddressSize());
	}

	SSL_METHOD *method = NULL;
	if (getDirection() == EDirectionListen) {
		assert(false);
		return;
	}

	switch (getSecure()) {
	case ESecureTls:
#if 0
		method = (SSL_METHOD *)((getDirection() == EDirectionAccept)?
				TLS_server_method() : TLS_client_method());
		break;
#endif
	case ESecureTls_1:
		method = (SSL_METHOD *)((getDirection() == EDirectionAccept)?
				TLSv1_server_method() : TLSv1_client_method());
		break;

	case ESecureTls_1_1:
		method = (SSL_METHOD *)((getDirection() == EDirectionAccept)?
				TLSv1_1_server_method() : TLSv1_1_client_method());
		break;

	case ESecureTls_1_2:
		method = (SSL_METHOD *)((getDirection() == EDirectionAccept)?
				TLSv1_2_server_method() : TLSv1_2_client_method());
		break;

	case ESecureNone:
	default:
		return;
	}

	m_pobjSslCtx = ::SSL_CTX_new(method);

#ifndef __OPENSSL_CAPSULE
	::SSL_CTX_set_msg_callback_arg(getSslCtx(), this);
#else /* __OPENSSL_CAPSULE */
	::SSL_CTX_set_default_passwd_cb_userdata(getSslCtx(), this);
#endif /* __OPENSSL_CAPSULE */
	::SSL_CTX_set_msg_callback(getSslCtx(), msgCbFunc);
	::SSL_CTX_set_info_callback(getSslCtx(), infoCbFunc);

	if (propertyCertificateFile()) {
		::SSL_CTX_use_certificate_file(getSslCtx(), propertyCertificateFile(),
				SSL_FILETYPE_PEM);
	}
	if (propertyPrivateKeyFile()) {
		::SSL_CTX_use_PrivateKey_file(getSslCtx(), propertyPrivateKeyFile(),
				SSL_FILETYPE_PEM);
	}
	if (propertyVerifyCertificateFile()) {
		::SSL_CTX_load_verify_locations(getSslCtx(),
				propertyVerifyCertificateFile(), NULL);
		::SSL_CTX_set_verify(getSslCtx(), m_cbVerify, verifyCbFunc);
		::SSL_CTX_set_verify_depth(getSslCtx(), getVerifyDepth());
	}

	m_pobjSsl = ::SSL_new(getSslCtx());
	::SSL_set_fd(getSsl(), getHandle());
}



