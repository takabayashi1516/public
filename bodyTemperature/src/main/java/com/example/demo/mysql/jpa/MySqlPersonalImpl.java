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
public class MySqlPersonalImpl implements MySqlPersonal {
	@Autowired
	private MySqlPersonalRepository mRepository;

	public MySqlPersonalImpl() {
	}

	@Autowired
	public void context(ApplicationContext context) {
	}

	@Override
	public boolean update(String name, String mail) {
		PersonalDataEntity d = mRepository.get(name);
		if (d == null) {
			d = new PersonalDataEntity();
		}
		d.setName(name);
		d.setMail(mail);
		return (mRepository.save(d) != null);
	}

	@Override
	public boolean delete(long id, String name) {
		PersonalDataEntity d = mRepository.get(name);
		if (d == null) {
			return false;
		}
		if (id != d.getId()) {
			return false;
		}
		mRepository.delete(d);
		return (true);
	}

	@Override
	public ArrayList<PersonalDataEntity> getAll() {
		return mRepository.findAll();
	}

	@Override
	public boolean isContain(long id) {
		ArrayList<PersonalDataEntity> list =mRepository.findAll();
		for (int i = 0; i < list.size(); i++) {
			if (list.get(i).getId() == id) {
				return true;
			}
		}
		return false;
	}
}
