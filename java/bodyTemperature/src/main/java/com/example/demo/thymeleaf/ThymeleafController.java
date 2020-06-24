package com.example.demo.thymeleaf;

import java.io.IOException;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.util.UriUtils;

import com.example.demo.json.Json;
import com.example.demo.mysql.jpa.HealthDataEntity;
import com.example.demo.mysql.jpa.HealthViewEntity;
import com.example.demo.mysql.jpa.HealthViewSpec;
import com.example.demo.mysql.jpa.Health;
import com.example.demo.mysql.jpa.HealthView;
import com.example.demo.mysql.jpa.Personal;
import com.example.demo.mysql.jpa.PersonalDataEntity;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonMappingException;

@Controller
public class ThymeleafController {
	@Autowired
	private Health mHealth;
	@Autowired
	private Personal mPersonal;
	@Autowired
	private HealthView mHealthView;

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

	@RequestMapping(value = "/_report.csv", method = RequestMethod.GET, produces = "text/csv")
	public void _reportData(
			@RequestParam(value = "hash") String hash,
			Writer writer) throws IOException {
		List<HealthViewEntity> list = null;
		if (!isAdministrator(hash)) {
			writer.write("permition denied.");
			return;
		}
		list = mHealthView.getRepository().findAll();
		writer.write("id,name,mail,temperature,timestamp\r\n");
		for (int i = 0; i < list.size(); i++) {
			HealthViewEntity e = list.get(i);
			String record = "";
			record += String.valueOf(e.getId());
			record += ",";
			record += e.getName();
			record += ",";
			record += e.getMail();
			record += ",";
			record += String.valueOf(e.getTemperature());
			record += ",";
			record += String.valueOf(e.getTimeStamp());
			record += "\r\n";
			writer.write(record);
		}
	}

	@RequestMapping(value = "/report/{csv}.csv", method = RequestMethod.GET, produces = "text/csv")
	public ResponseEntity<byte[]> reportData(
			@RequestParam(value = "hash") String hash,
			@PathVariable("csv") String csv) throws IOException {
		String csv_file = csv + ".csv";
		HttpHeaders headers = new HttpHeaders();
		String headerValue = String.format("attachment; filename=\"%s\"; filename*=UTF-8''%s",
				csv_file, UriUtils.encode(csv_file, StandardCharsets.UTF_8.name()));
		headers.add(HttpHeaders.CONTENT_DISPOSITION, headerValue);

		List<HealthViewEntity> list = null;
		if (!isAdministrator(hash)) {
			list = mHealthView.getRepository().findAll(Specification.where(
					HealthViewSpec.fieldsEquals("mPerson", getPersonalDataFromHash(hash).getId())));
		} else {
			list = mHealthView.getRepository().findAll();
		}
		String record = "id,name,mail,temperature,timestamp,date,time\r\n";
		for (int i = 0; i < list.size(); i++) {
			HealthViewEntity e = list.get(i);
			record += String.valueOf(e.getId());
			record += ",";
			record += e.getName();
			record += ",";
			record += e.getMail();
			record += ",";
			record += String.valueOf(e.getTemperature());
			record += ",";
			record += String.valueOf(e.getTimeStamp());
			record += ",";
			Date d = (new Date(e.getTimeStamp()));
			SimpleDateFormat sdf = new SimpleDateFormat("yyyy'/'MM'/'dd','kk':'mm':'ss");
			sdf.setTimeZone(TimeZone.getTimeZone("Asia/Tokyo"));
			record += sdf.format(d);
			record += "\r\n";
		}
		return new ResponseEntity<>(record.getBytes("MS932"), headers, HttpStatus.OK);
	}

