import asyncio

from httputil.httputil import (
    HttpUtil
  )
from util.util import (
    Util,
  )

async def main():
  util = HttpUtil(retry_param = {
      HttpUtil.Constants.RETRY_PARAM_NUM: 3,
      HttpUtil.Constants.RETRY_PARAM_SEC: 2
    })
  response = await util.get("https://www.google.co.jp")
  #response = await util.get("https://www.yahoo.co.jp")
  print(await response.text())
  await Util.sleep(1000)

asyncio.run(main())

