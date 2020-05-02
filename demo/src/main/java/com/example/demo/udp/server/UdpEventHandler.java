package com.example.demo.udp.server;

public interface UdpEventHandler {
	public void onReceive(String ip_address, int port, byte[] data, long timestamp);
}
