package com.example.demo.mysql.jpa;

public interface MySqlHealth {
	public boolean append(long pid, long ts, float temperature);
}
