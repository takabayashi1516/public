package com.example.demo.tcp.client;

import org.springframework.messaging.Message;

public class TcpClientDispatcher {
	TcpClientController mController;
	public TcpClientDispatcher(TcpClientController controller) {
		mController = controller;
	}
	public void onReceive(Message<byte[]> m) {
		mController.onReceive(m);
	}
}
