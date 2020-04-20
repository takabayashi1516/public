package com.example.demo.mysql.jpa;

import java.util.ArrayList;

public interface MySqlPersonal {
	public boolean update(String name, String mail);
	public boolean delete(long id, String name);
	public ArrayList<PersonalDataEntity> getAll();
	public boolean isContain(long id);
}
