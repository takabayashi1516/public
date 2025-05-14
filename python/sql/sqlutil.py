'''
$ pip install psycopg2-binary
$ pip install mysql-connector-python
$ pip install pymysql
$ pip install cx_Oracle
$ pip install sqlparse
'''

'''
直接接続の例
  $ python -c "import sqlutil; sqlutl=sqlutil.PostgreSqlUtil(host=${DB_HOST},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}',is_commit=False); rows=sqlutl.fetchall(); print(rows)"
  $ python -c "import sqlutil; sqlutl=sqlutil.MySqlUtil(host=${DB_HOST},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}',is_commit=False); rows=sqlutl.fetchall(); print(rows)"
  $ python -c "import sqlutil; sqlutl=sqlutil.OracleSqlUtil(host=${DB_HOST},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}',is_commit=False); rows=sqlutl.fetchall(); print(rows)"

sshトンネリングを使用する場合の例
  $ ssh -N -L ${DB_LOCAL_PORT}:${DB_HOST}:${DB_PORT} ${SSH_USER}@${SSH_HOST} -i $id_rsa

  $ python -c "import sqlutil; sqlutl=sqlutil.PostgreSqlUtil(host=localhost,port=${DB_LOCAL_PORT},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}',is_commit=False); rows=sqlutl.fetchall(); print(rows)"
  $ python -c "import sqlutil; sqlutl=sqlutil.MySqlUtil(host=localhost,port=${DB_LOCAL_PORT},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}',is_commit=False); rows=sqlutl.fetchall(); print(rows)"
  $ python -c "import sqlutil; sqlutl=sqlutil.OracleSqlUtil(host=localhost,port=${DB_LOCAL_PORT},user=${USER},password=${PASSWORD},database=${DATABASE}); sqlutl.connect(); sqlutl.execute('select * from ${TABLE}',is_commit=False); rows=sqlutl.fetchall(); print(rows)"
'''

import cx_Oracle
import mysql.connector
import os
import psycopg2
import pymysql
import re
import sqlparse
from dataclasses import dataclass
from logging import getLogger

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
    self.logger = getLogger(name = __name__)
    self.logger.debug(f"__init__({host}, {user}, {password}, {database}, {notify_param}, {port})")
    self.host = host
    self.user = user
    self.password = password
    self.database = database
    self.port = port
    self.notify_param = notify_param
    self.conn = None
    self.cur = None

  def __del__(self):
    self.logger.debug('__del__()')
    self.disconnect()
    self.notify_param = None
    self.logger = None

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
      q1 = re.sub('^((\t)|( ))*\-\- +.*$', '', q)
      if q1:
        if query1:
          query1 += ' '
        query1 += q1
    #query1 = re.sub('/\*.*\*/', '', query1)
    query1 = re.sub('^ +', '', query1)
    return query1

  def execute1(self, sql: str, params: [] = None, is_commit: bool = True):
    try:
      self.logger.debug(f"execute1: {sql}, {params}")
      if (params):
        self.cur.execute(sql, params)
      else:
        self.cur.execute(sql)
      self.logger.info(f"execute1: {sql}, {params}")
      if is_commit:
        self.logger.debug(f"commit:{sql}")
        self.conn.commit()
        self.logger.info(f"committed")
    except Exception as e:
      if is_commit:
        self.logger.debug(f"rollback:{sql}")
        self.conn.rollback()
        self.logger.info(f"rolled back")
      raise e

  def execute(self, sql: str, is_commit: bool = True):
    if not self.cur:
      raise Exception(Constants.EXCPT_SQLUTIL_EXEC_FAILED.format(self))
      return

    try:
      queries = sqlparse.split(sql)
    except Exception as e:
      self.logger.error(sql)
      raise e

    for query in queries:
      query1 = self.__exclude_comments(query)
      if not query1:
        continue
      try:
        self.logger.debug(f"execute: {query1}")
        self.execute1(sql = query1, is_commit = False)
        self.logger.info(f"executed: {query1}")
        try:
          self.notify(None, self.cur.description,
              self.notify_param)
        except Exception as e1:
          #pass
          self.logger.debug(f"desc:{self.cur.description}, param:{self.notify_param}")
      except Exception as e:
        self.logger.error(f"error: {query1}")
        try:
          self.notify(e, self.cur.description,
              self.notify_param)
        except Exception as e1:
          #pass
          self.logger.debug(f"desc:{self.cur.description}, param:{self.notify_param}")
        if is_commit:
          self.logger.debug(f"rollback:{sql}")
          self.conn.rollback()
          self.logger.info(f"rolled back")
        raise e
    if is_commit:
      self.logger.debug(f"commit:{sql}")
      self.conn.commit()
      self.logger.info(f"committed")

  def executeSql(self, sql_file: str, is_commit: bool = True):
    if not os.path.exists(sql_file):
      raise Exception(Constants.EXCPT_SQLUTIL_EXEC_FILE_NOT_EXIST
          .format(sql_file, self))
    with open(sql_file, 'r', encoding = 'utf-8') as f:
      sql = f.read()
      self.execute(sql = sql, is_commit = is_commit)

  def notify(self, err: Exception, description, notify_param):
    return

  def fetchall(self):
    if self.cur:
      self.logger.debug(self.cur.description)
      return self.cur.fetchall()
    raise Exception(Constants.EXCPT_SQLUTIL_CONN_NOT_EXIST.format(self))
    return None

'''
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
'''
class OracleSqlUtil(SqlUtilBase):
  def __init__(self, host: str, user: str, password: str,
      service_name: str, notify_param = None,
      port: int = Constants.DEFULT_PORT_ORACLESQL):
    super().__init__(host = host, user = user, password = password,
        database = service_name, notify_param = notify_param, port = port)

  def connect(self):
    super().disconnect()
    self.conn = cx_Oracle.connect(
      user = self.user,
      password = self.password,
      dsn = self.host + ":" + str(self.port) + "/" + self.database)
    self.cur = self.conn.cursor()

