package com.example.demo.tcp.server;

import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;
import org.springframework.context.event.EventListener;
import org.springframework.integration.core.MessagingTemplate;
import org.springframework.integration.dsl.IntegrationFlow;
import org.springframework.integration.dsl.IntegrationFlows;
import org.springframework.integration.ip.dsl.Tcp;
import org.springframework.integration.ip.tcp.connection.AbstractServerConnectionFactory;
import org.springframework.integration.ip.tcp.connection.DefaultTcpSocketSupport;
import org.springframework.integration.ip.tcp.connection.TcpConnection;
import org.springframework.integration.ip.tcp.connection.TcpConnectionCloseEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionExceptionEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionFailedCorrelationEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionOpenEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionServerExceptionEvent;
import org.springframework.integration.ip.tcp.connection.TcpConnectionServerListeningEvent;
import org.springframework.integration.ip.tcp.connection.TcpSocketSupport;
import org.springframework.integration.ip.tcp.serializer.TcpCodecs;
import org.springframework.messaging.Message;

@Configuration
@Profile("server")
public class TcpController {
	@Value("${tcp_port}")
	private int mPort;
	@Autowired
	private SocketManager mSocketManager;
	@Autowired
	private TcpEventHandler mEventHandler;

	@Bean
	public TcpDispatcher createTcpDispatcher() {
		return new TcpDispatcher(this);
	}

	public void onReceive(Message<byte[]> m) {
		if (mEventHandler == null) {
			return;
		}
//		System.out.println(m.getHeaders());
		String id = (String)m.getHeaders().get("ip_hostname");
		id += "/";
		id += (String)m.getHeaders().get("ip_address");
		id += ":";
		id += String.valueOf((int)m.getHeaders().get("ip_tcp_remotePort"));
		mEventHandler.onTcpReceive(id, (byte[])m.getPayload(),
				(long)m.getHeaders().get("timestamp"));
	}

	@Bean
	public IntegrationFlow integrationInboundFlow(ApplicationEventPublisher publisher) {
		mSocketManager = socketManager();
		AbstractServerConnectionFactory factory = Tcp.nioServer(mPort)
				.serializer(TcpCodecs.crlf())
				.deserializer(TcpCodecs.crlf())
				.tcpSocketSupport((TcpSocketSupport) mSocketManager)
				.get();
		factory.setApplicationEventPublisher(publisher);
		return IntegrationFlows.from(Tcp.inboundGateway(factory))
//				.transform(Transformers.objectToString())
//				.transform(m -> m)
				.handle("createTcpDispatcher", "onReceive")
				.get();
	}

	@Bean
	public SocketManager socketManager() {
		return new SocketManager();
	}

	@Bean
	public MessagingTemplate messagingTemplate() {
		return new MessagingTemplate();
	}

	public int send(String peerAddress, byte[] data) throws Exception {
		return mSocketManager.send(peerAddress, data);
	}

	public List<String> getPeers() {
		return mSocketManager.getPeers();
	}

	static class SocketManager extends DefaultTcpSocketSupport {
		private final Map<String, TcpConnection> mSockets = new ConcurrentHashMap<>();

		public int send(String peerAddress, byte[] data) throws Exception {
			TcpConnection conn = mSockets.get(peerAddress);
			if (conn == null) {
				return -1;
			}
			ByteBuffer b = ByteBuffer.wrap(data);
			return conn.getSocketInfo().getChannel().write(b);
		}

		public List<String> getPeers() {
			List<String> peers = new ArrayList<String>();
			for (String key : mSockets.keySet()) {
				peers.add(key);
			}
			return peers;
		}

		@Override
		public void postProcessSocket(Socket socket) {
			super.postProcessSocket(socket);
		}

		@EventListener
		public void handleTcpConnectionOpenEvent(TcpConnectionOpenEvent event) {
			TcpConnection conn = (TcpConnection) event.getSource();
			String peer = conn.getSocketInfo().getRemoteSocketAddress().toString();
			System.out.println("TcpConnectionOpenEvent: " + peer);
			mSockets.put(peer, conn);
		}

		@EventListener
		public void handleTcpConnectionCloseEvent(TcpConnectionCloseEvent event) {
			TcpConnection conn = (TcpConnection) event.getSource();
			String peer = closeConnection(conn);
			System.out.println("TcpConnectionCloseEvent: " + peer);
		}

		@EventListener
		public void handleTcpConnectionExceptionEvent(TcpConnectionExceptionEvent event) {
			TcpConnection conn = (TcpConnection) event.getSource();
			String peer = closeConnection(conn);
			System.out.println("TcpConnectionExceptionEvent: " + peer);
		}

		@EventListener
		public void handleTcpConnectionFailedCorrelationEvent(TcpConnectionFailedCorrelationEvent event) {
			TcpConnection conn = (TcpConnection) event.getSource();
			String peer = closeConnection(conn);
			System.out.println("TcpConnectionFailedCorrelationEvent: " + peer);
		}

		@EventListener
		public void handleTcpConnectionServerListeningEvent(TcpConnectionServerListeningEvent event) {
		}

		@EventListener
		public void handleTcpConnectionServerExceptionEvent(TcpConnectionServerExceptionEvent event) {
			TcpConnection conn = (TcpConnection) event.getSource();
			String peer = closeConnection(conn);
			System.out.println("TcpConnectionServerExceptionEvent: " + peer);
		}

		private String closeConnection(TcpConnection conn) {
			String peer = conn.getSocketInfo().getRemoteSocketAddress().toString();
			conn.close();
			mSockets.remove(peer);
			return peer;
		}
	}
}
