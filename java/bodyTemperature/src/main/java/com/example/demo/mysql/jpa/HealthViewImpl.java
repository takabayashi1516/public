package com.example.demo.mysql.jpa;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Service;

@Service
public class HealthViewImpl implements HealthView {
	@Autowired
	private HealthViewRepository mRepository;

	public HealthViewImpl() {
	}

	@Autowired
	public void context(ApplicationContext context) {
	}

	@Override
	public HealthViewRepository getRepository() {
		return mRepository;
	}
}
