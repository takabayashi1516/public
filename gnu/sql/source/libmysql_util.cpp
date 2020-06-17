
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <libmysql_util.h>


/**
 */
CMySqlUtil::CMySqlUtil(const char a_szDbName[], const char a_szUser[],
		const char a_szHost[], const char a_szPasswd[])
	:	CMySqlUtilBase(a_szDbName, a_szUser, a_szHost, a_szPasswd)
{
}

/**
 */
CMySqlUtil::CMySqlUtil(const char a_szDbName[], const char a_szUser[],
		const char *a_pszPasswd/* = NULL*/)
	:	CMySqlUtilBase(a_szDbName, a_szUser, (a_pszPasswd)? a_pszPasswd : "")
{
}

/**
 */
CMySqlUtil::~CMySqlUtil()
{
}

/**
	mysql > create user `testuser`@`localhost` IDENTIFIED BY 'password';
 */
int CMySqlUtil::createUser(const char a_szUser[], const char a_szHost[],
		const char a_szPasswd[])
{
	char cmd[256];
	::snprintf(cmd, sizeof(cmd), "create user %s@%s IDENTIFIED BY \'%s\';",
			a_szUser, a_szHost, a_szPasswd);
	return query(cmd);
}

/**
	mysql > create user `testuser`@`localhost` IDENTIFIED BY 'password';
 */
int CMySqlUtil::createUser(const char a_szUser[],
		const char *a_pszPasswd/* = NULL*/)
{
	return createUser(a_szUser, "localhost", (a_pszPasswd)? a_pszPasswd : "");
}

/**
	mysql > grant all privileges on test_db.* to testuser@localhost IDENTIFIED BY 'password';
 */
int CMySqlUtil::grantPrivileges(const char a_szPrivileges[], const char a_szDbName[],
		const char a_szUser[], const char a_szHost[], const char a_szPasswd[])
{
	char cmd[256];
	::snprintf(cmd, sizeof(cmd), "grant %s on %s.* to %s@%s IDENTIFIED BY \'%s\';",
			a_szPrivileges, a_szDbName, a_szUser, a_szHost, a_szPasswd);
	return query(cmd);
}

/**
	mysql > grant all privileges on test_db.* to testuser@localhost IDENTIFIED BY 'password';
 */
int CMySqlUtil::grantPrivileges(const char a_szPrivileges[], const char a_szDbName[],
		const char a_szUser[], const char *a_pszPasswd/* = NULL*/)
{
	return grantPrivileges(a_szPrivileges, a_szDbName, a_szUser, "localhost",
			(a_pszPasswd)? a_pszPasswd : "");
}

/**
	mysql > grant all privileges on test_db.* to testuser@localhost IDENTIFIED BY 'password';
 */
int CMySqlUtil::grantAllPrivileges(const char a_szDbName[], const char a_szUser[],
		const char a_szHost[], const char a_szPasswd[])
{
	return grantPrivileges("all privileges", a_szDbName, a_szUser, "localhost",
			a_szPasswd);
}

/**
	mysql > grant all privileges on test_db.* to testuser@localhost IDENTIFIED BY 'password';
 */
int CMySqlUtil::grantAllPrivileges(const char a_szDbName[], const char a_szUser[],
		const char *a_pszPasswd/* = NULL*/)
{
	return grantAllPrivileges(a_szDbName, a_szUser, "localhost",
			(a_pszPasswd)? a_pszPasswd : "");
}

/**
	mysql > set password for 'testuser'@'localhost' = password('hogehoge123');
 */
int CMySqlUtil::setPassword(const char a_szUser[], const char a_szHost[],
		const char a_szPasswd[])
{
	char cmd[256];
	::snprintf(cmd, sizeof(cmd), "set password for %s@%s = password(\'%s\');",
			a_szUser, a_szHost, a_szPasswd);
	return query(cmd);
}

/**
	mysql > set password for 'testuser'@'localhost' = password('hogehoge123');
 */
int CMySqlUtil::setPassword(const char a_szUser[], const char a_szPasswd[])
{
	return setPassword(a_szUser, "localhost", a_szPasswd);
}

/**
	mysql > set password = password('hogehoge123');
 */
int CMySqlUtil::setPassword(const char a_szPasswd[])
{
	char cmd[256];
	::snprintf(cmd, sizeof(cmd), "set password = password(\'%s\');", a_szPasswd);
	return query(cmd);
}

/**
	mysql > CREATE TABLE [テーブル名] (
	  [フィールド名] [データ型] [オプション]
	) ENGINE=[InnoDB/MyISAM] DEFAULT CHARSET=[文字コード];
 */
int CMySqlUtil::createTable(const char a_szTableName[], const char a_szSchema[])
{
	char cmd[1024];
	::snprintf(cmd, sizeof(cmd), "create table %s %s;", a_szTableName, a_szSchema);
	return query(cmd);
}

/**
	mysql > DROP TABLE [テーブル名]
 */
