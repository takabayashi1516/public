package com.example.demo.tls.client;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;
import org.springframework.integration.ip.tcp.connection.DefaultTcpNioSSLConnectionSupport;
import org.springframework.integration.ip.tcp.connection.DefaultTcpSSLContextSupport;
import org.springframework.integration.ip.tcp.connection.TcpConnection;
import org.springframework.integration.ip.tcp.connection.TcpConnectionCloseEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionExceptionEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionFailedCorrelationEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionFailedEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionOpenEvent;
import org.springframework.integration.ip.tcp.connection.TcpListener;
import org.springframework.integration.ip.tcp.connection.TcpNioClientConnectionFactory;
import org.springframework.messaging.Message;
import org.springframework.messaging.support.GenericMessage;

@Configuration
@Profile("client")
public class TlsClientController implements TcpListener, ApplicationEventPublisher {
	@Value("${tls_port}")
	private int mPort;
	@Value("${tls_address}")
	private String mAddress;
	@Autowired
	private TlsClientEventHandler mEventHandler;
	@Value("${server.ssl.key-store}")
	private String mKeyStorePath;
	@Value("${server.ssl.key-store-password}")
	private String mKeyStorePasswd;
	@Value("${server.ssl.trust-key-store}")
	private String mTrustKeyStorePath;
	@Value("${server.ssl.trust-key-store-password}")
	private String mTrustKeyStorePasswd;

	TcpNioClientConnectionFactory mClient;

	@Autowired
	public void context(ApplicationContext context) {
		createConnection();
	}

	public DefaultTcpNioSSLConnectionSupport connectionSupport() {
		DefaultTcpSSLContextSupport sslContextSupport =
				new DefaultTcpSSLContextSupport(mKeyStorePath, mTrustKeyStorePath,
						mKeyStorePasswd, mTrustKeyStorePasswd);
		sslContextSupport.setProtocol("TLS");
		DefaultTcpNioSSLConnectionSupport tcpNioConnectionSupport =
				new DefaultTcpNioSSLConnectionSupport(sslContextSupport, false);
		return tcpNioConnectionSupport;
	}

	public void createConnection() {
		TcpNioClientConnectionFactory client =
				new TcpNioClientConnectionFactory(mAddress, mPort);
		client.setTcpNioConnectionSupport(connectionSupport());
		client.registerListener(this);
		client.setApplicationEventPublisher(this);
		client.start();
		mClient = client;
	}

	@Override
	public boolean onMessage(Message<?> m) {
		if (mEventHandler == null) {
			return false;
		}
		try {
			String id = (String) m.getHeaders().get("ip_hostname");
			id += "/";
			id += (String) m.getHeaders().get("ip_address");
			id += ":";
			id += String.valueOf((int) m.getHeaders().get("ip_tcp_remotePort"));
			mEventHandler.onTlsClientReceive((byte[]) m.getPayload(),
					(long) m.getHeaders().get("timestamp"));
		} catch (Exception e) {
//			e.printStackTrace();
		}
		return true;
	}

	public void send(byte[] data) throws Exception {
		TcpConnection conn = mClient.getConnection();
		conn.send(new GenericMessage<byte[]>(data));
	}

	@Override
	public void publishEvent(Object event) {
		if (event instanceof TcpConnectionOpenEvent) {
			TcpConnection conn = (TcpConnection) ((TcpConnectionOpenEvent) event).getSource();
			String peer = conn.getSocketInfo().getRemoteSocketAddress().toString();
			System.out.println("TcpConnectionOpenEvent: " + peer);
		}
		if (event instanceof TcpConnectionCloseEvent) {
//			System.out.println("TcpConnectionCloseEvent: " + ((TcpConnectionCloseEvent) event).getConnectionId());
		}
		if (event instanceof TcpConnectionExceptionEvent) {
//			System.out.println("TcpConnectionExceptionEvent: " + ((TcpConnectionExceptionEvent) event).getConnectionId());
		}
		if (event instanceof TcpConnectionFailedCorrelationEvent) {
//			System.out.println("TcpConnectionFailedCorrelationEvent: " + ((TcpConnectionFailedCorrelationEvent) event).getConnectionId());
		}
		if (event instanceof TcpConnectionFailedEvent) {
//			System.out.println("TcpConnectionFailedEvent: " + ((TcpConnectionFailedEvent) event).toString());
		}
	}
}
