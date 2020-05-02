package com.example.demo.websocket.server;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.BinaryMessage;
import org.springframework.web.socket.CloseStatus;
//import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketHandler;
import org.springframework.web.socket.WebSocketMessage;
import org.springframework.web.socket.WebSocketSession;

@Component
@Profile("server")
public class WebSocketDispacher implements WebSocketHandler {
	@Autowired
	private WebSocketController mController;
	private WebSocketEventHandler mHandler;
	private HashMap<String, WebSocketSession> mSessionList;

	public WebSocketDispacher() {
		mSessionList = new HashMap<String, WebSocketSession>();
	}

	public boolean setHandler(WebSocketEventHandler handler) {
		if (mHandler != null) {
			return false;
		}
		mHandler = handler;
		return true;
	}

	public void send(String id, byte[] data) throws IOException {
		if (!(mSessionList.keySet().contains(id))) {
			return;
		}
		send(mSessionList.get(id), data);
	}

	private void send(WebSocketSession session, byte[] data) throws IOException {
		BinaryMessage message = new BinaryMessage(data);
		if (session.isOpen()) {
			session.sendMessage(message);
		}
	}

	public void broadCast(byte[] data) throws IOException {
		BinaryMessage message = new BinaryMessage(data);
		for (Map.Entry<String, WebSocketSession> entry : mSessionList.entrySet()) {
			WebSocketSession session = entry.getValue();
			if (session.isOpen()) {
				session.sendMessage(message);
			}
		}
	}

	@Override
	public void afterConnectionEstablished(WebSocketSession session) throws Exception {
		mSessionList.put(session.getId(), session);
		if (mHandler == null) {
			return;
		}
		mHandler.afterConnectionEstablished(session);
	}

	@SuppressWarnings("unchecked")
	@Override
	public void handleMessage(WebSocketSession session, WebSocketMessage<?> message) throws Exception {
		if (mHandler == null) {
			return;
		}
		mHandler.handleMessage(session, message);
	}

	@Override
	public void handleTransportError(WebSocketSession session, Throwable exception) throws Exception {
		if (mHandler == null) {
			return;
		}
		mHandler.handleTransportError(session, exception);
	}

	@Override
	public void afterConnectionClosed(WebSocketSession session, CloseStatus closeStatus) throws Exception {
		mSessionList.remove(session.getId());
		if (mHandler == null) {
			return;
		}
		mHandler.afterConnectionClosed(session, closeStatus);
		session.close();
	}

	@Override
	public boolean supportsPartialMessages() {
		if (mHandler == null) {
			return false;
		}
		return mHandler.supportsPartialMessages();
	}
}