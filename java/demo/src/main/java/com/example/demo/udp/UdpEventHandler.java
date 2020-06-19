package com.example.demo.udp;

import org.springframework.messaging.Message;

public interface UdpEventHandler {
	public void onUdpReceive(Message<byte[]> message);
}
