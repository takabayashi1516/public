package com.example.demo.mysql.jpa;

import java.util.Date;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;

import org.springframework.lang.NonNull;

@Entity
@Table(name = "health_view")
public class HealthViewEntity {
	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@Column(name = "id")
	@NonNull
	private long mId;

	@Column(name = "person")
	private long mPerson;

	@Column(name = "name")
	private String mName;

	@Column(name = "mail")
	private String mMail;

	@Column(name = "timestamp")
	private long mTimeStamp;

	@Column(name = "temperature")
	private float mTemperature;

	public long getId() {
		return mId;
	}

	public long getPerson() {
		return mPerson;
	}

	public String getName() {
		return mName;
	}

	public String getMail() {
		return mMail;
	}

	public long getTimeStamp() {
		return mTimeStamp;
	}

	public float getTemperature() {
		return mTemperature;
	}
}