	// $ curl http://${host_front}:${server.port}/control?hash=...
	/**
	 * 
	 * @param hash
	 * @param model
	 * @return
	 */
	@RequestMapping(value = "/control", method = RequestMethod.GET)
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
	@RequestMapping(value = "/data", method = RequestMethod.GET)
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
	@RequestMapping(value = "/personal", method = RequestMethod.GET)
	public String getPersonalPage(
			@RequestParam(value = "hash") String hash,
			Model model) {
		PersonalDataEntity e = getPersonalDataFromHash(hash);
		if (e != null) {
			model.addAttribute("name", e.getName());
		}
		model.addAttribute("hash", hash);
		model.addAttribute("epoch", (new Date()).getTime());
		HealthDataEntity he = mHealth.getLatest(e.getId());
		float temperature = 36f;
		if (he != null) {
			temperature = he.getTemperature();
		}
		model.addAttribute("temperature", String.format("%.1f", temperature));
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
	@RequestMapping(value = "/personal/{data}/{epoch}", method = RequestMethod.POST)
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
		if (!mHealth.append(personalId, Long.parseLong(epoch), Float.parseFloat(data))) {
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
	@RequestMapping(value = "/regist/{name}", method = RequestMethod.POST)
	public String registPersonal(
			@PathVariable("name") String name,
			@RequestParam(value = "mail") String mail,
			Model model) {
		String result = ("success: " + getHash(mail));
		result += (" id: " + mPersonal.update(name, mail));
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
	@RequestMapping(value = "/delete/{id}", method = RequestMethod.POST)
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
		if (mPersonal.delete(Long.parseLong(id))) {
			result = "success";
		}
		model.addAttribute("result", result);
		return "result";
	}

	// $ curl -X POST http://${host_front}:${server.port}/update/${id}?v=${value}
	@RequestMapping(value = "/update/{id}", method = RequestMethod.PUT)
	public String modifyDate(
			@PathVariable("id") String id,
			@RequestParam(value = "v") long value,
			Model model) {
		String result = "success";
/*
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");
		TimeZone tz = TimeZone.getTimeZone("Asia/Tokyo");
		long epoch_s = sdf.parse(date).getTime();
		mHealth.getRange(person, ts_start, ts_end);
*/
		HealthDataEntity e = mHealth.get(Long.parseLong(id));
		e.setTimeStamp(e.getTimeStamp() + value);
		mHealth.update(e);
		model.addAttribute("result", result);
		return "result";
	}

	/**
	 * 
	 * @param hash
	 * @return
	 */
	private boolean isAdministrator(String hash) {
		PersonalDataEntity e = mPersonal.get(mAdminId);
		return testHash(e.getMail(), hash);
	}

	public String getAdministratorHash() {
		PersonalDataEntity e = mPersonal.get(mAdminId);
		return getHash(e.getMail());
	}

	/**
	 * 
	 * @param hash
	 * @return
	 */
	private PersonalDataEntity getPersonalDataFromHash(String hash) {
		PersonalDataEntity rc = null;
		List<PersonalDataEntity> list = mPersonal.getRepository().findAll();
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
		float max = 0f;
		try {
			json.load("[]");

			List<HealthViewEntity> list = null;
			if (ctrl) {
				if (!isAdministrator(hash)) {
					return "data";
				}
				list = mHealthView.getRepository().findAll();
			} else {
				PersonalDataEntity e = getPersonalDataFromHash(hash);
				list = mHealthView.getRepository().findAll(Specification.where(
						HealthViewSpec.fieldsEquals("mPerson", e.getId())));
			}

			for (int i = 0; i < list.size(); i++) {
				HealthViewEntity e = list.get(i);
//				if (!testHash(e.getMail(), hash)) {
//					continue;
//				}
				float temp = e.getTemperature();
				if (max < temp) {
					max = temp;
				}
				Json nest = new Json();
				nest.load("{}");
				nest.set("id", e.getId());
				nest.set("person", e.getPerson());
				nest.set("name", e.getName());
				nest.set("mail", e.getMail());
				nest.set("timestamp", e.getTimeStamp());
				nest.set("temperature", temp);
				json.set(null, i, nest.get(null));
			}
		} catch (JsonMappingException e) {
		} catch (JsonProcessingException e) {
		} catch (IOException e) {
		} finally {
			max = (float) Math.ceil(max);
			try {
				model.addAttribute("max", max);
				model.addAttribute("data", json.get(null).toString());
			} catch (IOException e) {
				model.addAttribute("data", "[]");
			}
		}
		return "data";
	}
}