int CMySqlUtil::dropTable(const char a_szTableName[])
{
	char cmd[256];
	::snprintf(cmd, sizeof(cmd), "drop table %s;", a_szTableName);
	return query(cmd);
}

/**
	mysql > SHOW FULL COLUMNS FROM [テーブル名];
 */
int CMySqlUtil::showDescriptTable(const char a_szTableName[])
{
	return -1;
}

/**
	mysql > INSERT INTO [テーブル名] [フィールド名] VALUES [値]
 */
int CMySqlUtil::insertRecord(const char a_szTableName[], const char a_szFields[],
		const char a_szValues[])
{
	char cmd[512];
	::snprintf(cmd, sizeof(cmd), "insert into %s %s values %s;",
			a_szTableName, a_szFields, a_szValues);
	return query(cmd);
}

/**
	mysql > UPDATE [テーブル名] SET [フィールド名]=[値] [条件式]
 */
int CMySqlUtil::updateRecord(const char a_szTableName[], const char a_szSettings[],
		const char a_szConditions[])
{
	char cmd[512];
	::snprintf(cmd, sizeof(cmd), "update %s set %s %s;",
			a_szTableName, a_szSettings, a_szConditions);
	return query(cmd);
}

/**
	mysql > DELETE FROM [テーブル名]
	mysql > DELETE FROM [テーブル名] WHERE [条件式]
 */
int CMySqlUtil::deleteRecord(const char a_szTableName[],
		const char *a_pszConditions/* = NULL*/)
{
	char cmd[512];
	if (a_pszConditions) {
		::snprintf(cmd, sizeof(cmd), "delete from %s where %s;",
				a_szTableName, a_pszConditions);
	} else {
		::snprintf(cmd, sizeof(cmd), "delete from %s;", a_szTableName);
	}
	return query(cmd);
}

/**
 */
int CMySqlUtilBase::select(const char a_szParams[],
		const int a_nMaxCounts/* = EInfinite*/)
{
	char cmd[1024];
	::strcpy(cmd, "select ");
	::strcat(cmd, a_szParams);

	if (query(cmd)) {
		return 0;
	}

	MYSQL_RES *res = ::mysql_use_result(m_pConn);
	if (!res) {
		return 0;
	}

	int n;
	MYSQL_ROW row;
	for (n = 0; (row = ::mysql_fetch_row(res)) &&
			((a_nMaxCounts <= EInfinite) || (n < a_nMaxCounts)); n++) {
		onResult(n, row);
	}
	mysql_free_result(res);
	return n;
}

/**
 */
CMySqlUtilBase::CMySqlUtilBase(const char a_szDbName[], const char a_szUser[],
			const char *a_pszPasswd)
	:	m_pConn(NULL)
{
	char szSqlServ[] = "localhost";
	if (prepare(a_szDbName, a_szUser, szSqlServ, a_pszPasswd)) {
		assert(! "CMySqlUtilBase::CMySqlUtilBase localhost");
	}
}

/**
 */
CMySqlUtilBase::CMySqlUtilBase(const char a_szDbName[], const char a_szUser[],
		const char a_szHost[], const char *a_pszPasswd)
	:	m_pConn(NULL)
{
	if (prepare(a_szDbName, a_szUser, a_szHost, a_pszPasswd)) {
		assert(! "CMySqlUtilBase::CMySqlUtilBase");
	}
}

/**
 */
CMySqlUtilBase::~CMySqlUtilBase()
{
	if (m_pConn) {
		::mysql_close(m_pConn);
	}
}

/**
 */
int CMySqlUtilBase::prepare(const char a_szDbName[], const char a_szUser[],
			const char a_szHost[], const char *a_pszPasswd)
{
	::memset(m_szDbName, 0, sizeof(m_szDbName));
	::memset(m_szUser, 0, sizeof(m_szUser));
	::memset(m_szPasswd, 0, sizeof(m_szPasswd));
	::memset(m_szHost, 0, sizeof(m_szHost));

	::strncpy(m_szDbName, a_szDbName, sizeof(m_szDbName));
	::strncpy(m_szUser, a_szUser, sizeof(m_szUser));
	::strncpy(m_szHost, a_szHost, sizeof(m_szHost));
	if (a_pszPasswd) {
		::strncpy(m_szPasswd, a_pszPasswd, sizeof(m_szPasswd));
	}

	m_pConn = ::mysql_init(NULL);
	if (!m_pConn) {
		return -1;
	}

	if (!::mysql_real_connect(m_pConn, m_szHost, m_szUser, m_szPasswd,
			m_szDbName, 0, NULL, 0)) {
		::mysql_close(m_pConn);
		m_pConn = NULL;
		return -1;
	}
	return 0;
}

/**
 */
int CMySqlUtilBase::query(const char a_szSqlCmd[])
{
	return ::mysql_query(m_pConn, a_szSqlCmd);
}

/**
 */
void CMySqlUtilBase::onResult(int a_nIndex, MYSQL_ROW& a_objRow)
{
}
