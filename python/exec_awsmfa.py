import argparse
import boto3
import configparser
import os
import sys
import logging
from dataclasses import dataclass
from otp.libotp import (
    LibOtp,
  )
from awsmfa.libawsmfa import (
    Constants as lamConst,
    get_aws_credentials,
    update_credentials_mfa,
  )

def main():
  parser = argparse.ArgumentParser(description = "")
  parser.add_argument("--profile", default = lamConst.DEFAULT_PROFILE)
  parser.add_argument("--mfaserial")
  parser.add_argument("--qrcode")
  parser.add_argument("--secretkey")
  parser.add_argument("--service", default = lamConst.DEFAULT_SERVICE)
  parser.add_argument("--region", default = lamConst.DEFAULT_RESION)
  parser.add_argument("--log", type = int, default = logging.WARNING)
  args = parser.parse_args()

  logging.basicConfig(level = args.log)
  logger = logging.getLogger(name = __name__)

  if (((not args.secretkey) and (not args.qrcode))
      or (not args.mfaserial)):
    print("something wrong: secret-key={}, mfa-serial={}"
        .format(args.secretkey, args.mfaserial), file=sys.stderr)
    return

  # MFAデバイスの情報を入力
  secret_key = args.secretkey
  if args.qrcode:
    secret_key = LibOtp().get_secret_key_qr(args.qrcode)
  mfa_code = LibOtp().create_totp(secret_key)

  credentials = get_aws_credentials(
      mfaserial = args.mfaserial,
      mfacode = mfa_code,
      profile = args.profile,
      service = args.service,
      region = args.region)

  update_credentials_mfa(credentials)

  logger.debug("MFA session credentials updated!")

if __name__ == "__main__":
  main()

