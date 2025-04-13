#
# read qr-code
#   $ python qr.py [ --inp=${imagePath} --camera=${cameraIndex} --displayTime=1000 [ --hide ] ]
# create qr-code
#   $ python qr.py --data=${planeText} --outp=${imagePath} [ --size=${pixcelSize} --version=${qrCodeSize} ]
#

import argparse
import cv2
import datetime
import logging
import qrcode

from pyzbar.pyzbar import decode
from PIL import Image

from logging import getLogger

class QrCodeUtil:

  def __init__(self, camera: int = 0, version: int = 1, size: int = 10):
    self.camera = camera
    self.version = version
    self.size = size
    self.detector = cv2.QRCodeDetector()
    self.logger = getLogger(name = __name__)

  '''
  $ python -c "import qr; qrutl=qr.QrCodeUtil(0); rc=qrutl.read_from_camera(); print(rc)"
  '''
  def read_from_camera(self, hide: bool = False,
      displayTime: int = 1000):
    data = None
    bbox = None

    # カメラデバイスを開く
    cap = cv2.VideoCapture(self.camera)

    while True:
      # フレームをキャプチャ
      ret, img = cap.read()
      if not ret:
        break
      # QRコードを検出してデコード
      data, bbox, _ = self.detector.detectAndDecode(img)
      if bbox is not None:
        if not hide:
          cv2.imshow("QR Code Reader", img)
          # 'q'キーが押されたら終了
          if cv2.waitKey(10) & 0xFF == ord('q'):
            break
        if data:
          img = cv2.polylines(img, bbox.astype(int), True, (0, 255, 0), 3)
          cv2.imshow("QR Code Reader", img)
          cv2.waitKey(displayTime)
          break

      if hide:
        self.logger.info("再スキャン開始 {}".format(datetime.datetime.now()))

    # リソースを解放
    cap.release()
    if not hide:
      cv2.destroyAllWindows()
    return data, bbox

  '''
  $ python -c "import qr; qrutl=qr.QrCodeUtil(0,4,10); rc=qrutl.read_from_image('./a.png'); print(rc)"
  '''
  def read_from_image(self, imagePath: str):

    # 画像を読み込む
    img = Image.open(imagePath)
    if img is None:
      return None

    # QRコードを検出してデコード
    return decode(img)

  '''
  $ python -c "import qr; qrutl=qr.QrCodeUtil(0,10,10); qrutl.create('test-data', './test.png')"
  '''
  def create(self, data: str, output_file: str):
    # QRコードを作成
    qr = qrcode.QRCode(
      version = self.version,
      error_correction=qrcode.constants.ERROR_CORRECT_L,  # 誤り訂正レベル
      box_size = self.size,
      border = 4,  # ボーダーの幅（ボックス単位）
    )

    # データを追加
    qr.add_data(data)
    qr.make(fit=True)

    # QRコードを画像として生成
    img = qr.make_image(fill_color="black", back_color="white")

    # 画像を保存
    img.save(output_file)

################################################################################
def main():
  logging.basicConfig(level = logging.INFO)
  logger = logging.getLogger(name = __name__)

  parser = argparse.ArgumentParser(description = '')
  parser.add_argument("--inp", default = '')
  parser.add_argument("--outp", default = '')
  parser.add_argument("--camera", type = int, default = 0)
  parser.add_argument("--hide", default = False, action = 'store_true')
  parser.add_argument("--displayTime", type = int, default = 1000)
  parser.add_argument("--data", default = '')
  parser.add_argument("--size", type = int, default = 10)    # 各ボックスのサイズ（ピクセル）
  parser.add_argument("--version", type = int, default = 1)  # QRコードのサイズ（1〜40）。1が最小。
  args = parser.parse_args()

  qrcodeUtil = QrCodeUtil(args.camera, args.version, args.size)
  if args.outp != '':
    qrcodeUtil.create(args.data, args.outp)
    return

  if args.inp == '':
    data, bbox = qrcodeUtil.read_from_camera(args.hide, args.displayTime)

    if bbox is not None:
      print(f"{data}", end = "\n")
      logger.debug("bbox={},\ndecode: \n{}".format(bbox, data))

    else:
      logger.debug("QRコードが見つかりませんでした。")

    return

  objs = qrcodeUtil.read_from_image(args.inp)

  # data, type, rect, polygon, quality, orientation
  for obj in objs:
    data = obj.data.decode("utf-8")
    print(f"{data}", end = "\n")
    logger.debug("bbox={},\ndecode: \n{}".format(obj.rect, data))

  return

if __name__ == '__main__':
  main()
