/**
 * java -Dhost_front=xx.xx.xx.xx -jar bodyTemperature-0.0.1-SNAPSHOT.jar
 */
package com.example.demo;

import java.util.ArrayList;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ApplicationContext;
import org.springframework.core.env.Environment;
import org.springframework.mail.MailException;
import org.springframework.mail.MailSender;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;

import com.example.demo.mysql.jdbc.SqlController;
import com.example.demo.mysql.jpa.HealthViewEntity;
import com.example.demo.mysql.jpa.MySqlHealth;
import com.example.demo.mysql.jpa.MySqlHealthView;
import com.example.demo.mysql.jpa.MySqlPersonal;
import com.example.demo.mysql.jpa.PersonalDataEntity;
import com.example.demo.thymeleaf.ThymeleafController;

@SpringBootApplication
@EnableScheduling
public class BodyTemperatureApplication {
	private static final String PERSONAL_TABLE = "personal"; 

	@Autowired
	private SqlController mSqlController;
	@Autowired
	private MySqlHealth mMysqlHealth;
	@Autowired
	private MySqlPersonal mMysqlPersonal;
	@Autowired
	private MailSender mMailSender;
	@Autowired
	private MySqlHealthView mMysqlHealthView;
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
		if (mIsBootBroadcast) {
			broadcastMail();
		}
	}

	@Scheduled(cron = "0 30 8 * * *", zone = "Asia/Tokyo")
	public void doBroadcastRequest() {
		broadcastMail();
	}

	private void broadcastMail() {
		mMysqlPersonal.getAll().forEach(e -> {
			System.out.println("[" + String.valueOf(e.getId()) + "]"
					+ e.getName() + ": " + e.getMail());
			String hash = mThymeleafController.getHash(e.getMail());
			String msg = "http://" + mHost + ":" + mPort + "/personal?hash=" + hash + "\n";
			msg += "confirm: ";
			msg += "http://" + mHost + ":" + mPort + "/data?hash=" + hash + "\n";
			System.out.println(msg);
			sendMail(e.getMail(), "request measure body temperature!", msg);
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
