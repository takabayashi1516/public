package com.example.demo.mysql.jpa;

import java.util.List;

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
	public long update(String name, String mail) {
		PersonalDataEntity d = mRepository.getByName(name);
		if (d == null) {
			d = new PersonalDataEntity();
			d.setName(name);
		}
		d.setMail(mail);
		mRepository.save(d);
		d = get(mail);
		return d.getId();
	}

	@Override
	public boolean delete(long id) {
		PersonalDataEntity d = mRepository.get(id);
		if (d == null) {
			return false;
		}
		mRepository.delete(d);
		return (true);
	}

	@Override
	public List<PersonalDataEntity> getAll() {
		return mRepository.findAll();
	}

	@Override
	public boolean isContain(long id) {
		List<PersonalDataEntity> list =mRepository.findAll();
		for (int i = 0; i < list.size(); i++) {
			if (list.get(i).getId() == id) {
				return true;
			}
		}
		return false;
	}

	@Override
	public PersonalDataEntity get(long id) {
		return mRepository.get(id);
	}

	@Override
	public PersonalDataEntity get(String mail) {
		return mRepository.get(mail);
	}
}
