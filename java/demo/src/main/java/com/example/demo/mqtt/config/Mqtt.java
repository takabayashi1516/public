package com.example.demo.mqtt.config;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import org.eclipse.paho.client.mqttv3.IMqttClient;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttPersistenceException;
import org.eclipse.paho.client.mqttv3.MqttSecurityException;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;

import com.example.demo.mqtt.handler.MqttSubScribe;
import com.example.demo.mqtt.model.MqttSubscribeModel;

@Configuration
@Profile({"server", "client", "front"})
public class Mqtt implements ApplicationContextAware {

	@Value("${mqtt_publisher_id}")
	private String mMqttPublisherId;
	@Value("${mqtt_host}")
	private String mHostAddress;
	@Value("${mqtt_port}")
	private int mPort;

	private MqttSubScribe mSubscribe;

	private IMqttClient instance = null;
	private static Mqtt singleton = null;

	public Mqtt() {
		assert(singleton == null);
		singleton = this;
	}

	public static Mqtt getSingleton() {
		if (singleton == null) {
//			new Mqtt();
			return null;
		}
		return singleton;
	}
	public String getPublisherId() {
		return mMqttPublisherId;
	}
	public String getHostAddress() {
		return mHostAddress;
	}
	public int getPort() {
		return mPort;
	}
	public void setSubscribeHandler(MqttSubScribe subscribe) {
		mSubscribe = subscribe;
	}

	public IMqttClient getInstance() {
		MqttConnectOptions options = new MqttConnectOptions();
		options.setAutomaticReconnect(true);
		options.setCleanSession(true);
		options.setConnectionTimeout(10);

		if (!instance.isConnected()) {
			try {
				instance.connect(options);
			} catch (MqttSecurityException e) {
//				e.printStackTrace();
			} catch (MqttException e) {
//				e.printStackTrace();
			}
		}

		return instance;
	}

	public void publishMessage(String topic, byte[] message, int qos, boolean retained)
			throws MqttPersistenceException, MqttException {
		MqttMessage mqttMessage = new MqttMessage(message);
		mqttMessage.setQos(qos);
		mqttMessage.setRetained(retained);
		getInstance().publish(topic, mqttMessage);
	}

	public void subscribeChannel(String topic, int latchNo)
			throws MqttException, InterruptedException {
		getInstance().subscribeWithResponse(topic, (s, mqttMessage) -> {
			List<MqttSubscribeModel> messages = new ArrayList<>();
			CountDownLatch countDownLatch = new CountDownLatch(latchNo);
			MqttSubscribeModel mqttSubscribeModel = new MqttSubscribeModel();
			mqttSubscribeModel.setId(mqttMessage.getId());
			mqttSubscribeModel.setMessage(new String(mqttMessage.getPayload()));
			mqttSubscribeModel.setQos(mqttMessage.getQos());
			messages.add(mqttSubscribeModel);
			countDownLatch.countDown();
			if (mSubscribe != null) {
				mSubscribe.onSubScribe(messages);
			}
		});
		return;
	}

	@Override
	public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
		if (instance != null) {
			return;
		}
		try {
			instance = new MqttClientCust(this);
		} catch (MqttException e) {
		} finally {
		}
	}
}

class MqttClientCust extends MqttClient {
	public MqttClientCust(Mqtt owner) throws MqttException {
		super("tcp://" + owner.getHostAddress() + ":" + String.valueOf(owner.getPort()), owner.getPublisherId());
		System.out.print("MqttClientCust::MqttClientCust(" + "tcp://" + owner.getHostAddress() + ":" + String.valueOf(owner.getPort()) + ")\n");
	}
}
