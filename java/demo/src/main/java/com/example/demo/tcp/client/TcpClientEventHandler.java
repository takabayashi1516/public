package com.example.demo.tcp.client;

public interface TcpClientEventHandler {
	public void onTcpClientReceive(byte[] data, long timestamp);
}
