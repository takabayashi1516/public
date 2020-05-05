package com.example.demo.thymeleaf;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;

import com.example.demo.json.Json;
import com.example.demo.mysql.jpa.HealthDataEntity;
import com.example.demo.mysql.jpa.HealthViewEntity;
import com.example.demo.mysql.jpa.MySqlHealth;
import com.example.demo.mysql.jpa.MySqlHealthView;
import com.example.demo.mysql.jpa.MySqlPersonal;
import com.example.demo.mysql.jpa.PersonalDataEntity;
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

	@Value("${admin_id}")
	private long mAdminId;

	@Autowired
	PasswordEncoder mEncoder;

	/**
	 * 
	 * @return
	 */
	@Bean
	PasswordEncoder encoder() {
		return new BCryptPasswordEncoder();
	}

	/**
	 * 
	 * @param target
	 * @return
	 */
	public String getHash(String target) {
		return mEncoder.encode(target);
	}

	/**
	 * 
	 * @param target
	 * @param hash
	 * @return
	 */
	public boolean testHash(String target, String hash) {
		return mEncoder.matches(target, hash);
	}

	// $ curl http://${host_front}:${server.port}/control?hash=...
	/**
	 * 
	 * @param hash
	 * @param model
	 * @return
	 */
	@RequestMapping(value = "/control", method=RequestMethod.GET)
	public String getAllData(
			@RequestParam(value = "hash") String hash,
			Model model) {
		return getReferenceHealthDataPage(hash, model, true);
	}

	// $ curl http://${host_front}:${server.port}/data?hash=...
	/**
	 * 
	 * @param hash
	 * @param model
	 * @return
	 */
	@RequestMapping(value = "/data", method=RequestMethod.GET)
	public String getPersonalData(
			@RequestParam(value = "hash") String hash,
			Model model) {	
		return getReferenceHealthDataPage(hash, model, false);
	}

	// $ curl http://${host_front}:${server.port}/personal?hash=...
	/**
	 * 
	 * @param hash
	 * @param model
	 * @return
	 */
	@RequestMapping(value = "/personal", method=RequestMethod.GET)
	public String getPersonalPage(
			@RequestParam(value = "hash") String hash,
			Model model) {
		model.addAttribute("hash", hash);
		model.addAttribute("epoch", (new Date()).getTime());
		return "save-bodyTemperature";
	}

	/**
	 * 
	 * @param data
	 * @param epoch
	 * @param hash
	 * @param model
	 * @return
	 */
	@RequestMapping(value = "/personal/{data}/{epoch}", method=RequestMethod.POST)
	public String postPersonalData(
			@PathVariable("data") String data,
			@PathVariable("epoch") String epoch,
			@RequestParam(value = "hash") String hash,
			Model model) {
		String result = "success";
		PersonalDataEntity e = getPersonalDataFromHash(hash);
		if (e == null) {
			result = "fail (not exist)";
			model.addAttribute("result", result);
			return "result";
		}
		long personalId = e.getId();
		if (!mMySqlHealth.append(personalId, Long.parseLong(epoch), Float.parseFloat(data))) {
			result = "fail (database error)";
		}
		model.addAttribute("result", result);
		return "result";
	}

	// $ curl -X POST http://${host_front}:${server.port}/regist/${name}?mail=${mail}
	/**
	 * 
	 * @param name
	 * @param mail
	 * @param model
	 * @return
	 */
	@RequestMapping(value = "/regist/{name}", method=RequestMethod.POST)
	public String registPersonal(
			@PathVariable("name") String name,
			@RequestParam(value = "mail") String mail,
			Model model) {
		String result = ("success: " + getHash(mail));
		result += (" id: " + mMySqlPersonal.update(name, mail));
		model.addAttribute("result", result);
		return "result";
	}

	// $ curl -X POST http://${host_front}:${server.port}/delete/${id}?hash=...
	/**
	 * 
	 * @param id
	 * @param name
	 * @param model
	 * @return
	 */
	@RequestMapping(value = "/delete/{id}", method=RequestMethod.POST)
	public String deletePersonal(
			@PathVariable("id") String id,
			@RequestParam(value = "hash") String hash,
			Model model) {
		String result = "fail";
		PersonalDataEntity e = getPersonalDataFromHash(hash);
		if (e == null) {
			model.addAttribute("result", result);
			return "result";
		}
		if (e.getId() != Long.parseLong(id)) {
			model.addAttribute("result", result);
			return "result";
		}
		if (mMySqlPersonal.delete(Long.parseLong(id))) {
			result = "success";
		}
		model.addAttribute("result", result);
		return "result";
	}

	// $ curl -X POST http://${host_front}:${server.port}/update/${id}?v=${value}
	@RequestMapping(value = "/update/{id}", method=RequestMethod.PUT)
	public String modifyDate(
			@PathVariable("id") String id,
			@RequestParam(value = "v") long value,
			Model model) {
		String result = "success";
/*
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");
		TimeZone tz = TimeZone.getTimeZone("Asia/Tokyo");
		long epoch_s = sdf.parse(date).getTime();
		mMySqlHealth.getRange(person, ts_start, ts_end);
*/
		HealthDataEntity e = mMySqlHealth.get(Long.parseLong(id));
		e.setTimeStamp(e.getTimeStamp() + value);
		mMySqlHealth.update(e);
		model.addAttribute("result", result);
		return "result";
	}

	/**
	 * 
	 * @param hash
	 * @return
	 */
	private boolean isAdministrator(String hash) {
		PersonalDataEntity e = mMySqlPersonal.get(mAdminId);
		return testHash(e.getMail(), hash);
	}

	public String getAdministratorHash() {
		PersonalDataEntity e = mMySqlPersonal.get(mAdminId);
		return getHash(e.getMail());
	}

	/**
	 * 
	 * @param hash
	 * @return
	 */
	private PersonalDataEntity getPersonalDataFromHash(String hash) {
		PersonalDataEntity rc = null;
		List<PersonalDataEntity> list = mMySqlPersonal.getAll();
		for (int i = 0; i < list.size(); i++) {
			PersonalDataEntity e = list.get(i);
			if (testHash(e.getMail(), hash)) {
				rc = e;
				break;
			}
		}
		return rc;
	}

	/**
	 * 
	 * @param hash
	 * @param model
	 * @param ctrl
	 * @return
	 */
	private String getReferenceHealthDataPage(String hash, Model model, boolean ctrl) {
		Json json = new Json();
		try {
			json.load("[]");

			List<HealthViewEntity> list = null;
			if (ctrl) {
				if (!isAdministrator(hash)) {
					return "data";
				}
				list = mMySqlHealthView.getAll();
			} else {
				PersonalDataEntity p = getPersonalDataFromHash(hash);
				list = mMySqlHealthView.get(p.getId());
			}

			for (int i = 0; i < list.size(); i++) {
				HealthViewEntity e = list.get(i);
//				if (!testHash(e.getMail(), hash)) {
//					continue;
//				}
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
		} catch (JsonMappingException e) {
		} catch (JsonProcessingException e) {
		} catch (IOException e) {
		} finally {
			try {
				model.addAttribute("data", json.get(null).toString());
			} catch (IOException e) {
				model.addAttribute("data", "[]");
			}
		}
		return "data";
	}
}
