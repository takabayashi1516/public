package com.example.demo.tls.server;

public interface TlsEventHandler {
	public void onTlsReceive(String id, byte[] data, long timestamp);
}
