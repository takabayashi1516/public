package com.example.demo.tcp.client;

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
import org.springframework.integration.dsl.Transformers;
import org.springframework.integration.ip.dsl.Tcp;
import org.springframework.integration.ip.tcp.connection.AbstractClientConnectionFactory;
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
import org.springframework.messaging.support.MessageBuilder;

@Configuration
@Profile("client")
public class TcpClientController {
	@Value("${tcp_port}")
	private int mPort;
	@Value("${tcp_address}")
	private String mAddress;
	@Autowired
	private SocketManager mSocketManager;
	@Autowired
	private TcpClientEventHandler mEventHandler;

	@Bean
	public TcpClientDispatcher createTcpClientDispatcher() {
		return new TcpClientDispatcher(this);
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
		mEventHandler.onTcpClientReceive((byte[])m.getPayload(),
				(long)m.getHeaders().get("timestamp"));
	}

	@Bean
	public IntegrationFlow integrationOutboundFlow(ApplicationEventPublisher publisher) {
		AbstractClientConnectionFactory factory = Tcp.nioClient(mAddress, mPort)
				.serializer(TcpCodecs.crlf())
				.deserializer(TcpCodecs.crlf())
				.tcpSocketSupport((TcpSocketSupport) socketManager())
				.get();
		factory.setApplicationEventPublisher(publisher);
		return IntegrationFlows.from(Tcp.inboundGateway(factory).clientMode(true))
//				.transform(Transformers.objectToString())
				.handle("createTcpClientDispatcher", "onReceive")
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

	public int send(byte[] data) throws Exception {
		return mSocketManager.send(data);
	}

	static class SocketManager extends DefaultTcpSocketSupport {
		private TcpConnection mSocket = null;

		public int send(byte[] data) throws Exception {
			ByteBuffer b = ByteBuffer.wrap(data);
			return mSocket.getSocketInfo().getChannel().write(b);
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
			mSocket = conn;
		}

		@EventListener
		public void handleTcpConnectionCloseEvent(TcpConnectionCloseEvent event) {
			String peer = closeConnection();
			System.out.println("TcpConnectionCloseEvent: " + peer);
		}

		@EventListener
		public void handleTcpConnectionExceptionEvent(TcpConnectionExceptionEvent event) {
			String peer = closeConnection();
			System.out.println("TcpConnectionExceptionEvent: " + peer);
		}

		@EventListener
		public void handleTcpConnectionFailedCorrelationEvent(TcpConnectionFailedCorrelationEvent event) {
			String peer = closeConnection();
			System.out.println("TcpConnectionFailedCorrelationEvent: " + peer);
		}

		private String closeConnection() {
			String peer = mSocket.getSocketInfo().getRemoteSocketAddress().toString();
			mSocket.close();
			mSocket = null;
			return peer;
		}
	}
}
