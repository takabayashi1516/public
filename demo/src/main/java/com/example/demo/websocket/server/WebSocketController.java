package com.example.demo.websocket.server;

import java.io.IOException;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Controller;
import org.springframework.web.socket.BinaryMessage;
import org.springframework.web.socket.WebSocketSession;

@Controller
@Profile("server")
public class WebSocketController implements ApplicationContextAware {
	private ApplicationContext mContext = null;
	@Autowired
	private WebSocketConfig mConfig;
	@Autowired
	private WebSocketDispacher mDispacher;

	public WebSocketController() {
	}

	public void broadCast(byte[] data) throws IOException {
		mDispacher.broadCast(data);
	}

	public void send(String id, byte[] data) throws IOException {
		mDispacher.send(id, data);
	}

	public void send(WebSocketSession session, byte[] data) throws IOException {
		mDispacher.send(session, data);
	}

	@Override
	public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
		mContext = applicationContext;
//		mConfig = mContext.getBean(WebSocketConfig.class);
		mDispacher.setHandler(mContext.getBean(WebSocketEventHandler.class));
	}
}