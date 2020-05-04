package com.example.demo;

import java.io.IOException;
import java.nio.charset.Charset;
import java.util.Date;
import java.util.List;
import java.util.Map;

import javax.script.ScriptException;
import javax.websocket.DeploymentException;

import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttPersistenceException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.Profile;
import org.springframework.messaging.Message;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.web.socket.CloseStatus;
import org.springframework.web.socket.WebSocketMessage;
import org.springframework.web.socket.WebSocketSession;

import com.example.demo.js.JavaScript;
import com.example.demo.json.Json;
import com.example.demo.resource.ResourceController;
import com.example.demo.tcp.server.TcpController;
import com.example.demo.tcp.server.TcpEventHandler;
import com.example.demo.thymeleaf.ThymeleafController;
import com.example.demo.mqtt.config.Mqtt;
import com.example.demo.mqtt.handler.MqttSubScribe;
import com.example.demo.mqtt.model.MqttSubscribeModel;
import com.example.demo.mysql.jpa.DataEntity;
import com.example.demo.mysql.jpa.MySql;

import com.example.demo.mysql.jdbc.SqlController;

//import com.example.demo.pubsub.RedisMessagePublisher;

import com.example.demo.udp.client.UdpClientController;
import com.example.demo.udp.client.UdpClientListener;
import com.example.demo.udp.server.UdpEventHandler;

import com.example.demo.websocket.client.WebSocketClientController;
import com.example.demo.websocket.client.WebSocketClientHandler;
import com.example.demo.websocket.server.WebSocketEventHandler;

