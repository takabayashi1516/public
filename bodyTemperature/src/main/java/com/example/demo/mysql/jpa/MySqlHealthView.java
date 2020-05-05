package com.example.demo.mysql.jpa;

import java.util.List;

public interface MySqlHealthView {
	public List<HealthViewEntity> getAll();
	public List<HealthViewEntity> get(long person);
	public List<HealthViewEntity> get(String name);
}
