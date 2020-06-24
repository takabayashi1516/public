package com.example.demo.mysql.jdbc;

import java.util.List;
import java.util.Map;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Controller;

@Controller
public class SqlController implements ApplicationContextAware {

	@Autowired
	private JdbcTemplate mJdbc;
	@Value("${sql_database}")
	private String mDatabase;

	@Override
	public void setApplicationContext(ApplicationContext applicationContext)
			throws BeansException {
	}

	private JdbcTemplate getJdbc() {
		return mJdbc;
	}

/*
	public String test_create() {
		mJdbcTemplate.execute("DROP TABLE IF EXISTS test");
		mJdbcTemplate.execute("CREATE TABLE test (id SERIAL NOT NULL, title VARCHAR(20), sub VARCHAR(20), PRIMARY KEY(id))");
		return null;
	}
	public String test_insert() {
		String[][] data = {{"AAA","aaa"},{"BBB","bbb"},{"CCC","ccc"}};
		for (int i=0; i<data.length; i++) {
			mJdbcTemplate.update("INSERT INTO test (title, sub) VALUES (?, ?)", data[i][0], data[i][1]);
		}
		return null;
	}
*/
	/**
	 * 
	 * @param a_strUser
	 * @param a_strPassword
	 * @return
	 */
	public int createUser(String a_strUser, String a_strPassword) {
		String sql = String.format("create user \'%s\'@\'%%\' IDENTIFIED BY \'%s\';",
				a_strUser, a_strPassword);
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strPrivileges
	 * @param a_strUser
	 * @return
	 */
	public int grantPrivileges(String a_strPrivileges, String a_strUser) {
		String sql = String.format("grant %s on *.* to \'%s\'@\'%%\';",
				a_strPrivileges, a_strUser);
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strUser
	 * @return
	 */
	public int grantAllPrivileges(String a_strUser) {
		return grantPrivileges("all privileges", a_strUser);
	}

	/**
	 * 
	 * @param a_strUser
	 * @param a_strPassword
	 * @return
	 */
	public int setPassword(String a_strUser, String a_strPassword) {
		String sql = String.format("set password for \'%s\'@\'%%\' = password(\'%s\');",
				a_strUser, a_strPassword);
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strTableName
	 * @param a_strSchema
	 * @return
	 */
	public int createTable(String a_strTableName, String a_strSchema) {
		String sql = String.format("create table if not exists %s %s;",
				a_strTableName, a_strSchema);
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strTableName
	 * @return
	 */
	public int dropTable(String a_strTableName) {
		String sql = String.format("drop table if exists %s;", a_strTableName);
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strTableName
	 * @param a_strFields
	 * @param a_strValues
	 * @return
	 */
	public int insertRecord(String a_strTableName, String a_strFields,
			String a_strValues)
	{
		String sql = String.format("insert into %s %s values %s;",
				a_strTableName, a_strFields, a_strValues);
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strTableName
	 * @param a_strSettings
	 * @param a_szConditions
	 * @return
	 */
	public int updateRecord(String a_strTableName, String a_strSettings,
			String a_szConditions)
	{
		String sql = String.format("update %s set %s where %s;",
				a_strTableName, a_strSettings, a_szConditions);
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strTableName
	 * @param a_strConditions
	 * @return
	 */
	public int deleteRecord(String a_strTableName, String a_strConditions)
	{
		String sql = String.format("delete from %s", a_strTableName);
		if ((a_strConditions != null) && (!a_strConditions.isEmpty())) {
			sql += String.format(" where %s", a_strConditions);
		}
		sql += ";";
		return getJdbc().update(sql);
	}

	/**
	 * 
	 * @param a_strTableName
	 * @param a_strCondition
	 * @return
	 */
	public List<Map<String, Object>> select(String a_strTableName, String a_strCondition) {
		String sql = String.format("select * from %s", a_strTableName);;
		if ((a_strCondition != null) && (!a_strCondition.isEmpty())) {
			sql += " where " + a_strCondition;
		}
		sql += ";";
		return getJdbc().queryForList(sql);
	}

	/**
	 * 
	 * @param sql
	 * @return
	 */
	public int update(String sql) {
		return getJdbc().update(sql);
	}
}