@SpringBootApplication
@EnableScheduling
public class DemoApplication implements UdpEventHandler, UdpClientListener,
		WebSocketEventHandler, WebSocketClientHandler, MqttSubScribe, TcpEventHandler {
	private TcpController mTcpController = null;
	private WebSocketClientController mWsController = null;
	private UdpClientController mUdpController = null;
	private Mqtt mMqtt = null;
	private boolean mWsStatus = false;
	private ResourceController mResourceController;
	private ThymeleafController mThymeleafController;
	private SqlController mSqlController;

	private static final String MQTT_TOPIC_UDP_RX = "udp_rx";
	private static final String MQTT_TOPIC_TCP_RX = "tcp_rx";

//	@Autowired
//	RedisMessagePublisher publisher;

	private MySql mMySql;

	public DemoApplication() {
	}

	public static void main(String[] args) {
		SpringApplication.run(DemoApplication.class, args);
		try {
			JavaScript js = new JavaScript("js/test.js");
			String s = (String) js.execute();
			System.out.print(s);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (ScriptException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

	@Override
	public void onUdpReceive(String ip_address, int port, byte[] data, long timestamp) {
		String s = "[" + String.valueOf(timestamp) + "]"
				+ ip_address + ":" + String.valueOf(port) + "/" + new String(data);
		System.out.print(s + "\n");
		try {
			Mqtt.getSingleton().publishMessage(MQTT_TOPIC_UDP_RX, data, 0, false);
		} catch (MqttPersistenceException e) {
			e.printStackTrace();
		} catch (MqttException e) {
			e.printStackTrace();
		}
	}

	@SuppressWarnings("rawtypes")
	@Override
	public void onUdpReceive(Message message) {
		/*
		message.getHeaders()
		{
			ip_packetAddress=127.0.0.1/127.0.0.1:63617,
			ip_address=127.0.0.1,
			id=03940627-7a8a-776f-ac24-7fd39f4ea19d,
			ip_port=63617,
			ip_hostname=127.0.0.1,
			timestamp=1588293050172
		}
*/
		String id = message.getHeaders().getId().toString();
		System.out.println(message.getHeaders());
//		System.out.println(message.getHeaders().get(IpHeaders.PACKET_ADDRESS));
		Object payload = message.getPayload();
		System.out.println(new String((byte[]) payload, Charset.defaultCharset()));
	}

	@Override
	public void afterConnectionEstablished(WebSocketSession session) {
		// TODO Auto-generated method stub
		try {
			Mqtt.getSingleton().subscribeChannel(MQTT_TOPIC_UDP_RX, 10);
		} catch (MqttException e) {
			// TODO Auto-generated catch block
//			e.printStackTrace();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
//			e.printStackTrace();
		}
	}

	@Autowired
	public void context(ApplicationContext context) {
		try {
			mTcpController = context.getBean(TcpController.class);
		} catch (Exception e) {
		}
		try {
			mUdpController = context.getBean(UdpClientController.class);
		} catch (Exception e) {
		}
		try {
			mWsController = context.getBean(WebSocketClientController.class);
		} catch (Exception e) {
		}
		try {
			mMqtt = context.getBean(Mqtt.class);
			Mqtt.getSingleton().setSubscribeHandler(this);
		} catch (Exception e) {
		}
		try {
			mMySql = context.getBean(MySql.class);
			mMySql.append("test data");
/*			List<DataEntity> list = mMySql.getRepository()
					.getAll("data_table");
/*					.doQuery("select * from data_table"); */
//			System.out.println(list);
		} catch (Exception e) {
//			e.printStackTrace();
		}

		try {
			mThymeleafController = context.getBean(ThymeleafController.class);
		} catch (Exception e) {
//			e.printStackTrace();
		}

		mResourceController = context.getBean(ResourceController.class);
		try {
			Json json1 = new Json();
			Json json2 = new Json();

			byte[] data = "[]".getBytes();
			json1.load(data);
			json1.set(null, 0, 10);
			json1.set(null, 1, 10.01);
			System.out.println(json1.get("").toString());

			data = "{}".getBytes();
			json2.load("{\"util2\": \"jsonutil2\"}");
			json1.load(data);
			json1.set("test_root1", json2.get(null));
			json1.set("test_root2", "--- test ---");
			System.out.println(json1.get("").toString());
			json1.set(null, json2.get(null));
			System.out.println(json1.get("").toString());

			data = mResourceController.readBytes("./test-data/data.json");
			json2.load(data);
			json1.load(data);
			json1.set("nest", json2.get(null));
			json1.del("nest.root.sub3");
			System.out.println(json1.get(null).toString());
			System.out.println(json1.get("root.sub1"));
			System.out.println(json1.get("root.sub2", 0));
			System.out.println(json1.set("root.sub2", 0, 10000));
			System.out.println(json1.get("root.sub2", 0));
			System.out.println(json1.get("root.sub2", 1));
			System.out.println(json1.get("root.sub2", 2));
			System.out.println(json1.get(json1.get("root.sub3", 0), "sub3_1"));
			System.out.println(json1.get(json1.get("root.sub3", 1), "sub3_2"));
			mResourceController.writeBytes("./test-data/data_2.txt", json1.get("").toString().getBytes());
			if (mThymeleafController != null) {
				mThymeleafController.setData(json1.get("").toString().getBytes().clone());
			}
		} catch (IOException e) {
//			e.printStackTrace();
		}

		try {
			mSqlController = context.getBean(SqlController.class);
			int rc = 0;
			rc = mSqlController.createTable("table1", "(id int not null auto_increment primary key, name text  not null, mail text, upd datetime, age int) default charset=utf8");
			System.out.printf("createTable rc=%d\n", rc);
			rc = mSqlController.insertRecord("table1", "(name, mail, age, upd)", "(\"aaaa\", \"aaaa@gmail.com\", 20, now())");
			System.out.printf("insertRecord rc=%d\n", rc);
			rc = mSqlController.insertRecord("table1", "(name, mail, upd)", "(\"bbbb\", \"bbbb@gmail.com\", now())");
			System.out.printf("insertRecord rc=%d\n", rc);
			rc = mSqlController.insertRecord("table1", "(name, mail, age)", "(\"cccc\", \"cccc@gmail.com\", 10)");
			System.out.printf("insertRecord rc=%d\n", rc);
			rc = mSqlController.insertRecord("table1", "(name, mail, age)", "(\"dddd\", \"dddd@gmail.com\", 30)");
			System.out.printf("insertRecord rc=%d\n", rc);
			List<Map<String, Object>> list = mSqlController.select("table1", null);
			rc = mSqlController.updateRecord("table1", "name=\"Shigeru.Takabayashi\", upd=now()", "age = 35");
			System.out.printf("updateRecord rc=%d\n", rc);
			list = mSqlController.select("table1", null);
			rc = mSqlController.deleteRecord("table1", "age = 35");
			System.out.printf("deleteRecord rc=%d\n", rc);
			list = mSqlController.select("table1", null);
			rc = mSqlController.dropTable("table1");
			System.out.printf("dropTable rc=%d\n", rc);
		} catch (Exception e) {
//			e.printStackTrace();
		}
	}

	@Profile("client")
	@Scheduled(fixedRate = 1000)
	public void push() {
		try {
			if (mWsController != null) {
				if ((!mWsStatus)) {
					mWsController.connect("/");
				} else {
					mWsController.send("[ws]hello" + String.valueOf((new Date()).getTime()));
				}
			}
		} catch (Exception e) {
//			e.printStackTrace();
		}
		if (mUdpController != null) {
			try {
				System.out.println("udp send");
				mUdpController.sendTo(("[udp]hello " + String.valueOf((new Date()).getTime())).getBytes());
			} catch (IOException e) {
			}
		}
		if (mMqtt != null) {
		}

	}

	@Override
	public void handleMessage(WebSocketSession session, WebSocketMessage<?> message) {
		// TODO Auto-generated method stub
		System.out.println("DemoApplication::onMessage: " + message);
//		publisher.publish((String) message.getPayload());
	}

	@Override
	public void handleTransportError(WebSocketSession session, Throwable exception) {
		// TODO Auto-generated method stub
		System.out.println("DemoApplication::handleTransportError: " + exception);
	}

	@Override
	public void afterConnectionClosed(WebSocketSession session, CloseStatus closeStatus) {
		// TODO Auto-generated method stub
		System.out.println("DemoApplication::afterConnectionClosed: " + closeStatus);
	}

	@Override
	public boolean supportsPartialMessages() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void onOpen(WebSocketClientController controller) {
		// TODO Auto-generated method stub
		mWsStatus = true;
		System.out.println("DemoApplication::onOpen");
	}

	@Override
	public void onMessage(WebSocketClientController controller, String message) {
		// TODO Auto-generated method stub
		System.out.println("DemoApplication::onMessage: " + message);
	}

	@Override
	public void onError(WebSocketClientController controller, Throwable th) {
		// TODO Auto-generated method stub
		System.out.println("DemoApplication::onError: " + th.getMessage());
	}

	@Override
	public void onClose(WebSocketClientController controller) {
		// TODO Auto-generated method stub
		mWsStatus = false;
		System.out.println("DemoApplication::onClose");
	}

	@Override
	public void onSubScribe(List<MqttSubscribeModel> messages) {
		// TODO Auto-generated method stub
		System.out.println("DemoApplication::onSubScribe");
	}

	@Override
	public void onTcpReceive(String id, byte[] data, long timestamp) {
		String s = "[" + String.valueOf(timestamp) + "]"
				+ id + " / " + new String(data);
		System.out.print(s + "\n");
		try {
			mTcpController.send(id, data);
		} catch (Exception e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
	}
}
