package com.example.demo.thymeleaf;

import org.springframework.web.multipart.MultipartFile;
import lombok.Data;

@Data
public class UploadForm/* implements Serializable */{
	private MultipartFile uploadFile;
	public MultipartFile getUploadFile() {
		return uploadFile;
	}
}
