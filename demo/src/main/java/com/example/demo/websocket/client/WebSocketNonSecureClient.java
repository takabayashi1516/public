package com.example.demo.websocket.client;

import java.io.IOException;
import java.net.URI;
import java.nio.ByteBuffer;

import javax.websocket.ClientEndpoint;
import javax.websocket.ContainerProvider;
import javax.websocket.OnClose;
import javax.websocket.OnError;
import javax.websocket.OnMessage;
import javax.websocket.OnOpen;
import javax.websocket.Session;
import javax.websocket.WebSocketContainer;

@ClientEndpoint
public class WebSocketNonSecureClient {
	protected WebSocketClientController mController;
	protected Session mSession = null;

	protected String mKeyStorePath;
	protected String mKeyStorePasswd;
	protected String mCaPath;

	public WebSocketNonSecureClient(WebSocketClientController controller) {
		super();
		mController = controller;
	}

	public void setSecureSettingPath(String keyStorePath, String keyStorePasswd, String caPath) {
		mKeyStorePath = keyStorePath;
		mKeyStorePasswd = keyStorePasswd;
		mCaPath = caPath;
	}

	public boolean connect(String ipAddr, int port, String path) throws Exception {
		if (mSession != null) {
			return false;
		}
		String url = "ws://" + ipAddr + ":" + String.valueOf(port) + path; 
		WebSocketContainer container = ContainerProvider.getWebSocketContainer();
		container.setDefaultMaxTextMessageBufferSize(1024 * 1024);
		URI uri = URI.create(url);
		try {
			mSession = container.connectToServer(this, uri);
		} catch (Exception e) {
			throw e;
		}
		return true;
	}

	public boolean send(String data) throws IOException {
		if (mSession == null) {
			return false;
		}
		mSession.getBasicRemote().sendText(data);
		return true;
	}

	public boolean send(byte[] data) throws IOException {
		if (mSession == null) {
			return false;
		}
		ByteBuffer b = ByteBuffer.wrap(data); 
		mSession.getBasicRemote().sendBinary(b);
		return true;
	}

	public void close() throws Exception {
		if (mSession == null) {
			return;
		}
		mSession.close();
	}

	@OnOpen
	public void onOpen(Session session) {
		mController.getHandler().onOpen(mController);
	}

	@OnMessage
	public void onMessage(String message) {
		mController.getHandler().onMessage(mController, message);
	}

	@OnError
	public void onError(Throwable th) {
		mController.getHandler().onError(mController, th);
	}

	@OnClose
	public void onClose(Session session) {
		mController.getHandler().onClose(mController);
		try {
			mSession.close();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			mSession = null;
		}
		
	}
}
