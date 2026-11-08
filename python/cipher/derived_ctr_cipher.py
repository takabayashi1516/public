from crypt import Crypt

class DerivedCtrCipher:

  def __init__(
    self,
    master_key,
    context,
    nonce
  ):
    self.master_key = master_key
    self.context = context
    self.nonce = nonce

  def encrypt(
    self,
    data,
    counter
  ):

    derived_key = self.derive_key(
      counter
    )

    keystream = self.generate_keystream(
      derived_key,
      counter,
      len(data)
    )

    return Crypt.xor_vector(
      data,
      keystream
    )

  def derive_key(
    self,
    counter
  ):

    msg = bytearray()

    msg.extend(self.context)

    msg.extend(self.nonce)

    msg.extend(
      counter.to_bytes(
        4,
        "big"
      )
    )

    aes = Crypt(
      self.master_key,
      bytes(16)
    )

    return aes.cmac(bytes(msg))

  def build_counter_block(
    self,
    counter
  ):

    block = bytearray()

    block.extend(
      self.nonce
    )

    block.extend(
      counter.to_bytes(
        4,
        "big"
      )
    )

    while len(block) < 16:
      block.append(0)

    return bytes(block)

  def generate_keystream(
    self,
    derived_key,
    counter,
    length
  ):

    stream = bytearray()

    aes = Crypt(
      derived_key,
      bytes(16)
    )

    while len(stream) < length:

      block = self.build_counter_block(
        counter
      )

      enc = aes.ecb_crypt(
        block
      )

      stream.extend(enc)

      counter += 1

    return bytes(
      stream[:length]
    )
