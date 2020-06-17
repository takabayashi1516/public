
/**
 *
 * pre-condition
 *  > create database test_db;
 *
 * root password: aaa
 *  # mysql_secure_installation
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libmysql_util.h>


class CTestMySqlUtil : public CMySqlUtil {
public:
	class CRecord {
	public:
		char	m_szName[128];
		char	m_szMail[128];
		char	m_szDate[128];
		char	m_szAge[16];
	public:
		CRecord() {
			::memset(m_szName, 0, sizeof(m_szName));
			::memset(m_szMail, 0, sizeof(m_szMail));
			::memset(m_szDate, 0, sizeof(m_szDate));
			::memset(m_szAge,  0, sizeof(m_szAge));
		}
	};
private:
	CRecord		*m_pobjRecord;
	int			m_nRecords;
public:
	CTestMySqlUtil() : CMySqlUtil("test_db", "root", "aaa"),
			m_pobjRecord(NULL), m_nRecords(0) {}
	~CTestMySqlUtil() {
		if (m_pobjRecord) {
			delete [] m_pobjRecord;
		}
	}
	int getRecords(CRecord*& a_pobjRecord) {
		a_pobjRecord = m_pobjRecord;
		return m_nRecords;
	}
	void clearRecords() {
		m_nRecords = 0;
		if (m_pobjRecord) {
			delete [] m_pobjRecord;
			m_pobjRecord = NULL;
		}
	}
protected:
	virtual void onResult(int a_nIndex, MYSQL_ROW& a_objRow) {
//		::printf("l(%4d): %s: %s\n", __LINE__, __FUNCTION__, a_objRow[0]);
		CRecord *rec;
		if (m_pobjRecord) {
			CRecord *tmp = m_pobjRecord;
			m_pobjRecord = new CRecord[++m_nRecords];
			for (int i = 0; i < (m_nRecords - 1); i++) {
				m_pobjRecord[i] = tmp[i];
			}
			delete [] tmp;
		} else {
			m_pobjRecord = new CRecord[++m_nRecords];
		}
		rec = &m_pobjRecord[m_nRecords - 1];
		::strcpy(rec->m_szName, (a_objRow[1])? a_objRow[1] : "");
		::strcpy(rec->m_szMail, (a_objRow[2])? a_objRow[2] : "");
		::strcpy(rec->m_szDate, (a_objRow[3])? a_objRow[3] : "");
		::strcpy(rec->m_szAge,  (a_objRow[4])? a_objRow[4] : "");
	}
};

class CTest2MySqlUtil : public CMySqlUtil {
public:
	class CRecord {
	public:
		char	m_szName[128];
		char	m_szData[4096];
	public:
		CRecord() {
			::memset(m_szName, 0, sizeof(m_szName));
			::memset(m_szData, 0, sizeof(m_szData));
		}
	};
private:
	CRecord		*m_pobjRecord;
	int			m_nRecords;
public:
	CTest2MySqlUtil() : CMySqlUtil("test_db", "root", "aaa"),
			m_pobjRecord(NULL), m_nRecords(0) {}
	~CTest2MySqlUtil() {
		if (m_pobjRecord) {
			delete [] m_pobjRecord;
		}
	}
	int getRecords(CRecord*& a_pobjRecord) {
		a_pobjRecord = m_pobjRecord;
		return m_nRecords;
	}
	void clearRecords() {
		m_nRecords = 0;
		if (m_pobjRecord) {
			delete [] m_pobjRecord;
			m_pobjRecord = NULL;
		}
	}
protected:
	virtual void onResult(int a_nIndex, MYSQL_ROW& a_objRow) {
		::printf("l(%4d): %s: %s\n", __LINE__, __FUNCTION__, a_objRow[0]);
		CRecord *rec;
		if (m_pobjRecord) {
			CRecord *tmp = m_pobjRecord;
			m_pobjRecord = new CRecord[++m_nRecords];
			for (int i = 0; i < (m_nRecords - 1); i++) {
				m_pobjRecord[i] = tmp[i];
			}
			delete [] tmp;
		} else {
			m_pobjRecord = new CRecord[++m_nRecords];
		}
		rec = &m_pobjRecord[m_nRecords - 1];
		::strcpy(rec->m_szName, (a_objRow[1])? a_objRow[1] : "");
		::strcpy(rec->m_szData, (a_objRow[2])? a_objRow[2] : "");
	}
};

int main(int argc, char *argv[])
{
	int rc, n;

#if 1

	CTestMySqlUtil sql;

	rc = sql.createTable("test_table", "(id int not null auto_increment primary key, name text  not null, mail text, upd datetime, age int) default charset=utf8");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
	rc = sql.insertRecord("test_table", "(name, mail, age)", "(\"sss.ttt\", \"ttt.999@gmail.com\", 33)");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
	rc = sql.insertRecord("test_table", "(name, mail, age, upd)", "(\"mmm.ttt\", \"h.999@gmail.com\", 27, now())");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
	rc = sql.insertRecord("test_table", "(name, mail, upd)", "(\"hhh.ttt\", \"h2.ttt.999@gmail.com\", now())");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
	rc = sql.insertRecord("test_table", "(name, mail, age)", "(\"sss2.ttt\", \"s.ttt.999@gmail.com\", 7)");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	n = sql.select("* from test_table where name = \"sss.ttt\"");
	::printf("l(%4d): %s: n=%d\n", __LINE__, __FUNCTION__, n);
	CTestMySqlUtil::CRecord *rec;
	n = sql.getRecords(rec);
	for (int i = 0; i < n; i++) {
		::printf("%d %s, %s, %s, %s\n", i, rec[i].m_szName, rec[i].m_szMail, rec[i].m_szDate, rec[i].m_szAge);
	}
	sql.clearRecords();

	n = sql.select("* from test_table", 2);
	::printf("l(%4d): %s: n=%d\n", __LINE__, __FUNCTION__, n);
	n = sql.getRecords(rec);
	for (int i = 0; i < n; i++) {
		::printf("%d %s, %s, %s, %s\n", i, rec[i].m_szName, rec[i].m_szMail, rec[i].m_szDate, rec[i].m_szAge);
	}
	sql.clearRecords();

	rc = sql.updateRecord("test_table", "name=\"sss.ttt\", upd=now()",
			"where age = 45");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
	n = sql.select("* from test_table");
	::printf("l(%4d): %s: n=%d\n", __LINE__, __FUNCTION__, n);
	n = sql.getRecords(rec);
	for (int i = 0; i < n; i++) {
		::printf("%d %s, %s, %s, %s\n", i, rec[i].m_szName, rec[i].m_szMail, rec[i].m_szDate, rec[i].m_szAge);
	}
	n = sql.getRecords(rec);

	rc = sql.deleteRecord("test_table");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
	rc = sql.dropTable("test_table");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

#else

	CTest2MySqlUtil sql2;


	rc = sql2.createTable("test2_table", "(id int not null auto_increment primary key, name text, data VARBINARY(65500)) default charset=utf8");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	char name[] = "00:00:00:00:00:00:00:00";
	uint8_t buff[100];
	::memset(buff, 0xa5, sizeof(buff));
	buff[sizeof(buff) - 1] = 0u;
	char cmd_data[sizeof(buff) + 128];
	::sprintf(cmd_data, "(\"%s\", hex(\"%s\"))", name, buff);
//	::printf("%s\n", cmd_data);

	rc = sql2.insertRecord("test2_table", "(name, data)", cmd_data);
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

	n = sql2.select("* from test2_table");
	::printf("l(%4d): %s: n=%d\n", __LINE__, __FUNCTION__, n);

	CTest2MySqlUtil::CRecord *rec2;
	n = sql2.getRecords(rec2);
	::printf("l(%4d): %s: n=%d\n", __LINE__, __FUNCTION__, n);
	for (int i = 0; i < n; i++) {
		::printf("%d %s, %s\n", i, rec2[i].m_szName, rec2[i].m_szData);
	}
	n = sql2.getRecords(rec2);

	rc = sql2.deleteRecord("test2_table");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);
	rc = sql2.dropTable("test2_table");
	::printf("l(%4d): %s: rc=%d\n", __LINE__, __FUNCTION__, rc);

#endif

	return 0;
}
