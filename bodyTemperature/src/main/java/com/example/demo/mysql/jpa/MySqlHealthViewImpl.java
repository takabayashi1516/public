package com.example.demo.mysql.jpa;

import java.util.ArrayList;
import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.Persistence;
import javax.persistence.Query;

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
	public ArrayList<HealthViewEntity> getAll() {
		return mRepository.findAll();
	}

	@Override
	public ArrayList<HealthViewEntity> get(long person) {
		return mRepository.get(person);
	}

	@Override
	public ArrayList<HealthViewEntity> get(String name) {
		return mRepository.get(name);
	}
}
