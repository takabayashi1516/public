package com.example.demo.mysql.jpa;

import java.util.Date;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;

import org.springframework.context.annotation.Profile;
import org.springframework.lang.NonNull;

@Profile("server")
@Entity
@Table(name = "data_table")
public class DataEntity {
	enum TimeStampType {
		Sender,
		DbSave,
		DbLoad
	};

	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@Column(name = "id")
	@NonNull
	private long mId;

	@Column(name = "contents")
	private String mContents;

	@Column(name = "sender_timestamp")
	private double mSenderTimeStamp;

	@Column(name = "dbsave_timestamp")
	private double mDbSaveTimeStamp;

	@Column(name = "dbload_timestamp")
	private double mLoadTimeStamp;

	public long getId() {
		return mId;
	}

	public String getContents() {
		return mContents;
	}

	public double getTimeStamp(TimeStampType type) {
		double rc = -1.0;
		switch (type) {
		case Sender:
			rc = mSenderTimeStamp;
			break;
		case DbSave:
			rc = mDbSaveTimeStamp;
			break;
		case DbLoad:
			rc = mLoadTimeStamp;
			break;
		default:
			break;
		}
		return rc;
	}

	public void setContents(String contents) {
		mContents = contents;
	}

	public void setTimeStamp(TimeStampType type, double time) {
		switch (type) {
		case Sender:
			mSenderTimeStamp = time;
			break;
		case DbSave:
			mDbSaveTimeStamp = time;
			break;
		case DbLoad:
			mLoadTimeStamp = time;
			break;
		default:
			break;
		}
	}

	public void setCurrentTime(TimeStampType type) {
		setTimeStamp(type, (double) ((new Date()).getTime()) / 1000);
	}
}
