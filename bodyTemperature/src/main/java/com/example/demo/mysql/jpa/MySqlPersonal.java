package com.example.demo.mysql.jpa;

import java.util.List;

public interface MySqlPersonal {
	public long update(String name, String mail);
	public boolean delete(long id);
	public List<PersonalDataEntity> getAll();
	public boolean isContain(long id);
	public PersonalDataEntity get(long id);
	public PersonalDataEntity get(String mail);
}
