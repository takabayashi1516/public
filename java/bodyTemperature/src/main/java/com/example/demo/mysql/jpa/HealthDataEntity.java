package com.example.demo.mysql.jpa;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;

import org.springframework.lang.NonNull;

@Entity
@Table(name = "health")
public class HealthDataEntity {
	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@Column(name = "id")
	@NonNull
	private long mId;

	@Column(name = "personal_id")
	private long mPersonalId;

	@Column(name = "timestamp")
	private long mTimeStamp;

	@Column(name = "temperature")
	private float mTemperature;

	public long getId() {
		return mId;
	}

	public long getPersonalId() {
		return mPersonalId;
	}

	public long getTimeStamp() {
		return mTimeStamp;
	}

	public float getTemperature() {
		return mTemperature;
	}

	public void setPersonalId(long id) {
		mPersonalId = id;
	}

	public void setTimeStamp(long timestamp) {
		mTimeStamp = timestamp;
	}

	public void setTemperature(float temperature) {
		mTemperature = temperature;
	}
}
