import aiohttp
import asyncio
from aiohttp import ClientSession
from logging import getLogger

from dataclasses import dataclass

from util.util import (
    Util,
  )

class HttpUtil:

  @dataclass(frozen = True)
  class Constants:
    HTTP_GET: str = 'GET'
    HTTP_POST: str = 'POST'
    HTTP_PUT: str = 'PUT'
    HTTP_DELETE: str = 'DELETE'

    HTTP_RESP_ERROR: str = 'error'
    HTTP_RESP_RESPONSE: str = 'response'
    HTTP_RESP_BODY: str = 'body'

    HTTP_RESP_TYPE: str = 'response_type'
    HTTP_RESP_TYPE_STREAM: str = 'stream'

    RETRY_PARAM_NUM: str = 'num'
    RETRY_PARAM_SEC: str = 'sec'

  def __init__(self, retry_param):
    self.logger = getLogger(name = __name__)
    self.logger.debug(f"__init__({retry_param})")
    self._retry_param = retry_param or {}

  async def _request(self, method, url, session, **kwargs):
    rc = {}
    try:
      async with session.request(method, url, **kwargs) as response:
        body = await response.text()
        rc[self.Constants.HTTP_RESP_ERROR] = None
        rc[self.Constants.HTTP_RESP_RESPONSE] = response
        rc[self.Constants.HTTP_RESP_BODY] = body
        return rc
    except Exception as e:
      rc[self.Constants.HTTP_RESP_ERROR] = e
      rc[self.Constants.HTTP_RESP_RESPONSE] = None
      rc[self.Constants.HTTP_RESP_BODY] = None
      return rc

  async def get(self, url, opt=None):
    return await self._retry_request(
        self.Constants.HTTP_GET, url, None, opt)

  async def post(self, url, data=None, opt=None):
    return await self._retry_request(
        self.Constants.HTTP_POST, url, data, opt)

  async def put(self, url, data=None, opt=None):
    return await self._retry_request(
        self.Constants.HTTP_PUT, url, data, opt)

  async def delete(self, url, opt=None):
    return await self._retry_request(
        self.Constants.HTTP_DELETE, url, None, opt)

  async def _retry_request(self, method, url, data, opt):
    opt = opt or {}
    nm_retry = self._retry_param.get(
        self.Constants.RETRY_PARAM_NUM, 1)
    sec_retry = self._retry_param.get(
        self.Constants.RETRY_PARAM_SEC, 1)
    error = None

    async with ClientSession() as session:
      for n in range(nm_retry):
        rslt = await self._request(method, url, session, json=data, **opt)
        response = rslt[self.Constants.HTTP_RESP_RESPONSE]
        if rslt[self.Constants.HTTP_RESP_ERROR] is None and response.status == 200:
          return response
        error = rslt[self.Constants.HTTP_RESP_ERROR]
        self.logger.info(f"stop http: {method.lower()}, {getattr(error, 'status', error)}, retry: {n}")
        await Util.sleep(sec_retry * 1000)

    raise error

  async def stream(self, url, opt, out_file):
    opt = opt or {}
    opt[self.Constants.HTTP_RESP_TYPE] = self.Constants.HTTP_RESP_TYPE_STREAM

    async with ClientSession() as session:
      try:
        async with session.get(url, **opt) as response:
          with open(out_file, 'wb') as f:
            while True:
              chunk = await response.content.read(1024)
              if not chunk:
                break
              f.write(chunk)
          return response
      except Exception as e:
        raise e
