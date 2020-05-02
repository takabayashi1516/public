package com.example.demo.pubsub;

import org.springframework.context.annotation.Profile;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.data.redis.listener.ChannelTopic;
import org.springframework.stereotype.Service;

@Service
@Profile("PubSub")
public class RedisMessagePublisher {

	private final StringRedisTemplate redisTemplate;

	private final ChannelTopic topic;

	public RedisMessagePublisher(StringRedisTemplate redisTemplate, ChannelTopic topic) {
		this.redisTemplate = redisTemplate;
		this.topic = topic;
	}

	public void publish(String message) {
		redisTemplate.convertAndSend(topic.getTopic(), message);
	}
}
