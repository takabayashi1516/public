package com.example.demo.thymeleaf;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;

import com.example.demo.json.Json;
import com.example.demo.mysql.jpa.HealthViewEntity;
import com.example.demo.mysql.jpa.MySqlHealth;
import com.example.demo.mysql.jpa.MySqlHealthView;
import com.example.demo.mysql.jpa.MySqlPersonal;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonMappingException;

@Controller
public class ThymeleafController {
	@Autowired
	private MySqlHealth mMySqlHealth;
	@Autowired
	private MySqlPersonal mMySqlPersonal;
	@Autowired
	private MySqlHealthView mMySqlHealthView;

	@RequestMapping(value = "/", method=RequestMethod.GET)
	public String getAllData(
			Model model) {
		ArrayList<HealthViewEntity> list = mMySqlHealthView.getAll();
		return getReferenceHealthDataPage(list, model);
	}

	@RequestMapping(value = "/data", method=RequestMethod.GET)
	public String getPersonalData(
			@RequestParam(value = "name") String name,
			Model model) {
		ArrayList<HealthViewEntity> list = mMySqlHealthView.get(name);
		return getReferenceHealthDataPage(list, model);
	}

	@RequestMapping(value = "/personal", method=RequestMethod.GET)
	public String getPersonalPage(
			@RequestParam(value = "id") String id,
			Model model) {
		model.addAttribute("id", id);
		model.addAttribute("epoch", (new Date()).getTime());
		return "save-bodyTemperature";
	}

	@RequestMapping(value = "/personal/{id}/{epoch}", method=RequestMethod.POST)
	public String postPersonalData(
			@PathVariable("id") String id,
			@PathVariable("epoch") String epoch,
			@RequestParam(value = "data") String data,
			Model model) {
		String result = "Done!";
		long personalId = Long.parseLong(id);
		if (!mMySqlPersonal.isContain(personalId)) {
			result = "fail (not exist)";
			model.addAttribute("result", result);
			return "result";
		}
		if (!mMySqlHealth.append(personalId, Long.parseLong(epoch), Float.parseFloat(data))) {
			result = "fail (database error)";
		}
		model.addAttribute("result", result);
		return "result";
	}

	@RequestMapping(value = "/regist/{name}", method=RequestMethod.POST)
	public String registPersonal(
			@PathVariable("name") String name,
			@RequestParam(value = "mail") String mail,
			Model model) {
		String result = "Done!";
		if (!mMySqlPersonal.update(name, mail)) {
			result = "fail";
		}
		model.addAttribute("result", result);
		return "result";
	}

	@RequestMapping(value = "/delete/{id}", method=RequestMethod.POST)
	public String deletePersonal(
			@PathVariable("id") String id,
			@RequestParam(value = "name") String name,
			Model model) {
		String result = "Done!";
		if (!mMySqlPersonal.delete(Long.parseLong(id), name)) {
			result = "fail";
		}
		model.addAttribute("result", result);
		return "result";
	}

	private String getReferenceHealthDataPage(ArrayList<HealthViewEntity> list, Model model) {
		Json json = new Json();
		try {
			json.load("[]");
			for (int i = 0; i < list.size(); i++) {
				HealthViewEntity e = list.get(i);
				Json nest = new Json();
				nest.load("{}");
				nest.set("id", e.getId());
				nest.set("person", e.getPerson());
				nest.set("name", e.getName());
				nest.set("mail", e.getMail());
				nest.set("timestamp", e.getTimeStamp());
				nest.set("temperature", e.getTemperature());
				json.set(null, i, nest.get(null));
			}
			model.addAttribute("data", json.get(null).toString());
		} catch (JsonMappingException e) {
		} catch (JsonProcessingException e) {
		} catch (IOException e) {
		}
		return "data";
	}
}
