package com.example.demo.mqtt.handler;

import java.util.List;
import com.example.demo.mqtt.model.MqttSubscribeModel;

public interface MqttSubScribe {
	public void onSubScribe(List<MqttSubscribeModel> messages);
}
