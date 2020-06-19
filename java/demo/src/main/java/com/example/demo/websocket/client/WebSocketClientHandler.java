package com.example.demo.websocket.client;

public interface WebSocketClientHandler {
	public void onOpen(WebSocketClientController controller);
	public void onMessage(WebSocketClientController controller, String message);
	public void onError(WebSocketClientController controller, Throwable th);
	public void onClose(WebSocketClientController controller);
}
