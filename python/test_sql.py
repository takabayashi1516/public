import argparse
import logging
from dataclasses import dataclass

from sql.sqlutil import (
    Constants as SqlUtilConstants,
    PostgreSqlUtil,
    MySqlUtil,
    OracleSqlUtil
  )

@dataclass(frozen = True)
class Constants:
  DB_ENGINE_MYSQL: int = 0
  DB_ENGINE_POSTGRESQL: int = 1
  DB_ENGINE_ORACLESQL: int = 2

class CustomPostgreSqlUtil(PostgreSqlUtil):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = SqlUtilConstants.DEFULT_PORT_POSTGRESQL):
    super().__init__(host, user, password, database,
        notify_param, port)
    self.descs = {}
    self.rows = {}

  def notify(self, e, description, notify_param):
    for desc in description:
      pass
    self.descs[notify_param] = description
    self.rows[notify_param] = self.fetchall()
    self.set_notify_pram(notify_param + 1)

class CustomMySqlUtil(MySqlUtil):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = SqlUtilConstants.DEFULT_PORT_MYSQL):
    super().__init__(host, user, password, database,
        notify_param, port)
    self.descs = {}
    self.rows = {}

  def notify(self, e, description, notify_param):
    for desc in description:
      pass
    self.descs[notify_param] = description
    self.rows[notify_param] = self.fetchall()
    self.set_notify_pram(notify_param + 1)

class CustomOracleSqlUtil(OracleSqlUtil):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = SqlUtilConstants.DEFULT_PORT_ORACLESQL):
    super().__init__(host, user, password, database,
        notify_param, port)
    self.descs = {}
    self.rows = {}

  def notify(self, e, description, notify_param):
    for desc in description:
      pass
    self.descs[notify_param] = description
    self.rows[notify_param] = self.fetchall()
    self.set_notify_pram(notify_param + 1)

def main():
  logging.basicConfig(level = logging.ERROR)
  logger = logging.getLogger(name = __name__)

  parser = argparse.ArgumentParser(description = "")
  parser.add_argument("--host", default = 'localhost')
  parser.add_argument("--user", default = 'root')
  parser.add_argument("--password")
  parser.add_argument("--database")
  parser.add_argument("--port", type = int, default = 0)
  parser.add_argument("--query")
  parser.add_argument("--sql")
  parser.add_argument("--hide", action = 'store_true', default = False)
  parser.add_argument("--engine", type = int, default = Constants.DB_ENGINE_MYSQL)
  args = parser.parse_args()

  sqlutl = None
  port = args.port
  pram = 0

  if args.engine == Constants.DB_ENGINE_MYSQL:
    if port == 0:
      port = SqlUtilConstants.DEFULT_PORT_MYSQL
    sqlutl = CustomMySqlUtil(host = args.host,
        user = args.user,
        password = args.password,
        database = args.database,
        notify_param = pram,
        port = port)
  elif args.engine == Constants.DB_ENGINE_POSTGRESQL:
    if port == 0:
      port = SqlUtilConstants.DEFULT_PORT_POSTGRESQL
    sqlutl = CustomPostgreSqlUtil(host = args.host,
        user = args.user,
        password = args.password,
        database = args.database,
        notify_param = pram,
        port = port)
  elif args.engine == Constants.DB_ENGINE_ORACLESQL:
    if port == 0:
      port = SqlUtilConstants.DEFULT_PORT_ORACLESQL
    sqlutl = CustomOracleSqlUtil(host = args.host,
        user = args.user,
        password = args.password,
        database = args.database,
        notify_param = pram,
        port = port)

  try:
    sqlutl.connect()
  except Exception as e:
    logger.error(e)
    return

  if args.sql:
    sqlutl.executeSql(args.sql, True)

  if args.query:
    sqlutl.execute(args.query, False)

  if not args.hide:
    rows = sqlutl.rows
    descs = sqlutl.descs
    for key in rows:
      for desc in descs[key]:
        print("{}, ".format(desc[0]), end = "")
      print("", end = "\n")
      for row in rows[key]:
        for cell in row:
          print("{}, ".format(cell), end = "")
        print("", end = "\n")

if __name__ == "__main__":
  main()

