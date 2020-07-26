/**
 * java -Dhost_front=xx.xx.xx.xx -jar bodyTemperature-0.0.1-SNAPSHOT.jar
 */
package com.example.demo;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ApplicationContext;
import org.springframework.mail.MailException;
import org.springframework.mail.MailSender;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;

import com.example.demo.mysql.jdbc.SqlController;
import com.example.demo.mysql.jpa.Health;
import com.example.demo.mysql.jpa.HealthDataEntity;
import com.example.demo.mysql.jpa.Personal;
import com.example.demo.thymeleaf.ThymeleafController;

@SpringBootApplication
@EnableScheduling
public class BodyTemperatureApplication {

	@Autowired
	private SqlController mSqlController;
	@Autowired
	private Personal mPersonal;
	@Autowired
	private MailSender mMailSender;
	@Autowired
	private Health mHealth;

	@Autowired
	private ThymeleafController mThymeleafController;

	@Value("${spring.mail.username}")
	private String mMailUser;
	@Value("${health_view}")
	private String mHealthView;

	@Value("${boot_broadcast}")
	private boolean mIsBootBroadcast;

//	@Autowired
//	Environment mEnvironment;

	@Value("${host_front}")
	private String mHost;
	@Value("${server.port}")
	private String mPort;

	public static void main(String[] args) {
		SpringApplication.run(BodyTemperatureApplication.class, args);
	}

	@Autowired
	public void context(ApplicationContext context) {
//		mPort = mEnvironment.getProperty("local.server.port");
		int rc = 0;
		try {
			String sql = "create view ";
//			sql += "if not exist ";
			sql += mHealthView + " ";
			sql += "(id, person, name, mail, timestamp, temperature) ";
			sql += "as select H.id, P.id, P.name, P.mail, H.timestamp, H.temperature ";
			sql += "from health as H left join personal as P on H.personal_id = P.id;";
			rc = mSqlController.update(sql);
		} catch (Exception e) {
//			e.printStackTrace();
		}
		System.out.println("update rc=" + rc);

		System.out.println("admin: " + mThymeleafController.getAdministratorHash());
		broadcastMail(mIsBootBroadcast);
	}

	@Scheduled(cron = "0 0 8 * * *", zone = "Asia/Tokyo")
	public void doBroadcastRequest() {
		broadcastMail(true);
	}

	private void broadcastMail(boolean action) {
		mPersonal.getRepository().findAll().forEach(e -> {
			System.out.println("[" + String.valueOf(e.getId()) + "]"
					+ e.getName() + ": " + e.getMail());
			String hash = mThymeleafController.getHash(e.getMail());
			while (hash.substring(hash.length() - 1).equals(".")) {
				hash = mThymeleafController.getHash(e.getMail());
			}
			HealthDataEntity he = mHealth.getLatest(e.getId());
			String msg = "";
			String title = "";
			if (he.getTimeStamp() < ((new Date().getTime()) - (60 * 60 * 24 * 1000))) {
				SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd");
				msg = "[alert] recently data: " + sdf.format(he.getTimeStamp()) + "\n\n";
				title = "[alert] recently->" + sdf.format(he.getTimeStamp()) + " ";
			}
			title += "request measure body temperature!";
			msg += "http://" + mHost + ":" + mPort + "/personal?hash=" + hash + "\n\n";
			msg += "confirm: \n";
			msg += "http://" + mHost + ":" + mPort + "/data?hash=" + hash + "\n";
			System.out.println(title);
			System.out.println(msg);
			if (action) {
				sendMail(e.getMail(), title, msg);
			}
		});
	}

	private void sendMail(String to, String subject, String body) {
		SimpleMailMessage msg = (new SimpleMailMessage());
		msg.setFrom(mMailUser);
		msg.setTo(to);
		msg.setSubject(subject);
		msg.setText(body);
		try {
			mMailSender.send(msg);
		} catch (MailException e) {
			//
		}
	}
}
