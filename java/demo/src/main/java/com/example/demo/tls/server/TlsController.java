package com.example.demo.tls.server;

import java.util.Collections;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

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
import org.springframework.integration.ip.tcp.connection.TcpConnectionServerListeningEvent;
import org.springframework.integration.ip.tcp.connection.TcpListener;
import org.springframework.integration.ip.tcp.connection.TcpMessageMapper;
import org.springframework.integration.ip.tcp.connection.TcpNioServerConnectionFactory;
import org.springframework.messaging.Message;
import org.springframework.messaging.support.GenericMessage;

@Configuration
@Profile("server")
public class TlsController implements TcpListener, ApplicationEventPublisher {
	@Value("${tls_port}")
	private int mPort;
	@Autowired
	private TlsEventHandler mEventHandler;
	@Value("${server.ssl.key-store}")
	private String mKeyStorePath;
	@Value("${server.ssl.key-store-password}")
	private String mKeyStorePasswd;
	@Value("${server.ssl.trust-key-store}")
	private String mTrustKeyStorePath;
	@Value("${server.ssl.trust-key-store-password}")
	private String mTrustKeyStorePasswd;

	TcpNioServerConnectionFactory mServer;
	private final Map<String, TcpConnection> mSockets = new ConcurrentHashMap<>();

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
		TcpNioServerConnectionFactory server =
				new TcpNioServerConnectionFactory(mPort);
		server.setTcpNioConnectionSupport(connectionSupport());
		server.registerListener(this);
		server.setMapper(new SSLMapper());
		server.setApplicationEventPublisher(this);
		server.start();
		mServer = server;
	}

	@Override
	public boolean onMessage(Message<?> m) {
		if (mEventHandler == null) {
			return false;
		}
		try {
			String id = (String) m.getHeaders().get("ip_address");
			id += ":";
			id += String.valueOf((int) m.getHeaders().get("ip_tcp_remotePort"));
			mEventHandler.onTlsReceive(id, (byte[]) m.getPayload(),
					(long) m.getHeaders().get("timestamp"));
		} catch (Exception e) {
//			e.printStackTrace();
		}
		return true;
	}

	public void send(String id, byte[] data) throws Exception {
		TcpConnection conn = mSockets.get(id);
		conn.send(new GenericMessage<byte[]>(data));
	}

	@Override
	public void publishEvent(Object event) {
		if (event instanceof TcpConnectionServerListeningEvent) {
			System.out.println("TcpConnectionServerListeningEvent: ");
		}
		if (event instanceof TcpConnectionOpenEvent) {
			TcpConnection conn = (TcpConnection) ((TcpConnectionOpenEvent) event).getSource();
			String id = conn.getSocketInfo().getInetAddress().getHostAddress();
			id += ":";
			id += String.valueOf(conn.getSocketInfo().getPort());
			mSockets.put(id, conn);
			System.out.println("TcpConnectionOpenEvent: " + id);
		}
		if (event instanceof TcpConnectionCloseEvent) {
			TcpConnection conn = (TcpConnection) ((TcpConnectionCloseEvent) event).getSource();
			String id = conn.getSocketInfo().getRemoteSocketAddress().toString();
			id += ":";
			id += String.valueOf(conn.getSocketInfo().getPort());
			mSockets.remove(id);
			System.out.println("TcpConnectionCloseEvent: " + id);
		}
		if (event instanceof TcpConnectionExceptionEvent) {
			TcpConnection conn = (TcpConnection) ((TcpConnectionExceptionEvent) event).getSource();
			String id = conn.getSocketInfo().getRemoteSocketAddress().toString();
			id += ":";
			id += String.valueOf(conn.getSocketInfo().getPort());
			mSockets.remove(id);
			System.out.println("TcpConnectionCloseEvent: " + id);
		}
		if (event instanceof TcpConnectionFailedCorrelationEvent) {
			TcpConnection conn = (TcpConnection) ((TcpConnectionFailedCorrelationEvent) event).getSource();
			String id = conn.getSocketInfo().getRemoteSocketAddress().toString();
			id += ":";
			id += String.valueOf(conn.getSocketInfo().getPort());
			mSockets.remove(id);
			System.out.println("TcpConnectionCloseEvent: " + id);
		}
		if (event instanceof TcpConnectionFailedEvent) {
//			System.out.println("TcpConnectionFailedEvent: " + ((TcpConnectionFailedEvent) event).toString());
		}
	}

	private static class SSLMapper extends TcpMessageMapper {
		@Override
		protected Map<String, ?> supplyCustomHeaders(TcpConnection connection) {
			return Collections.singletonMap("cipher", connection.getSslSession().getCipherSuite());
		}

	}
}
