'''
$ pip install psycopg2-binary
$ pip install mysql-connector-python
$ pip install pymysql
$ pip install cx_Oracle
$ pip install sqlparse
'''

import cx_Oracle
import mysql.connector
import os
import psycopg2
import pymysql
import re
import sqlparse
from dataclasses import dataclass

@dataclass(frozen = True)
class Constants:
  DEFULT_PORT_POSTGRESQL: int = 5432
  DEFULT_PORT_MYSQL: int = 3306
  DEFULT_PORT_ORACLESQL: int = 1521
  EXCPT_SQLUTIL_CONN_DENY: str = 'deny to call base method: {}'
  EXCPT_SQLUTIL_CONN_NOT_EXIST: str = 'no exist conn: {}'
  EXCPT_SQLUTIL_EXEC_FAILED: str = 'execute failed: {}'
  EXCPT_SQLUTIL_EXEC_FILE_NOT_EXIST: str = 'execute script file {} not exist: {}'

class SqlUtilBase:
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param, port: int):
    self.host = host
    self.user = user
    self.password = password
    self.database = database
    self.port = port
    self.notify_param = notify_param
    self.conn = None
    self.cur = None

  def __del__(self):
    self.disconnect()

  def connect(self):
    raise Exception(Constants.EXCPT_SQLUTIL_CONN_DENY.format(self))

  def disconnect(self):
    if self.cur:
      self.cur.close()
      self.cur = None
    if self.conn:
      self.conn.close()
      self.conn = None

  def set_notify_pram(self, notify_param):
    self.notify_param = notify_param

  def __exclude_comments(self, query: str) -> str:
    query1 = query.replace('\r\n', '\n').replace('\r', '\n')
    queries = query1.split('\n')
    query1 = ''
    for q in queries:
      q1 = re.sub('^\-\- +.+$', '', q)
      query1 += q1
    query1 = query1.replace('\n', '')
    return query1

  def execute(self, sql: str, is_commit: bool = True):
    if self.cur:
      queries = sqlparse.split(sql)
      for query in queries:
        query1 = self.__exclude_comments(query)
        if not query1:
          continue
        try:
          #print(f"--- {query1}")
          self.cur.execute(query1)
          try:
            self.notify(self.cur.description,
                self.notify_param)
          except Exception as e1:
            pass
        except Exception as e:
          if is_commit:
            self.conn.rollback()
          raise e
      if is_commit:
        #print("--- commit")
        self.conn.commit()
    else:
      raise Exception(Constants.EXCPT_SQLUTIL_EXEC_FAILED.format(self))

  def executeSql(self, sql_file: str, is_commit: bool = True):
    if not os.path.exists(sql_file):
      raise Exception(Constants.EXCPT_SQLUTIL_EXEC_FILE_NOT_EXIST
          .format(sql_file, self))
    with open(sql_file, 'r', encoding = 'utf-8') as f:
      sql = f.read()
      self.execute(sql = sql, is_commit = is_commit)

  def notify(self, description, notify_param):
    return

  def fetchall(self):
    if self.cur:
      #print(self.cur.description)
      return self.cur.fetchall()
    raise Exception(Constants.EXCPT_SQLUTIL_CONN_NOT_EXIST.format(self))
    return None

'''
$ python -c "import sqlutil; sqlutl=sqlutil.PostgreSqlUtil(host=${HOST},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}'); rows=sqlutl.fetchall(); print(rows)"
'''
class PostgreSqlUtil(SqlUtilBase):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = Constants.DEFULT_PORT_POSTGRESQL):
    super().__init__(host, user, password, database,
        notify_param, port)

  def connect(self):
    super().disconnect()
    self.conn = psycopg2.connect(
      dbname = self.database,
      user = self.user,
      password = self.password,
      host = self.host,
      port = self.port)
    self.cur = self.conn.cursor()

'''
$ python -c "import sqlutil; sqlutl=sqlutil.MySqlUtil(host=${HOST},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}'); rows=sqlutl.fetchall(); print(rows)"
'''
class MySqlUtil(SqlUtilBase):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = Constants.DEFULT_PORT_MYSQL):
    super().__init__(host, user, password, database,
        notify_param, port)

  def connect(self):
    super().disconnect()
    self.conn = mysql.connector.connect(
      host = self.host,
      user = self.user,
      password = self.password,
      database = self.database)
    self.cur = self.conn.cursor()

'''
$ python -c "import sqlutil; sqlutl=sqlutil.OracleSqlUtil(host=${HOST},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}'); rows=sqlutl.fetchall(); print(rows)"
'''
class OracleSqlUtil(SqlUtilBase):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = Constants.DEFULT_PORT_ORACLESQL):
    super().__init__(host, user, password, database,
        notify_param, port)

  def connect(self):
    super().disconnect()
    self.conn = pymysql.connect(
      host = self.host,
      user = self.user,
      password = self.password,
      database = self.database)
    self.cur = self.conn.cursor()

