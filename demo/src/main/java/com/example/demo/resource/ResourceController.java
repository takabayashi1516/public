package com.example.demo.resource;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

import org.springframework.context.ResourceLoaderAware;
import org.springframework.core.io.Resource;
import org.springframework.core.io.ResourceLoader;
import org.springframework.stereotype.Controller;

@Controller
public class ResourceController implements ResourceLoaderAware {
	private ResourceLoader mResourceLoader;

	@Override
	public void setResourceLoader(ResourceLoader resourceLoader) {
		mResourceLoader = resourceLoader;
	}

	public byte[] readBytes(String path) throws IOException {
		Resource r = getResource(path);
		if (r == null) {
			return null;
		}
		byte[] rc = Files.readAllBytes(r.getFile().toPath());
		return rc;
	}

	public boolean writeBytes(String path, byte[] data) throws IOException {
		Resource r = getResource(path);
		if (r == null) {
			return false;
		}
		OpenOption opt = StandardOpenOption.WRITE;
		if (!r.exists()) {
			opt = StandardOpenOption.CREATE_NEW;
		}
		Path pth = Files.write(r.getFile().toPath(), data, opt);
		return true;
	}

	private Resource getResource(String path) {
		if (mResourceLoader == null) {
			return null;
		}
		return mResourceLoader.getResource("file:" + path);
	}
}
