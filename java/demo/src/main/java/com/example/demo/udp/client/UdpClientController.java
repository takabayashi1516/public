package com.example.demo.udp.client;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.Profile;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Controller;

@Controller
@Profile("client")
public class UdpClientController {
	@Value("${udp_remote_address}")
	private String mUdpAddress;
	@Value("${udp_port}")
	private int mUdpPort;

	private UdpClientListener mListener = null;
	private DatagramSocket mSocket;
	private SocketAddress mInetAddr;

	public UdpClientController() {
		try {
			mSocket = new DatagramSocket();
		} catch (SocketException e) {
			e.printStackTrace();
		}
	}

	public void sendTo(byte[] data) throws IOException {
		DatagramPacket p = new DatagramPacket(data, data.length, mInetAddr);
		mSocket.send(p);
	}

	@Autowired
	public void context(ApplicationContext context) {
		mListener = context.getBean(UdpClientListener.class);
		mInetAddr = new InetSocketAddress(mUdpAddress, mUdpPort);
	}

	public void onReceive(Message message) {
		mListener.onUdpClientReceive(message);
	}
}
