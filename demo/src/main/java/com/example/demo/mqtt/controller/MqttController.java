package com.example.demo.mqtt.controller;

import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.springframework.context.annotation.Profile;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.*;

import com.example.demo.mqtt.config.Mqtt;
import com.example.demo.mqtt.exceptions.ExceptionMessages;
import com.example.demo.mqtt.exceptions.MqttException;
import com.example.demo.mqtt.model.MqttPublishModel;
import com.example.demo.mqtt.model.MqttSubscribeModel;

import javax.validation.Valid;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RestController
@RequestMapping(value = "/api/mqtt")
@Profile("front")
public class MqttController {

	public MqttController() {
//		System.out.print("MqttController\n");
	}

	@PostMapping("publish")
	public void publishMessage(@RequestBody @Valid MqttPublishModel messagePublishModel,
			BindingResult bindingResult) throws org.eclipse.paho.client.mqttv3.MqttException {
		if (bindingResult.hasErrors()) {
			throw new MqttException(ExceptionMessages.SOME_PARAMETERS_INVALID);
		}
		Mqtt.getSingleton().publishMessage(messagePublishModel.getTopic(),
				messagePublishModel.getMessage().getBytes(), messagePublishModel.getQos(),
				messagePublishModel.getRetained());
	}

	@GetMapping("subscribe")
	public List<MqttSubscribeModel> subscribeChannel(@RequestParam(value = "topic") String topic,
			@RequestParam(value = "wait_millis") Integer waitMillis) throws InterruptedException,
					org.eclipse.paho.client.mqttv3.MqttException {

		List<MqttSubscribeModel> messages = new ArrayList<>();
		CountDownLatch countDownLatch = new CountDownLatch(10);
		Mqtt.getSingleton().getInstance().subscribeWithResponse(topic, (s, mqttMessage) -> {
			MqttSubscribeModel mqttSubscribeModel = new MqttSubscribeModel();
			mqttSubscribeModel.setId(mqttMessage.getId());
			mqttSubscribeModel.setMessage(new String(mqttMessage.getPayload()));
			mqttSubscribeModel.setQos(mqttMessage.getQos());
			messages.add(mqttSubscribeModel);
			countDownLatch.countDown();
		});

		countDownLatch.await(waitMillis, TimeUnit.MILLISECONDS);

		return messages;
	}

}
