package com.example.demo.udp.server;

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
import org.springframework.messaging.Message;

@Configuration
@Profile("server")
public class UdpController implements ApplicationContextAware {
	@Value("${udp_port}")
	private int mUdpPort;
	private UdpEventHandler mEventHandler = null;
	private ApplicationContext mContext = null;

	public UdpController() {
	}

	@Bean
	public UdpDispatcher createUdpDispatcher() {
		return new UdpDispatcher(this);
	}

	@Bean
	public IntegrationFlow processUdpMessage() {
		return IntegrationFlows.from(new UnicastReceivingChannelAdapter(mUdpPort))
				.handle("createUdpDispatcher", "onReceive").get();
	}

	public void onReceive(Message<byte[]> m) {
		if (mEventHandler == null) {
			return;
		}
		mEventHandler.onUdpReceive((String)m.getHeaders().get("ip_address"),
				(int)m.getHeaders().get("ip_port"), (byte[])m.getPayload(),
				(long)m.getHeaders().get("timestamp"));
	}

	@Override
	public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
		// TODO Auto-generated method stub
		mContext = applicationContext;
		mEventHandler = mContext.getBean(UdpEventHandler.class);
//		mEventHandler.onReceive(null);
	}
}
