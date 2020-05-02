// http://localhost:18080/thymeleaf?data1=aaa&data2=bbb
// curl -X POST -H 'Content-Type:application/json' http://localhost:8080/thymeleaf -d "{\"id\": 1, \"name\":\"aaaaa\"}"
// curl -X GET -H 'Content-Type:application/json' http://localhost:8080/thymeleaf -d "{\"id\": 1, \"name\":\"aaaaa\"}"
// curl -X POST -H 'Content-Type:application/json' http://localhost:8080/thymeleaf2 -d "{\"id\": 1, \"name\":\"aaaaa\"}"
package com.example.demo.thymeleaf;

import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;

@Controller
@Profile("front")
public class ThymeleafController {
	private byte[] mData = null;

	public void setData(byte[] data) {
		mData = data;
	}

	@RequestMapping(value = "/thymeleafJson", method=RequestMethod.GET)
	public String thymeleafJsonGet(Model model) {
		model.addAttribute("data", new String(mData));
		return "thymeleaf_test";
	}

	@RequestMapping(value = "/thymeleaf", method=RequestMethod.POST)
	public String thymeleafPost(/*@RequestParam(value = "data", required = false) String data,*/
			@RequestBody String body,
			Model model) {
		model.addAttribute("data", body);
		return "thymeleaf_test";
	}

	@RequestMapping(value = "/thymeleaf", method=RequestMethod.GET)
	public String thymeleafGet(/*@RequestParam(value = "data", required = false) String data,*/
			@RequestBody String body,
			Model model) {
		model.addAttribute("data", body);
		return "thymeleaf_test";
	}

	@RequestMapping(value = "/thymeleaf2", method=RequestMethod.POST)
	public String thymeleafPost2(@RequestBody BodyData body,
			Model model) {
		model.addAttribute("data", String.valueOf(body.getId()) + ": " + body.getName());
		return "thymeleaf_test";
	}

	@RequestMapping(path = "/thymeleaf2", method = RequestMethod.GET)
	public String thymeleafTestMap(
			@RequestParam(value="data1", required=false, defaultValue="DATA1") String data1,
			@RequestParam(value="data2", required=false, defaultValue="DATA2") String data2,
			Model model) {
		model.addAttribute("data1", data1);
		model.addAttribute("data2", data2);
		return "thymeleafTest";
	}
}
