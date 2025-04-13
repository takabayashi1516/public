'''
$ python exec_otp.py --img=${image_path}
$ python exec_otp.py --img=${image_path} --sk=?
$ python exec_otp.py --sk=${secret_key}
$ python exec_otp.py --sk=${secret_key} --totp=${totp}
$ python exec_otp.py
'''

'''
import sys
sys.path.append('/path/to/libotp')
'''

import argparse
import logging

from otp.libotp import (
    LibOtp,
  )

from escape.escape import escape_string

'''
$ python -c "import exec_otp; exec_otp.print_totp_qr(${image_path})"
'''
def print_totp_qr(qr_image_path: str):
  libotp = LibOtp()
  secret_key = libotp.get_secret_key_qr(qr_image_path)
  print(f"{libotp.create_totp(secret_key)}", end = "\n")

'''
$ python -c "import exec_otp; exec_otp.print_secret_key_qr(${image_path})"
'''
def print_secret_key_qr(qr_image_path: str):
  print(f"{LibOtp().get_secret_key_qr(qr_image_path)}", end = "\n")

'''
$ python -c "import exec_otp; exec_otp.print_secret_key()"
'''
def print_secret_key():
  print(LibOtp().get_random_base32(), end = "\n")

'''
$ python -c "import exec_otp; exec_otp.print_totp_secret_key(${secret_key})"
'''
def print_totp_secret_key(secret_key: str):
  print(f"{LibOtp().create_totp(secret_key)}", end = "\n")

'''
$ python -c "import exec_otp; exec_otp.make_qrcode(${secret_key}, ${otp_user_name}, ${otp_issuer_name}, ${image_path})"
'''
def make_qrcode(secret_key: str, otp_user_name: str, otp_issuer_name: str,
    image_path: str, version: int = 1, size: int = 10):
  LibOtp().make_qrcode(secret_key, otp_user_name, otp_issuer_name,
      image_path, version, size)

def main():
  logging.basicConfig(level = logging.WARNING)
  logger = logging.getLogger(name = __name__)

  parser = argparse.ArgumentParser(description = '')
  parser.add_argument("--totp", default = '')
  parser.add_argument("--sk")
  parser.add_argument("--img", default = '')
  args = parser.parse_args()

  try:
    if args.img != '':
      if args.sk:
        print_secret_key_qr(args.img)
        return
      print_totp_qr(args.img)
      return

    if args.sk:
      if args.totp != '':
        rc = LibOtp().confirm_totp(args.sk, args.totp)
        logger.info(f"confirm_totp={rc}")
        return
      logger.debug("escape:{}".format(escape_string(args.sk, True)))
      logger.debug("raw   :{}".format(args.sk))
      print_totp_secret_key(args.sk)
      return

    while True:
      totp_code = input('totp_code: ')
      rc = confirm_totp(args.sk, totp_code)
      logger.info("result={}, sk={}, totp={}".format(rc, args.sk, totp_code))
  except ValueError as e:
    logger.error(f"[E]: {e}")

if __name__ == "__main__":
  main()
