package com.example.demo.tcp.server;

public interface TcpEventHandler {
	public void onTcpReceive(String id, byte[] data, long timestamp);
}
