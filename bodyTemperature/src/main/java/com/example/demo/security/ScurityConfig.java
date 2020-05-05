package com.example.demo.security;

import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.builders.WebSecurity;
import org.springframework.security.config.annotation.web.configuration.WebSecurityConfigurerAdapter;

@Configuration
public class ScurityConfig extends WebSecurityConfigurerAdapter {

	@Override
	public void configure(WebSecurity web) throws Exception {
		web.ignoring().antMatchers("/chart/*", "/d3/*", "/regist/*", "/data/*", "/personal/*", "/personal/*/*", "/delete/*");
	}

	@Override
	protected void configure(HttpSecurity http) throws Exception {
		http
			// AUTHORIZE
			.authorizeRequests()
			.mvcMatchers("/**", "/regist/*", "/personal/*", "/personal/*/*", "/delete/*").permitAll()
			.anyRequest()
			.authenticated()
			.and()
			.formLogin()
			.defaultSuccessUrl("/success")
		// end
		;
	}

}
