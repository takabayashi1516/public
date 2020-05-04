package com.example.demo.udp.client;

import org.springframework.messaging.Message;

public interface UdpClientListener {
	public void onUdpClientReceive(Message message);
}
