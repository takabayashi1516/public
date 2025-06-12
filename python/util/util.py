
import asyncio
import glob
import json
import random
import re
import os
import socket
import sys
import time
import pandas as pd

'''
$ python -c "from util.util import ( Util ); print(Util.basename('$(pwd)'))"
$ python -c "from util.util import ( Util ); print(Util.dirname('$(pwd)'))"

$ python -c "from util.util import ( Util ); print(Util.searchFiles('.', '*.py', recursive = True))"

$ python -c "from util.util import ( Util ); Util.mkDirectory('./a/b/c/d/')"
$ python -c "from util.util import ( Util ); print(Util.isDirectory('.'))"

$ python -c "from util.util import ( Util ); print(Util.jsonarr2tbl('./util/test/data.json'))"
$ python -c "from util.util import ( Util ); print(Util.jsonarr2tbl('./util/test/data.json', './out.xlsx'))"
$ python -c "from util.util import ( Util ); print(Util.jsonarr2tbl('./util/test/data.json', './out.tsv', sep = '\t'))"

& python -c "from util.util import ( Util ); print(Util.containf('./util/test/a.json', './util/test/b.json', extraction_diff = True))"
& python -c "from util.util import ( Util ); print(Util.containf('./util/test/a.json', './util/test/b.json', extraction_diff = False))"
'''

class Util:

  @staticmethod
  def basename(name):
    return os.path.basename(name)

  @staticmethod
  def dirname(name):
    return os.path.dirname(name)

  @staticmethod
  def searchFiles(location, pattern, recursive):
    conn = '/**/' if recursive else '/'
    return glob.glob(f"{location}{conn}{pattern}", recursive = recursive)

  @staticmethod
  def mkDirectory(name, exist_ok = True):
    os.makedirs(name, exist_ok = exist_ok)

  @staticmethod
  def isDirectory(name):
    if os.path.exists(name):
      if os.path.isdir(name):
        return True
      return False
    return None

  @staticmethod
  def sleep(delayMs):
    return asyncio.sleep(delayMs / 1000)

  @staticmethod
  def containf(json_a, json_b, extraction_diff = True):
    with open(json_a, "r", encoding = "utf-8") as f:
      a = json.load(f)
    with open(json_b, "r", encoding = "utf-8") as f:
      b = json.load(f)
    return Util.contain(a, b, extraction_diff)

  @staticmethod
  def contain(a, b, extraction_diff = True):
    # list case
    if isinstance(a, list):
      if not isinstance(b, list):
        return a if extraction_diff else None

      extraction_array = []
      for idx in range(len(a)):
        b_elem = b[idx] if idx < len(b) else None
        extraction = Util.contain(a[idx], b_elem, extraction_diff)
        if extraction is not None:
          extraction_array.append(extraction)
      return extraction_array if extraction_array else None

    # dict case
    if isinstance(a, dict):
      if not isinstance(b, dict):
        return a if extraction_diff else None

      extraction_object = {}
      for k in a:
        if k not in b:
          if extraction_diff:
            extraction_object[k] = a[k]
        else:
          extraction = Util.contain(a[k], b[k], extraction_diff)
          if extraction is not None:
            extraction_object[k] = extraction

      return extraction_object if extraction_object else None

    # primitive case
    if extraction_diff:
      return a if a != b else None
    return a if a == b else None

  # 期待データではなかった
  '''
  @staticmethod
  def jsonarr2tbl_lib(inp_json_f, outp_xls_f = None):
    df = pd.read_json(inp_json_f, orient = "records")
    if outp_xls_f:
      df.to_excel(outp_xls_f, index = False, engine = 'openpyxl')
    return df
  '''

  @staticmethod
  def jsonarr2tbl(inp_json_f, outp_xls_f = None, sep = None):
    with open(inp_json_f, "r", encoding = "utf-8") as f:
      data = json.load(f)
    df = Util.objectarray_to_dataframe(data)
    if outp_xls_f:
      if not sep:
        df.to_excel(outp_xls_f, index = False, engine = 'openpyxl')
      else:
        df.to_csv(outp_xls_f, sep = sep, index = False, encoding = "utf-8")
    return df

  @staticmethod
  def objectarray_to_dataframe(target_data):
    array_data = target_data
    # 単一オブジェクトならリスト化
    if not isinstance(array_data, list):
      array_data = [array_data]

    # 各要素をフラット化してテーブルに変換
    flat_rows = []
    for elem in array_data:
      row = {}
      Util._obj2row(elem, '', row)
      flat_rows.append(row)

    # DataFrameに変換（列自動補完、欠損はNaN）
    return pd.DataFrame(flat_rows)

  @staticmethod
  def _obj2row(objct, key, row):
    if isinstance(objct, list):
      for i, v in enumerate(objct):
        nkey = f"{key}[{i}]" if key else f"[{i}]"
        Util._obj2row(v, nkey, row)
      return

    if isinstance(objct, dict):
      for k, v in objct.items():
        nkey = f"{key}.{k}" if key else k
        Util._obj2row(v, nkey, row)
      return

    row[key] = objct

  @staticmethod
  def escape_string(inp_str: str, is_escape: bool) -> str:
    convert_table = [
        ['\\', r'\\'],
        ['\r', r'\r'],
        ['\n', r'\n'],
        ['\t', r'\t'],
      ]
    return Util.conv_from_list(convert_table, inp_str, direct_lr = is_escape)

  @staticmethod
  def conv_from_list(convert_table: list[list[str]], inp_str: str,
      direct_lr: bool, is_regex: bool = False) -> str:
    i0, i1 = (0, 1) if direct_lr else (1, 0)
    table = convert_table if direct_lr else list(reversed(convert_table))
    outp_str = inp_str
    for cnv in table:
      # print("cnv={}: i0={}, i1={}".format(cnv, i0, i1))
      if is_regex:
        outp_str = re.sub(cnv[i0], cnv[i1], outp_str)
      else:
        outp_str = outp_str.replace(cnv[i0], cnv[i1])
    return outp_str

  @staticmethod
  def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
      sock.settimeout(timeout)
      try:
        sock.connect((host, port))
      except (socket.timeout, ConnectionRefusedError, OSError):
        return False
      return True

  @staticmethod
  def wait_for_port(host: str, port: int, timeout: float, interval: float = 1.0) -> bool:
    t = time.time() + timeout
    while not Util.is_port_open(host, port):
      time.sleep(interval)
      if t < time.time():
        return False
    return True

  @staticmethod
  def wait_for_keyword(process, keyword) -> bool:
    for line in iter(process.stdout.readline, ''):
      decoded = line.strip()
      if keyword in decoded:
        return True
    return False
