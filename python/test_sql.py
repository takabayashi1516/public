import argparse
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

def main():
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
  if args.engine == Constants.DB_ENGINE_MYSQL:
    if port == 0:
      port = SqlUtilConstants.DEFULT_PORT_MYSQL
    sqlutl = MySqlUtil(host = args.host,
        user = args.user,
        password = args.password,
        database = args.database,
        port = port)
  elif args.engine == Constants.DB_ENGINE_POSTGRESQL:
    if port == 0:
      port = SqlUtilConstants.DEFULT_PORT_POSTGRESQL
    sqlutl = PostgreSqlUtil(host = args.host,
        user = args.user,
        password = args.password,
        database = args.database,
        port = port)
  elif args.engine == Constants.DB_ENGINE_ORACLESQL:
    if port == 0:
      port = SqlUtilConstants.DEFULT_PORT_ORACLESQL
    sqlutl = OracleSqlUtil(host = args.host,
        user = args.user,
        password = args.password,
        database = args.database,
        port = port)

  sqlutl.connect()

  if args.sql:
    sqlutl.executeSql(args.sql, True)

  sqlutl.execute(args.query, False)
  if not args.hide:
    rows = sqlutl.fetchall()
    for row in rows:
      print(row)

if __name__ == "__main__":
  main()

