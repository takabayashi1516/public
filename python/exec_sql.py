import argparse
import logging
import json5
from dataclasses import dataclass, field
from sql.sqlutil import (
    Constants as SqlUtilConstants,
    PostgreSqlUtil,
    MySqlUtil,
    OracleSqlUtil
  )
from proc.prcctrl import ( ProcessController )

class CustomPostgreSqlUtil(PostgreSqlUtil):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = SqlUtilConstants.DEFULT_PORT_POSTGRESQL):
    super().__init__(host, user, password, database,
        notify_param, port)
    self.descs = {}
    self.rows = {}

  def notify(self, e, description: tuple, datas: tuple, notify_param):
    for desc in description:
      pass
    self.descs[notify_param] = description
    self.rows[notify_param] = datas
    self.set_notify_pram(notify_param + 1)

class CustomMySqlUtil(MySqlUtil):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = SqlUtilConstants.DEFULT_PORT_MYSQL):
    super().__init__(host, user, password, database,
        notify_param, port)
    self.descs = {}
    self.rows = {}

  def notify(self, e, description: tuple, datas: tuple, notify_param):
    for desc in description:
      pass
    self.descs[notify_param] = description
    self.rows[notify_param] = datas
    self.set_notify_pram(notify_param + 1)

class CustomOracleSqlUtil(OracleSqlUtil):
  def __init__(self, host: str, user: str, password: str,
      database: str, notify_param = None,
      port: int = SqlUtilConstants.DEFULT_PORT_ORACLESQL):
    super().__init__(host, user, password, database,
        notify_param, port)
    self.descs = {}
    self.rows = {}

  def notify(self, e, description: tuple, datas: tuple, notify_param):
    for desc in description:
      pass
    self.descs[notify_param] = description
    self.rows[notify_param] = datas
    self.set_notify_pram(notify_param + 1)

@dataclass(frozen = True)
class Constants:
  '''
  DB_ENGINE_MYSQL: int = 0
  DB_ENGINE_POSTGRESQL: int = 1
  DB_ENGINE_ORACLESQL: int = 2
  '''

  LOG_LEVELS = {
      0: logging.CRITICAL,
      1: logging.ERROR,
      2: logging.WARNING,
      3: logging.INFO,
      4: logging.DEBUG,
      5: logging.NOTSET,
    }

  CLASSES = (
      CustomMySqlUtil,
      CustomPostgreSqlUtil,
      CustomOracleSqlUtil,
    )

  DEFAULT_PORTS = (
      SqlUtilConstants.DEFULT_PORT_MYSQL,
      SqlUtilConstants.DEFULT_PORT_POSTGRESQL,
      SqlUtilConstants.DEFULT_PORT_ORACLESQL,
    )

def main():
  parser = argparse.ArgumentParser(description = "")
  parser.add_argument("--config", default = 'config.json')
  parser.add_argument("--query")
  parser.add_argument("--sql")
  parser.add_argument("--commit", action = 'store_true', default = False)
  parser.add_argument("--log", type = int, default = 0)
  args = parser.parse_args()

  logging.basicConfig(level = Constants.LOG_LEVELS[args.log])
  logger = logging.getLogger(name = __name__)

  try:
    with open(args.config, 'r', encoding = 'utf-8') as f:
      config = json5.load(f)
  except Exception as e:
    logger.error(e)
    return

  sqlutl = None
  port = config['port']
  pram = 0

  cls = Constants.CLASSES[config['engine']]

  if port == 0:
    port = Constants.DEFAULT_PORTS[config['engine']]

  host = config['host']
  user = config['user']

  proc_ssh = None

  if 'ssh' in config:
    cmd = [
        'ssh', '-N', '-L',
        f"{port}:{config['ssh']['host']}:{config['ssh']['port']}",
        f"{user}@{host}",
      ]
    if 'privateKeyPath' in config['ssh']:
      cmd.append('-i').append(config['ssh']['privateKeyPath'])
    proc_ssh = ProcessController(cmd)
    proc_ssh.start()

  sqlutl = cls(host = host,
      user = user,
      password = config['password'],
      database = config['database'],
      notify_param = pram,
      port = port)

  try:
    sqlutl.connect()
  except Exception as e:
    logger.error(e)
    if proc_ssh:
      proc_ssh.stop()
    return

  if args.sql:
    sqlutl.executeSql(args.sql, args.commit)

  if args.query:
    sqlutl.execute(args.query, args.commit)

  if proc_ssh:
    proc_ssh.stop()

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

