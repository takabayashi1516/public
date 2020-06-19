package com.example.demo.tls.client;

public interface TlsClientEventHandler {
	public void onTlsClientReceive(byte[] data, long timestamp);
}
