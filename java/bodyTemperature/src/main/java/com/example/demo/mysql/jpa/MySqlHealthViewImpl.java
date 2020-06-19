package com.example.demo.mysql.jpa;

import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Service;

@Service
public class MySqlHealthViewImpl implements MySqlHealthView {
	@Autowired
	private MySqlHealthViewRepository mRepository;

	public MySqlHealthViewImpl() {
	}

	@Autowired
	public void context(ApplicationContext context) {
	}

	@Override
	public List<HealthViewEntity> getAll() {
		return mRepository.findAll();
	}

	@Override
	public List<HealthViewEntity> get(long person) {
		return mRepository.get(person);
	}

	@Override
	public List<HealthViewEntity> get(String name) {
		return mRepository.get(name);
	}
}
