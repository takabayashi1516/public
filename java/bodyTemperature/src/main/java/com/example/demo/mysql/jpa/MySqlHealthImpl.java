package com.example.demo.mysql.jpa;

import java.util.List;

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

	@Override
	public List<HealthDataEntity> getRange(long person, long ts_start, long ts_end) {
		return mRepository.get(person, ts_start, ts_end);
	}

	@Override
	public void update(HealthDataEntity e) {
		mRepository.save(e);
	}

	@Override
	public HealthDataEntity get(long id) {
		return mRepository.get(id);
	}

	@Override
	public List<HealthDataEntity> getLatest(long person, int num) {
		return mRepository.getLatest(person, num);
	}

	@Override
	public HealthDataEntity getLatest(long person) {
		List<HealthDataEntity> list = mRepository.getLatest(person, 1);
		if (list.isEmpty()) {
			return null;
		}
		return list.get(0);
	}
}
