#ifndef __LIBMYSQL_UTIL_H
#define __LIBMYSQL_UTIL_H

#include <mysql/mysql.h>

#ifdef __cplusplus

/**
 */
class CMySqlUtilBase {
public:
	enum {
		EInfinite = 0
	};

private:
	///
	MYSQL	*m_pConn;
	///
	char	m_szDbName[32];
	///
	char	m_szUser[32];
	///
	char	m_szPasswd[32];
	///
	char	m_szHost[256];

public:
	///
	int select(const char a_szParams[], const int a_nMaxCounts = EInfinite);

protected:
	///
	CMySqlUtilBase(const char a_szDbName[], const char a_szUser[],
			const char a_szHost[], const char *a_pszPasswd);
	///
	CMySqlUtilBase(const char a_szDbName[], const char a_szUser[],
			const char *a_pszPasswd);
	///
	virtual ~CMySqlUtilBase();

	///
	virtual int prepare(const char a_szDbName[], const char a_szUser[],
			const char a_szHost[], const char *a_pszPasswd);
	///
	int query(const char a_szSqlCmd[]);

	///
	virtual void onResult(int a_nIndex, MYSQL_ROW& a_objRow);
};

/**
 */
class CMySqlUtil : public CMySqlUtilBase {
public:
	///
	CMySqlUtil(const char a_szDbName[], const char a_szUser[],
			const char a_szHost[], const char a_szPasswd[]);
	///
	CMySqlUtil(const char a_szDbName[], const char a_szUser[],
			const char *a_pszPasswd = NULL);
	///
	virtual ~CMySqlUtil();

	///
	int createUser(const char a_szUser[], const char a_szHost[],
			const char a_szPasswd[]);
	///
	int createUser(const char a_szUser[], const char *a_pszPasswd = NULL);
	///
	int grantPrivileges(const char a_szPrivileges[], const char a_szDbName[],
			const char a_szUser[], const char a_szHost[],
			const char a_szPasswd[]);
	///
	int grantPrivileges(const char a_szPrivileges[], const char a_szDbName[],
			const char a_szUser[], const char *a_pszPasswd = NULL);
	///
	int grantAllPrivileges(const char a_szDbName[], const char a_szUser[],
			const char a_szHost[], const char a_szPasswd[]);
	///
	int grantAllPrivileges(const char a_szDbName[], const char a_szUser[],
			const char *a_pszPasswd = NULL);
	///
	int setPassword(const char a_szUser[], const char a_szHost[],
			const char a_szPasswd[]);
	///
	int setPassword(const char a_szUser[], const char a_szPasswd[]);
	///
	int setPassword(const char a_szPasswd[]);
	///
	int createTable(const char a_szTableName[], const char a_szSchema[]);
	///
	int dropTable(const char a_szTableName[]);
	///
	int showDescriptTable(const char a_szTableName[]);
	///
	int insertRecord(const char a_szTableName[], const char a_szFields[],
			const char a_szValues[]);
	///
	int updateRecord(const char a_szTableName[], const char a_szSettings[],
			const char a_szConditions[]);
	///
	int deleteRecord(const char a_szTableName[],
			const char *a_pszConditions = NULL);
};

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __LIBMYSQL_UTIL_H */
