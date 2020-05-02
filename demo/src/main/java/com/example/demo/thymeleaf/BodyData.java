package com.example.demo.thymeleaf;

import java.io.Serializable;

public class BodyData implements Serializable {
	private static final long serialVersionUID = 1L;

    private Integer id;
    private String name;

    public Integer getId() {
    	return id;
    }
    public String getName() {
    	return name;
    }

    public void setId(Integer _id) {
    	id = _id;
    }
    public void setName(String _name) {
    	name = _name;
    }
}
