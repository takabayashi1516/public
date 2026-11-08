from cryptography.hazmat.primitives.cmac import CMAC
from cryptography.hazmat.primitives.ciphers import (
  Cipher,
  algorithms,
  modes
)
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric.utils import (
  encode_dss_signature
)

class Crypt:

  def __init__(self, aes_key=None, aes_iv=None):
    self.aes_key = aes_key
    self.aes_iv = aes_iv

    if aes_key is not None:
      self.validate_aes_key()

  def validate_aes_key(self):

    if len(self.aes_key) not in (16, 24, 32):
      raise RuntimeError("invalid key size")

    if self.aes_iv is not None:
      if len(self.aes_iv) != 16:
        raise RuntimeError("invalid iv size")

  def cmac(self, data):

    c = CMAC(
      algorithms.AES(self.aes_key)
    )

    c.update(data)

    return c.finalize()

  def ctr_crypt(self, data):

    cipher = Cipher(
      algorithms.AES(self.aes_key),
      modes.CTR(self.aes_iv),
    )

    enc = cipher.encryptor()

    return enc.update(data) + enc.finalize()

  def ecb_crypt(self, data):

    cipher = Cipher(
      algorithms.AES(self.aes_key),
      modes.ECB(),
    )

    enc = cipher.encryptor()

    return enc.update(data) + enc.finalize()

  @staticmethod
  def xor_vector(data1, data2):

    if len(data1) != len(data2):
      raise RuntimeError(
        "xor size mismatch"
      )

    return bytes(
      a ^ b
      for a, b
      in zip(data1, data2)
    )

  @staticmethod
  def generate_keypair():
    return ec.generate_private_key(
      ec.SECP256R1()
    )

  @staticmethod
  def sign(private_key, data):

    return private_key.sign(
      data,
      ec.ECDSA(hashes.SHA256())
    )

  @staticmethod
  def verify(public_key,
         data,
         signature):

    try:
      public_key.verify(
        signature,
        data,
        ec.ECDSA(
          hashes.SHA256()
        )
      )
      return True
    except Exception:
      return False
