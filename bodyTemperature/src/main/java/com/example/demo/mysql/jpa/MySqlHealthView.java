package com.example.demo.mysql.jpa;

import java.util.ArrayList;

public interface MySqlHealthView {
	public ArrayList<HealthViewEntity> getAll();
	public ArrayList<HealthViewEntity> get(long person);
	public ArrayList<HealthViewEntity> get(String name);
}
