package com.example.demo.websocket.client;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.nio.ByteBuffer;

import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.KeyStore;
import javax.net.ssl.SSLContext;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.TrustManagerFactory;

import javax.websocket.ClientEndpoint;
import javax.websocket.OnClose;
import javax.websocket.OnError;
import javax.websocket.OnMessage;
import javax.websocket.OnOpen;
import javax.websocket.Session;

import org.eclipse.jetty.util.ssl.SslContextFactory;
import org.eclipse.jetty.client.HttpClient;
import org.eclipse.jetty.websocket.api.WebSocketConnectionListener;
import org.eclipse.jetty.websocket.api.WebSocketListener;
import org.eclipse.jetty.websocket.client.ClientUpgradeRequest;
import org.eclipse.jetty.websocket.client.WebSocketClient;

@ClientEndpoint
public class WebSocketSecureClient extends WebSocketNonSecureClient
		implements WebSocketConnectionListener, WebSocketListener {
	private WebSocketClient mSocket = null;
	private org.eclipse.jetty.websocket.api.Session mWsSession;

	public WebSocketSecureClient(WebSocketClientController controller) {
		super(controller);
	}

	@Override
	public boolean connect(String ipAddr, int port, String path) throws Exception {
		String url = "wss://" + ipAddr + ":" + String.valueOf(port) + path; 
		URI uri = URI.create(url);

		if (mSocket == null) {
			SslContextFactory sslContextFactory = new SslContextFactory.Client(true);
			SSLContext context;
			context = createSslContext(mKeyStorePath, mKeyStorePasswd, mCaPath);
			sslContextFactory.setSslContext(context);
			HttpClient http = new HttpClient(sslContextFactory);
			mSocket = new WebSocketClient(http);
			mSocket.start();
//			mSocket.addSessionListener(this);
		}

		if (mWsSession == null) {
			mSocket.connect(this, uri,  new ClientUpgradeRequest());
			return true;
		}
		return false;
	}

	@Override
	public boolean send(String data) throws IOException {
		if (mWsSession == null) {
			return false;
		}
		mWsSession.getRemote().sendString(data);
		return true;
	}

	@Override
	public boolean send(byte[] data) throws IOException {
		if (mWsSession == null) {
			return false;
		}
		ByteBuffer b = ByteBuffer.wrap(data);
		mWsSession.getRemote().sendBytes(b);
		return true;
	}

	@Override
	public void close() throws Exception {
		if (mWsSession == null) {
			return;
		}
		mWsSession.close();
		mWsSession = null;
	}

	@OnOpen
	public void onOpen(Session session) {
		mController.getHandler().onOpen(mController);
	}

	@OnMessage
	public void onMessage(String message) {
		mController.getHandler().onMessage(mController, message);
	}

	@OnMessage
	public void onMessage(byte[] message) {
		mController.getHandler().onMessage(mController, new String(message));
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

	private SSLContext createSslContext(String strKeyPath, String strKeyPasswd,
			String strCa) throws Exception{
		Certificate ca = loadCertificate(strCa);
		String keyStoreType = KeyStore.getDefaultType();

		KeyStore keyStore = null;
		KeyManagerFactory kmf = null;
		if ((strKeyPath != null) && (!strKeyPath.isEmpty())) {
			char[] keyPasswd = strKeyPasswd.toCharArray();
			keyStore = KeyStore.getInstance(keyStoreType);
			String kmfAlgorithm = KeyManagerFactory.getDefaultAlgorithm();
			kmf = KeyManagerFactory.getInstance(kmfAlgorithm);
			keyStore.load(new FileInputStream(strKeyPath), keyPasswd);
			kmf.init(keyStore, keyPasswd);
		}

		KeyStore trustStore = KeyStore.getInstance(keyStoreType);
		trustStore.load(null, null);
		trustStore.setCertificateEntry("ca", ca);
		String tmfAlgorithm = TrustManagerFactory.getDefaultAlgorithm();
		TrustManagerFactory tmf = TrustManagerFactory.getInstance(tmfAlgorithm);
		tmf.init(trustStore);

		SSLContext sslcontext = SSLContext.getInstance("TLS");
		if (kmf != null) {
			sslcontext.init(kmf.getKeyManagers(), tmf.getTrustManagers(), null);
		} else {
			sslcontext.init(null, tmf.getTrustManagers(), null);
		}

		return sslcontext;
	}

	private Certificate loadCertificate(String strCert) throws
			CertificateException, IOException {
		CertificateFactory cf = CertificateFactory.getInstance("X.509");
		InputStream caInput = new BufferedInputStream(new FileInputStream(strCert));
		Certificate ca = null;
		try {
			ca = cf.generateCertificate(caInput);
		} catch (Exception e) {
			System.out.println(e);
		} finally {
			caInput.close();
		}
		return ca;
	}

	@Override
	public void onWebSocketClose(int statusCode, String reason) {
		mController.getHandler().onClose(mController);
		mWsSession.close();
		mWsSession = null;
	}

	@Override
	public void onWebSocketConnect(org.eclipse.jetty.websocket.api.Session session) {
		mWsSession = session;
		mController.getHandler().onOpen(mController);
	}

	@Override
	public void onWebSocketError(Throwable cause) {
		mController.getHandler().onError(mController, cause);
		mWsSession.close();
		mWsSession = null;
	}

	@Override
	public void onWebSocketBinary(byte[] payload, int offset, int len) {
		mController.getHandler().onMessage(mController, new String(payload));
	}

	@Override
	public void onWebSocketText(String message) {
		mController.getHandler().onMessage(mController, message);
	}
}
