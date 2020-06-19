package com.example.demo.mysql.jpa;

import java.util.List;

public interface MySqlHealth {
	public boolean append(long pid, long ts, float temperature);
	public List<HealthDataEntity> getRange(long person, long ts_start, long ts_end);
	public HealthDataEntity get(long id);
	public void update(HealthDataEntity e);
	public List<HealthDataEntity> getLatest(long person, int num);
	public HealthDataEntity getLatest(long person);
}
