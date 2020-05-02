package com.example.demo.udp.server;

import org.springframework.messaging.Message;

public class UdpDispatcher {
	UdpController mController;
	public UdpDispatcher(UdpController controller) {
		mController = controller;
	}
	public void onReceive(Message<byte[]> m) {
		mController.onReceive(m);
	}
}
