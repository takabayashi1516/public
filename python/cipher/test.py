from crypt import Crypt
from derived_ctr_cipher import DerivedCtrCipher

def dump_hex(name, data):

  print(
    f"{name} ({len(data)} bytes): "
    f"{data.hex().upper()}"
  )

key = bytes([
  1,2,3,4,5,6,7,8,
  9,10,11,12,13,14,15,16,
  17,18,19,20,21,22,23,24,
  25,26,27,28,29,30,31,32
])

iv = bytes([
  1,2,3,4,
  5,6,7,8,
  9,10,11,12,
  13,14,15,16
])

aes = Crypt(key, iv)

text = b"Hello OpenSSL"

encrypted = aes.ctr_crypt(text)
dump_hex("encrypted", encrypted)

decrypted = aes.ctr_crypt(encrypted)
dump_hex("decrypted", decrypted)

mac = aes.cmac(text)
dump_hex("mac", mac)

context = b"CTX1"

nonce = bytes([
  0xA1,0xA2,0xA3,0xA4,
  0xA5,0xA6,0xA7,0xA8,
  0xA9,0xAA,0xAB,0xAC
])

ky = bytes([
  0x01,0x02,0x03,0x04,
  0x05,0x06,0x07,0x08,
  0x19,0x1A,0x1B,0x1C,
  0x1D,0x1E,0x1F,0x20
])

drv = DerivedCtrCipher(
  ky,
  context,
  nonce
)

enc = drv.encrypt(
  text,
  1
)

dump_hex(
  "derived encrypted",
  enc
)

dec = drv.encrypt(
  enc,
  1
)

dump_hex(
  "derived decrypted",
  dec
)

print(
  "derived text:",
  dec.decode()
)
