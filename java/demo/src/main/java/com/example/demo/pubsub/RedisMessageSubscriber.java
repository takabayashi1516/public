package com.example.demo.pubsub;

import java.nio.charset.StandardCharsets;

import org.springframework.data.redis.connection.Message;
import org.springframework.data.redis.connection.MessageListener;

public class RedisMessageSubscriber implements MessageListener {

	@Override
	public void onMessage(Message message, byte[] pattern) {
		// TODO Auto-generated method stub
		System.out.println("message received : " + new String(message.getBody(), StandardCharsets.UTF_8));
	}
}
