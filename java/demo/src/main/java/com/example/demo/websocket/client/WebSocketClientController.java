package com.example.demo.websocket.client;

import java.io.IOException;
import javax.websocket.DeploymentException;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Controller;
import org.springframework.web.socket.config.annotation.EnableWebSocket;

@EnableWebSocket
@Controller
@Profile("client")
public class WebSocketClientController {
	private WebSocketNonSecureClient mWsClient;
	private WebSocketClientHandler mHandler;
	@Autowired
	ApplicationContext mContext;
	@Value("${ws_remote_address}")
	private String mWsAddress;
	@Value("${ws_remote_port}")
	private int mWsPort;
	@Value("${ws_trust_cert:null}")
	private String mTrustCa;

	@Value("${ws_secure}")
	private boolean mIsSecure;

	public WebSocketClientController() {
	}

	public WebSocketClientHandler getHandler() {
		return mHandler;
	}

	public boolean connect(String path) throws Exception {
		return mWsClient.connect(mWsAddress, mWsPort, path);
	}

	public void send(String data) throws IOException {
		mWsClient.send(data);
	}

	public void send(byte[] data) throws IOException {
		mWsClient.send(data);
	}

	public void close() throws Exception {
		mWsClient.close();
	}

	@Autowired
	public void context(ApplicationContext context) {
		mContext = context;
		mHandler = mContext.getBean(WebSocketClientHandler.class);

		if (mIsSecure) {
			mWsClient = new WebSocketSecureClient(this);
			mWsClient.setSecureSettingPath(null, null, mTrustCa);
		} else {
			mWsClient = new WebSocketNonSecureClient(this);
		}
	}
}
