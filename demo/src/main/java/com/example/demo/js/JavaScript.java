package com.example.demo.js;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;

import org.springframework.core.io.ClassPathResource;

public class JavaScript {
	private String mScript = "";

	public JavaScript(String resourceFileName) throws IOException {
		InputStream is = new ClassPathResource(resourceFileName).getInputStream();
		BufferedReader br = new BufferedReader(new InputStreamReader(is));
		String line = null;
		while ((line = br.readLine()) != null) {
			mScript += line;
		}
	}

	public Object execute() throws ScriptException {
		ScriptEngineManager sem = new ScriptEngineManager();
		ScriptEngine se = sem.getEngineByName("JavaScript");
		return se.eval(mScript);
	}
}
