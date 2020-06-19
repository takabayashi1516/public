package com.example.demo.mysql.jpa;

import java.util.List;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.Persistence;
import javax.persistence.Query;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Service;

import com.example.demo.mysql.jpa.DataEntity.TimeStampType;

@Profile("server")
@Service
public class MySqlImpl implements MySql {
	@Autowired
	private MySqlRepository mRepository;

	public MySqlImpl() {
	}

	@Autowired
	public void context(ApplicationContext context) {
	}

	@Override
	public boolean createTable(String db, String table) {
		return false;
	}

	@Override
	public void append(byte[] data) {
		
	}

	@Override
	public void append(String data) {
		DataEntity d = new DataEntity();
		d.setContents(data);
		d.setCurrentTime(TimeStampType.DbLoad);
		mRepository.save(d);
	}

	@Override
	public MySqlRepository getRepository() {
		return mRepository;
	}

}
