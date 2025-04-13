import pyotp
from PIL import Image
import cv2
import os
import sys

from qr.qr import (
    QrCodeUtil,
  )
from logging import getLogger

class LibOtp:

  def __init__(self):
    self.logger = getLogger(name = __name__)

  def __get_totp(self, secret_key: str):
    return pyotp.TOTP(secret_key)

  def __totp_verify(self, totp, totp_code: str):
    return totp.verify(totp_code)

  '''
  $ python -c "import otp; otp.make_qrcode(${SECRET_KEY}, ${OTP_USER}, ${OTP_ISSUER}, ${QRCODE_IMAGE})"
  '''
  def make_qrcode(self, secret_key: str, otp_user_name: str, otp_issuer_name: str,
      qrcode_image: str, version: int = 1, size: int = 10):
    # timeベースotp生成
    totp = self.__get_totp(secret_key)
    data = totp.provisioning_uri(
        name = otp_user_name,
        issuer_name = otp_issuer_name)
    # qr生成
    QrCodeUtil(version, size).create(data, qrcode_image)

  def create_totp(self, secret_key: str):
    # PyOTPでTOTPを生成
    totp = self.__get_totp(secret_key)
    totp_code = totp.now()
    self.__totp_verify(totp, totp_code)
    return f"{totp_code}"

  def get_secret_key_qr(self, qr_image_path: str):

    # QRコード画像を読み込み
    objs = QrCodeUtil().read_from_image(qr_image_path)

    for obj in objs:
      data = obj.data.decode("utf-8")
      #print(f"{data}", end = "\n")
      self.logger.debug("bbox={},\ndecode: \n{}".format(obj.rect, data))

      if not data:
        self.logger.warn("No QR code.", file=sys.stderr)
        raise ValueError('No QR code.')
        return

      self.logger.info(f"{data}")
      # シークレットキーを抽出
      secret_key = data.split('secret=')[1].split('&')[0].split('%')[0]
      return f"{secret_key}"

    return None

  def confirm_totp(self, secret_key: str, totp_code: str):
    return self.__totp_verify(self.__get_totp(secret_key), totp_code)

  def get_random_base32(self):
    # base32のシークレットキーを発行
    return pyotp.random_base32()

