package com.example.demo.mysql.jpa;

public interface MySql {
	public boolean createTable(String db, String table);
	public void append(byte[] data);
	public void append(String data);
	public MySqlRepository getRepository();
}
