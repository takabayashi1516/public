import boto3
import configparser
import os
from dataclasses import dataclass
from logging import getLogger

logger = getLogger(name = __name__)

@dataclass(frozen = True)
class Constants:
  DEFAULT_PROFILE: str = "default"
  DEFAULT_SERVICE: str = "sts"
  DEFAULT_RESION: str = "ap-northeast-1"

def get_aws_credentials(
    mfaserial: str, mfacode: str,
    profile: str = Constants.DEFAULT_PROFILE,
    service: str = Constants.DEFAULT_SERVICE,
    region: str = Constants.DEFAULT_RESION):
  logger.debug("get_aws_credentials(mfaserial={}, profile={}, service={}, region={})"
      .format(mfaserial, profile, service, region))

  session = boto3.Session(profile_name = profile)
  client = session.client(service_name = service,
      region_name = region)

  response = client.get_session_token(
    DurationSeconds = 86400,
    SerialNumber = mfaserial,
    TokenCode = mfacode
  )

  return response["Credentials"]

def update_credentials_mfa(credentials):
  aws_credentials_path = os.path.expanduser("~/.aws/credentials")
  config = configparser.ConfigParser()
  config.read(aws_credentials_path)

  if not config.has_section("mfa"):
    config.add_section("mfa")

  config.set("mfa", "aws_access_key_id", credentials["AccessKeyId"])
  config.set("mfa", "aws_secret_access_key", credentials["SecretAccessKey"])
  config.set("mfa", "aws_session_token", credentials["SessionToken"])

  with open(aws_credentials_path, "w") as configfile:
    config.write(configfile)
