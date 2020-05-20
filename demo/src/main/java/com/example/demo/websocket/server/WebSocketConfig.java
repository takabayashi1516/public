package com.example.demo.websocket.server;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;

@EnableWebSocket
@Configuration
@Profile("server")
public class WebSocketConfig implements WebSocketConfigurer {
	private final String PATH;
	@Autowired
	private WebSocketDispacher mDispacher;

	public WebSocketConfig() {
		PATH = "/";
	}

	@Override
	public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
		// TODO Auto-generated method stub
		registry.addHandler(mDispacher, PATH).setAllowedOrigins("*");
	}
}