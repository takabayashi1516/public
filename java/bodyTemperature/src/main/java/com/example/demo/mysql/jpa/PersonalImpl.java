package com.example.demo.mysql.jpa;

import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.stereotype.Service;

@Service
public class PersonalImpl implements Personal {
	@Autowired
	private PersonalRepository mRepository;

	public PersonalImpl() {
	}

	@Autowired
	public void context(ApplicationContext context) {
	}

	@Override
	public long update(String name, String mail, boolean valid) {
		PersonalDataEntity d = null;
		List<PersonalDataEntity> list = mRepository.findAll(Specification.where(
				PersonalSpec.fieldsEquals("mMail", mail)));
		if (!list.isEmpty()) {
			// if e-mail duplicate
			d = list.get(0);
			if (d.getName().equals(name)) {
//				return -1;
			}
			d.setName(name);
			d.setValid(valid);
			mRepository.save(d);
			d = get(mail);
			return d.getId();
		}

		list = mRepository.findAll(Specification.where(
				PersonalSpec.fieldsEquals("mName", name).and(
						PersonalSpec.fieldsEquals("mMail", mail))));
		if (!list.isEmpty()) {
			// impossible case
			return -1;
		}
		d = new PersonalDataEntity();
		d.setName(name);
		d.setMail(mail);
		d.setValid(valid);
		mRepository.save(d);
		d = get(mail);
		return d.getId();
	}

	@Override
	public boolean delete(long id) {
		List<PersonalDataEntity> list = mRepository.findAll(Specification.where(
				PersonalSpec.fieldsEquals("mId", id)));
		if (list.isEmpty()) {
			return false;
		}
		PersonalDataEntity d = list.get(0);
		if (d == null) {
			return false;
		}
		mRepository.delete(d);
		return (true);
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
		List<PersonalDataEntity> list = mRepository.findAll(Specification.where(
				PersonalSpec.fieldsEquals("mId", id)));
		if (list.isEmpty()) {
			return null;
		}
		return list.get(0);
	}

	@Override
	public PersonalDataEntity get(String mail) {
		List<PersonalDataEntity> list = mRepository.findAll(Specification.where(
				PersonalSpec.fieldsEquals("mMail", mail)));
		if (list.isEmpty()) {
			return null;
		}
		return list.get(0);
	}

	@Override
	public PersonalRepository getRepository() {
		return mRepository;
	}
}
