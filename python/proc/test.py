import asyncio
import signal

from prcctrl import ProcessController

async def main():

  # SSH トンネルコマンド
  cmd = [
    'ssh',
    '-N',                       # コマンド実行なし
    '-L', '2345:localhost:5432',  # ローカルポート:宛先ホスト:宛先ポート
    'user@remotehost'
  ]

  pctrl = ProcessController(cmd)

  # トンネル開始
  ssh_proc = pctrl.start()

  print(f'timer start')
  try:
    # 任意の処理（ここでは5秒間）
    await asyncio.sleep(5)
    # pass

  finally:
    print('Terminating SSH tunnel...')
    pctrl.stop()
    print('SSH tunnel terminated.')

if __name__ == '__main__':
  asyncio.run(main())


