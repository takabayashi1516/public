package com.example.demo.websocket.server;

import org.springframework.web.socket.CloseStatus;
import org.springframework.web.socket.WebSocketMessage;
import org.springframework.web.socket.WebSocketSession;

public interface WebSocketEventHandler {
	public void afterConnectionEstablished(WebSocketSession session);
	public void handleMessage(WebSocketSession session, WebSocketMessage<?> message);
	public void handleTransportError(WebSocketSession session, Throwable exception);
	public void afterConnectionClosed(WebSocketSession session, CloseStatus closeStatus);
	public boolean supportsPartialMessages();
}