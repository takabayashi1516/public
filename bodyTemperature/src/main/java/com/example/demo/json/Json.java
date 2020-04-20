package com.example.demo.json;

import java.io.IOException;
import java.util.regex.Pattern;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.JsonNodeType;
import com.fasterxml.jackson.databind.node.ObjectNode;

public class Json {
	private ObjectMapper mMapper;
	private JsonNode mRootNode;
	private final String PATH_DELIMITER;

	/**
	 */
	public Json() {
		mMapper = new ObjectMapper();
		mRootNode = null;
		PATH_DELIMITER = Pattern.quote(".");
	}

	/**
	 */
	public synchronized void load(String json) throws JsonMappingException, JsonProcessingException {
		mRootNode = mMapper.readTree(json);
	}

	/**
	 */
	public synchronized void load(byte[] json) throws IOException {
		mRootNode = mMapper.readTree(json);
	}

	/**
	 */
	public synchronized Object get(Object a_objParent, String a_strKeyPath) throws IOException {
		if (a_strKeyPath == null) {
			return a_objParent;
		}
		if (a_strKeyPath.isEmpty()) {
			return a_objParent;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return getValue(a_objParent, nodes);
	}

	/**
	 */
	public synchronized Object get(Object a_objParent, String a_strKeyPath, int a_nIndex) throws IOException {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return getValue(a_objParent, nodes, a_nIndex);
	}

	/**
	 */
	public synchronized Object get(String a_strKeyPath) throws IOException {
		if (a_strKeyPath == null) {
			return mRootNode;
		}
		if (a_strKeyPath.isEmpty()) {
			return mRootNode;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return getValue(nodes);
	}

	/**
	 */
	public synchronized Object get(String a_strKeyPath, int a_nIndex) throws IOException {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return getValue(nodes, a_nIndex);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, byte[] a_byValue) {
		return set(mRootNode, a_strKeyPath, a_byValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, boolean a_bValue) {
		return set(mRootNode, a_strKeyPath, a_bValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nValue) {
		return set(mRootNode, a_strKeyPath, a_nValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, float a_fValue) {
		return set(mRootNode, a_strKeyPath, a_fValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, double a_dfValue) {
		return set(mRootNode, a_strKeyPath, a_dfValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, Object[] a_objArray) {
		return set(mRootNode, a_strKeyPath, a_objArray);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, String a_strValue) {
		return set(mRootNode, a_strKeyPath, a_strValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, Object a_objValue) {
		return set(mRootNode, a_strKeyPath, a_objValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, byte[] a_byValue) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_byValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, boolean a_bValue) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_bValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nValue) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, float a_fValue) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_fValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, double a_dfValue) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_dfValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, String a_strValue) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_strValue);
	}

	/**
	 */
	protected boolean set(Object a_objParent, String a_strKeyPath, boolean[] a_bArray) {
		String[] nodes = null;
		if ((a_strKeyPath != null) && (!(a_strKeyPath.isEmpty()))) {
			nodes = a_strKeyPath.split(PATH_DELIMITER);
		}
		return setValue(a_objParent, nodes, a_bArray);
	}

	/**
	 */
	protected boolean set(Object a_objParent, String a_strKeyPath, int[] a_nArray) {
		String[] nodes = null;
		if ((a_strKeyPath != null) && (!(a_strKeyPath.isEmpty()))) {
			nodes = a_strKeyPath.split(PATH_DELIMITER);
		}
		return setValue(a_objParent, nodes, a_nArray);
	}

	/**
	 */
	protected boolean set(Object a_objParent, String a_strKeyPath, float[] a_fArray) {
		String[] nodes = null;
		if ((a_strKeyPath != null) && (!(a_strKeyPath.isEmpty()))) {
			nodes = a_strKeyPath.split(PATH_DELIMITER);
		}
		return setValue(a_objParent, nodes, a_fArray);
	}

	/**
	 */
	protected boolean set(Object a_objParent, String a_strKeyPath, double[] a_dfArray) {
		String[] nodes = null;
		if ((a_strKeyPath != null) && (!(a_strKeyPath.isEmpty()))) {
			nodes = a_strKeyPath.split(PATH_DELIMITER);
		}
		return setValue(a_objParent, nodes, a_dfArray);
	}

	/**
	 */
	protected boolean set(Object a_objParent, String a_strKeyPath, String[] a_strArray) {
		String[] nodes = null;
		if ((a_strKeyPath != null) && (!(a_strKeyPath.isEmpty()))) {
			nodes = a_strKeyPath.split(PATH_DELIMITER);
		}
		return setValue(a_objParent, nodes, a_strArray);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, Object[] a_objArray) {
		String[] nodes = null;
		if ((a_strKeyPath != null) && (!(a_strKeyPath.isEmpty()))) {
			nodes = a_strKeyPath.split(PATH_DELIMITER);
		}
		return setValue(a_objParent, nodes, a_objArray);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, Object a_objValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			((ObjectNode) a_objParent).removeAll();
			((ObjectNode) a_objParent).setAll((ObjectNode) a_objValue);
			return true;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_objValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nIndex, byte[] a_byValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			if (((JsonNode) a_objParent).getNodeType() == JsonNodeType.ARRAY) {
				((ArrayNode) a_objParent).remove(a_nIndex);
				((ArrayNode) a_objParent).insert(a_nIndex, a_byValue);
				return true;
			}
			return false;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nIndex, a_byValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nIndex, boolean a_bValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			if (((JsonNode) a_objParent).getNodeType() == JsonNodeType.ARRAY) {
				((ArrayNode) a_objParent).remove(a_nIndex);
				((ArrayNode) a_objParent).insert(a_nIndex, a_bValue);
				return true;
			}
			return false;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nIndex, a_bValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nIndex, int a_nValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			if (((JsonNode) a_objParent).getNodeType() == JsonNodeType.ARRAY) {
				((ArrayNode) a_objParent).remove(a_nIndex);
				((ArrayNode) a_objParent).insert(a_nIndex, a_nValue);
				return true;
			}
			return false;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nIndex, a_nValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nIndex, float a_fValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			if (((JsonNode) a_objParent).getNodeType() == JsonNodeType.ARRAY) {
				((ArrayNode) a_objParent).remove(a_nIndex);
				((ArrayNode) a_objParent).insert(a_nIndex, a_fValue);
				return true;
			}
			return false;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nIndex, a_fValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nIndex, double a_dfValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			if (((JsonNode) a_objParent).getNodeType() == JsonNodeType.ARRAY) {
				((ArrayNode) a_objParent).remove(a_nIndex);
				((ArrayNode) a_objParent).insert(a_nIndex, a_dfValue);
				return true;
			}
			return false;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nIndex, a_dfValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nIndex, String a_strValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			if (((JsonNode) a_objParent).getNodeType() == JsonNodeType.ARRAY) {
				((ArrayNode) a_objParent).remove(a_nIndex);
				((ArrayNode) a_objParent).insert(a_nIndex, a_strValue);
				return true;
			}
			return false;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nIndex, a_strValue);
	}

	/**
	 */
	public synchronized boolean set(Object a_objParent, String a_strKeyPath, int a_nIndex, Object a_objValue) {
		if ((a_strKeyPath == null) || (a_strKeyPath.isEmpty())) {
			if (((JsonNode) a_objParent).getNodeType() == JsonNodeType.ARRAY) {
				((ArrayNode) a_objParent).remove(a_nIndex);
				((ArrayNode) a_objParent).insert(a_nIndex, (JsonNode) a_objValue);
				return true;
			}
			return false;
		}
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return setValue(a_objParent, nodes, a_nIndex, a_objValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, byte[] a_byValue) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_byValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, boolean a_bValue) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_bValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, int a_nValue) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_nValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, float a_fValue) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_fValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, double a_dfValue) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_dfValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, String a_strValue) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_strValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, Object[] a_objArray) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_objArray);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int a_nIndex, Object a_objValue) {
		return set(mRootNode, a_strKeyPath, a_nIndex, a_objValue);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, boolean[] a_bArray) {
		return set(mRootNode, a_strKeyPath, a_bArray);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, int[] a_nArray) {
		return set(mRootNode, a_strKeyPath, a_nArray);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, float[] a_fArray) {
		return set(mRootNode, a_strKeyPath, a_fArray);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, double[] a_dfArray) {
		return set(mRootNode, a_strKeyPath, a_dfArray);
	}

	/**
	 */
	public synchronized boolean set(String a_strKeyPath, String[] a_strArray) {
		return set(mRootNode, a_strKeyPath, a_strArray);
	}

	/**
	 */
	public synchronized boolean del(Object a_objParent, String a_strKeyPath) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return deleteKey(a_objParent, nodes);
	}

	/**
	 */
	public synchronized boolean del(String a_strKeyPath) {
		return del(mRootNode, a_strKeyPath);
	}

	/**
	 */
	public synchronized boolean exist(String a_strKeyPath) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return isExist(nodes);
	}

	/**
	 */
	public synchronized boolean exist(String a_strKeyPath, int a_nIndex) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return isExist(nodes, a_nIndex);
	}

	/**
	 */
	public synchronized boolean exist(Object a_objParent, String a_strKeyPath) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return isExist(a_objParent, nodes);
	}

	/**
	 */
	public synchronized boolean exist(Object a_objParent, String a_strKeyPath, int a_nIndex) {
		String[] nodes = a_strKeyPath.split(PATH_DELIMITER);
		return isExist(a_objParent, nodes, a_nIndex);
	}

	/**
	 */
	protected Object getValue(Object a_objParent, String[] a_strKeys) throws IOException {
		String[] keys = a_strKeys;
		if (keys.length <= 0) {
			return null;
		}
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return null;
			}
		}
		Object rc = null;
		switch (node.getNodeType()) {
		case ARRAY:
		case OBJECT:
			rc = node;
			break;
		case BINARY:
			rc = node.binaryValue();
			break;
		case BOOLEAN:
			rc = node.booleanValue();
			break;
		case NUMBER:
			if (node.isIntegralNumber()) {
				rc = node.intValue();
			} else if (node.isFloat()) {
				rc = node.floatValue();
			} else if (node.isDouble()) {
				rc = node.doubleValue();
			} else {
				System.out.println(this.getClass().getName() + "::getValue(): NUMBER");
			}
			break;
		case STRING:
			rc = node.asText();
			break;
		case POJO:
		case MISSING:
		case NULL:
		default:
			System.out.println(this.getClass().getName() + "::getValue(): other");
			break;
		}
		return rc;
	}

	/**
	 */
	protected Object getValue(Object a_objParent, String[] a_strKeys, int a_nIndex) throws IOException {
		String[] keys = a_strKeys;
		if (keys.length <= 0) {
			return null;
		}
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return null;
			}
		}
		Object rc = null;
		if (node.getNodeType() == JsonNodeType.ARRAY) {
			rc = node.get(a_nIndex);
		}
		return rc;
	}

	/**
	 */
	protected Object getValue(String[] a_strKeys) throws IOException {
		return getValue(mRootNode, a_strKeys);
	}

	/**
	 */
	protected Object getValue(String[] a_strKeys, int a_nIndex) throws IOException {
		return getValue(mRootNode, a_strKeys, a_nIndex);
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, byte[] a_byArray) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		((ObjectNode) node).put(keys[keys.length - 1], a_byArray);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, boolean a_bValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		((ObjectNode) node).put(keys[keys.length - 1], a_bValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int a_nValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		((ObjectNode) node).put(keys[keys.length - 1], a_nValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, float a_fValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		((ObjectNode) node).put(keys[keys.length - 1], a_fValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, double a_dfValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		((ObjectNode) node).put(keys[keys.length - 1], a_dfValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, String a_strValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		((ObjectNode) node).put(keys[keys.length - 1], a_strValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, Object a_jsonValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		((ObjectNode) node).set(keys[keys.length - 1], (JsonNode) a_jsonValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, boolean[] a_bArray) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		ObjectMapper mapper = new ObjectMapper();
		ArrayNode array = mapper.createArrayNode();
		if (keys == null) {
			for (boolean b : a_bArray) {
				array.add(b);
			}
			a_objParent = array;
			return true;
		}
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		for (int i = 0; i < a_bArray.length; i++) {
			array.add(a_bArray[i]);
		}
		((ObjectNode) node).set(keys[keys.length - 1], array);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int[] a_nArray) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		ObjectMapper mapper = new ObjectMapper();
		ArrayNode array = mapper.createArrayNode();
		if (keys == null) {
			for (int n : a_nArray) {
				array.add(n);
			}
			a_objParent = array;
			return true;
		}
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		for (int i = 0; i < a_nArray.length; i++) {
			array.add(a_nArray[i]);
		}
		((ObjectNode) node).set(keys[keys.length - 1], array);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, float[] a_fArray) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		ObjectMapper mapper = new ObjectMapper();
		ArrayNode array = mapper.createArrayNode();
		if (keys == null) {
			for (float f : a_fArray) {
				array.add(f);
			}
			a_objParent = array;
			return true;
		}
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		for (int i = 0; i < a_fArray.length; i++) {
			array.add(a_fArray[i]);
		}
		((ObjectNode) node).set(keys[keys.length - 1], array);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, double[] a_dfArray) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		ObjectMapper mapper = new ObjectMapper();
		ArrayNode array = mapper.createArrayNode();
		if (keys == null) {
			for (double d : a_dfArray) {
				array.add(d);
			}
			a_objParent = array;
			return true;
		}
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		for (int i = 0; i < a_dfArray.length; i++) {
			array.add(a_dfArray[i]);
		}
		((ObjectNode) node).set(keys[keys.length - 1], array);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, String[] a_strArray) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		ObjectMapper mapper = new ObjectMapper();
		ArrayNode array = mapper.createArrayNode();
		if (keys == null) {
			for (String s : a_strArray) {
				array.add(s);
			}
			a_objParent = array;
			return true;
		}
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		for (int i = 0; i < a_strArray.length; i++) {
			array.add(a_strArray[i]);
		}
		((ObjectNode) node).set(keys[keys.length - 1], array);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, Object[] a_jsonArray) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		ObjectMapper mapper = new ObjectMapper();
		ArrayNode array = mapper.createArrayNode();
		if (keys == null) {
			for (Object obj : a_jsonArray) {
				array.add((JsonNode) obj);
			}
			a_objParent = array;
			return true;
		}
		for (int i = 0; i < (keys.length - 1); i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		for (int i = 0; i < a_jsonArray.length; i++) {
			array.add((JsonNode) a_jsonArray[i]);
		}
		((ObjectNode) node).set(keys[keys.length - 1], array);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int a_nIndex, boolean a_bValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		if (node.getNodeType() != JsonNodeType.ARRAY) {
			return false;
		}
		((ArrayNode) node).remove(a_nIndex);
		((ArrayNode) node).insert(a_nIndex, a_bValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int a_nIndex, int a_nValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		if (node.getNodeType() != JsonNodeType.ARRAY) {
			return false;
		}
		((ArrayNode) node).remove(a_nIndex);
		((ArrayNode) node).insert(a_nIndex, a_nValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int a_nIndex, float a_fValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		if (node.getNodeType() != JsonNodeType.ARRAY) {
			return false;
		}
		((ArrayNode) node).remove(a_nIndex);
		((ArrayNode) node).insert(a_nIndex, a_fValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int a_nIndex, double a_dfValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		if (node.getNodeType() != JsonNodeType.ARRAY) {
			return false;
		}
		((ArrayNode) node).remove(a_nIndex);
		((ArrayNode) node).insert(a_nIndex, a_dfValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int a_nIndex, String a_strValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		if (node.getNodeType() != JsonNodeType.ARRAY) {
			return false;
		}
		((ArrayNode) node).remove(a_nIndex);
		((ArrayNode) node).insert(a_nIndex, a_strValue);
		return true;
	}

	/**
	 */
	protected boolean setValue(Object a_objParent, String[] a_strKeys, int a_nIndex, Object a_objValue) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < keys.length; i++) {
			node = node.get(keys[i]);
			if (node == null) {
				return false;
			}
		}
		if (node.getNodeType() != JsonNodeType.ARRAY) {
			return false;
		}
		((ArrayNode) node).remove(a_nIndex);
		((ArrayNode) node).insert(a_nIndex, (JsonNode) a_objValue);
		return true;
	}

	/**
	 */
	protected boolean deleteKey(Object a_objParent, String[] a_strKeys) {
		String[] keys = a_strKeys;
		JsonNode node = (JsonNode) a_objParent;

		for (int i = 0; i < keys.length; i++) {
			JsonNode node_tmp = node.get(keys[i]);
			if (node_tmp == null) {
				return false;
			}
			if (i < (keys.length - 1)) {
				node = node_tmp;
			}
		}
		((ObjectNode) node).remove(keys[keys.length - 1]);
		return true;
	}

	/**
	 */
	protected boolean isExist(String[] a_strKeys) {
		return isExist(mRootNode, a_strKeys);
	}

	/**
	 */
	protected boolean isExist(String[] a_strKeys, int a_nIndex) {
		return isExist(mRootNode, a_strKeys, a_nIndex);
	}

	/**
	 */
	protected boolean isExist(Object a_objParent, String[] a_strKeys) {
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < a_strKeys.length; i++) {
			node = node.get(a_strKeys[i]);
			if (node == null) {
				return false;
			}
		}
		return true;
	}

	/**
	 */
	protected boolean isExist(Object a_objParent, String[] a_strKeys, int a_nIndex) {
		JsonNode node = (JsonNode) a_objParent;
		for (int i = 0; i < a_strKeys.length; i++) {
			node = node.get(a_strKeys[i]);
			if (node == null) {
				return false;
			}
		}
		Object obj = null;
		if (node.getNodeType() == JsonNodeType.ARRAY) {
			obj = node.get(a_nIndex);
		}
		return (obj != null);
	}
}
