package com.example.demo.tcp.server;

import org.springframework.messaging.Message;

public class TcpDispatcher {
	TcpController mController;
	public TcpDispatcher(TcpController controller) {
		mController = controller;
	}
	public void onReceive(Message<byte[]> m) {
		mController.onReceive(m);
	}
}
