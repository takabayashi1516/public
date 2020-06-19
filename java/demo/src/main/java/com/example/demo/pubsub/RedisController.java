package com.example.demo.pubsub;

import org.springframework.context.annotation.Profile;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
@Profile("PubSub")
public class RedisController {

	private final RedisMessagePublisher publisher;

	public RedisController(RedisMessagePublisher publisher) {
		this.publisher = publisher;
	}

	@GetMapping("publish")
	public String publish(@RequestParam String message) {
		publisher.publish(message);

		return "published " + message;
	}
}