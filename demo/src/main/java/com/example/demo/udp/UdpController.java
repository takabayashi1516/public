package com.example.demo.udp;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;
import org.springframework.integration.dsl.IntegrationFlow;
import org.springframework.integration.dsl.IntegrationFlows;
import org.springframework.integration.ip.udp.UnicastReceivingChannelAdapter;
import org.springframework.integration.ip.udp.UnicastSendingMessageHandler;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.messaging.Message;

@Configuration
public class UdpController implements ApplicationContextAware {
	@Value("${udp_port}")
	private int mUdpPort;
	@Value("${udp_remote_port}")
	private int mUdpRemotePort;
	@Value("${udp_remote_address}")
	private String mUdpRemoteHost;
	private UnicastSendingMessageHandler mTxHandler;

	private UdpEventHandler mEventHandler = null;
	private ApplicationContext mContext = null;

	public UdpController() {
	}

	public void send(byte[] payload) {
		mTxHandler.handleMessage(MessageBuilder.withPayload(payload).build());
	}

	public void send(String payload) {
		mTxHandler.handleMessage(MessageBuilder.withPayload(payload).build());
	}

	@Bean
	public UdpDispatcher createUdpDispatcher() {
		return new UdpDispatcher(this);
	}

	@Bean
	public IntegrationFlow udpIn() {
		return IntegrationFlows.from(new UnicastReceivingChannelAdapter(mUdpPort))
				.handle("createUdpDispatcher", "onReceive").get();
	}

	public void onReceive(Message<byte[]> m) {
		if (mEventHandler == null) {
			return;
		}
		mEventHandler.onUdpReceive(m);

	}

	@Override
	public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
		mContext = applicationContext;
		mEventHandler = mContext.getBean(UdpEventHandler.class);
		mTxHandler = new UnicastSendingMessageHandler(mUdpRemoteHost, mUdpRemotePort);
//		mEventHandler.onReceive(null);
	}
}
