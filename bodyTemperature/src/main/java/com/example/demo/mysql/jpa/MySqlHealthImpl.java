package com.example.demo.mysql.jpa;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Service;

@Service
public class MySqlHealthImpl implements MySqlHealth {
	@Autowired
	private MySqlHealthRepository mRepository;

	public MySqlHealthImpl() {
	}

	@Autowired
	public void context(ApplicationContext context) {
	}

	@Override
	public boolean append(long pid, long ts, float temperature) {
		HealthDataEntity d = new HealthDataEntity();
		d.setPersonalId(pid);
		d.setTimeStamp(ts);
		d.setTemperature(temperature);
		return (mRepository.save(d) != null);
	}
}
