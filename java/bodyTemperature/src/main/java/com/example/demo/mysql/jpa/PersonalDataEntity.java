package com.example.demo.mysql.jpa;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;

import org.springframework.lang.NonNull;

@Entity
@Table(name = "personal")
public class PersonalDataEntity {
	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@Column(name = "id")
	@NonNull
	private long mId;

	@Column(name = "name")
	private String mName;

	@Column(name = "mail")
	private String mMail;

	@Column(name = "valid")
	private Boolean mValid;

	public long getId() {
		return mId;
	}

	public String getName() {
		return mName;
	}

	public String getMail() {
		return mMail;
	}

	public boolean getValid() {
		return (mValid == null)? true : mValid;
	}

	public void setName(String name) {
		mName = name;
	}

	public void setMail(String mail) {
		mMail = mail;
	}

	public void setValid(boolean valid) {
		mValid = valid;
	}
}
