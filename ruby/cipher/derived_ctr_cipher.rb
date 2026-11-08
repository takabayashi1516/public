require 'openssl'

class DerivedCtrCipher

  def initialize(master_key, context, nonce)
    @master_key = master_key
    @context    = context
    @nonce      = nonce
  end

  def encrypt(data, counter)
    derived_key = derive_key(counter)
    keystream   = generate_keystream(
      derived_key,
      counter,
      data.bytesize
    )

    xor_bytes(data, keystream)
  end

  private

  def derive_key(counter)
    msg = @context.dup
    msg << @nonce

    msg << [
      (counter >> 24) & 0xff,
      (counter >> 16) & 0xff,
      (counter >>  8) & 0xff,
      counter & 0xff
    ].pack('C*')

    cmac_aes(@master_key, msg)
  end

  def build_counter_block(counter)
    block = @nonce.dup

    block << [
      (counter >> 24) & 0xff,
      (counter >> 16) & 0xff,
      (counter >>  8) & 0xff,
      counter & 0xff
    ].pack('C*')

    block.ljust(16, "\x00")
  end

  def generate_keystream(derived_key,
                         counter,
                         length)

    stream = ''.b

    while stream.bytesize < length
      block = build_counter_block(counter)

      stream << aes_ecb_encrypt(
        derived_key,
        block
      )

      counter += 1
    end

    stream.byteslice(0, length)
  end

  def aes_ecb_encrypt(key, block)
    cipher = OpenSSL::Cipher.new(
      "AES-#{key.bytesize * 8}-ECB"
    )

    cipher.encrypt
    cipher.padding = 0

    cipher.key = key

    cipher.update(block) + cipher.final
  end

  def xor_bytes(data1, data2)
    raise 'xor size mismatch' \
      unless data1.bytesize == data2.bytesize

    result = ''.b

    data1.bytes.zip(data2.bytes) do |a, b|
      result << (a ^ b)
    end

    result
  end

  #
  # OpenSSL 3.x
  #
  def cmac_aes(key, data)
    cipher_name =
      case key.bytesize
      when 16 then "AES-128-CBC"
      when 24 then "AES-192-CBC"
      when 32 then "AES-256-CBC"
      else
        raise "invalid key size"
      end

    OpenSSL::KDF.hkdf(
      data,
      salt: "",
      info: "",
      length: 16,
      hash: "SHA256"
    )

    # OpenSSL::CMAC があればこちら推奨
    # OpenSSL::CMAC.digest(cipher_name, key, data)
  end
end
