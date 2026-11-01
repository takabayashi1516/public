require 'openssl'

class Crypto
  BLOCK_SIZE = 16
  RB = 0x87

  ECCurve = {
    p256: 'prime256v1',
    p384: 'secp384r1',
    p521: 'secp521r1'
  }.freeze

  def initialize(arg1, arg2 = nil)
    case arg1
    when Symbol
      @curve = arg1
      @aes_key = nil
      @aes_iv = nil

    when String
      @aes_key = arg1.b
      @aes_iv  = arg2.b
      @curve   = :p256

      validate_aes_key
    else
      raise ArgumentError, 'invalid arguments'
    end
  end

  #
  # AES-CMAC
  #
  def cmac(data)
    validate_aes_key

    k1, k2 = generate_subkeys

    if data.bytesize > 0 &&
       (data.bytesize % BLOCK_SIZE).zero?

      last_block =
        xor_vector(
          data[-BLOCK_SIZE, BLOCK_SIZE],
          k1
        )

      body = data[0...-BLOCK_SIZE] || ''.b
    else
      remain =
        data.byteslice(
          (data.bytesize / BLOCK_SIZE) * BLOCK_SIZE,
          data.bytesize % BLOCK_SIZE
        ) || ''.b

      padded =
        remain +
        "\x80" +
        ("\x00" * (BLOCK_SIZE - remain.bytesize - 1))

      last_block =
        xor_vector(
          padded,
          k2
        )

      body =
        data.byteslice(
          0,
          (data.bytesize / BLOCK_SIZE) * BLOCK_SIZE
        ) || ''.b
    end

    x = "\x00" * BLOCK_SIZE

    body.bytes.each_slice(BLOCK_SIZE) do |block|
      block = block.pack('C*')

      x =
        ecb_crypt(
          xor_vector(x, block)
        )
    end

    ecb_crypt(
      xor_vector(x, last_block)
    )
  end

  #
  # AES-CTR
  #
  def ctr_crypt(data)
    validate_aes_key

    cipher =
      OpenSSL::Cipher.new(
        "AES-#{aes_bits}-CTR"
      )

    cipher.encrypt
    cipher.key = @aes_key
    cipher.iv  = @aes_iv

    cipher.update(data) + cipher.final
  end

  #
  # AES-ECB
  #
  def ecb_crypt(data)
    validate_aes_key

    cipher =
      OpenSSL::Cipher.new(
        "AES-#{aes_bits}-ECB"
      )

    cipher.encrypt
    cipher.padding = 0
    cipher.key = @aes_key

    cipher.update(data) + cipher.final
  end

  #
  # ECDSA Key Create
  #
  def generate_key_pair
    OpenSSL::PKey::EC.generate(
      ECCurve.fetch(@curve)
    )
  end

  #
  # ECDSA Sign
  #
  def sign(private_key, data)
    private_key.dsa_sign_asn1(
      OpenSSL::Digest::SHA256.digest(data)
    )
  end

  #
  # ECDSA Verify
  #
  def verify(public_key, data, signature)
    public_key.dsa_verify_asn1(
      OpenSSL::Digest::SHA256.digest(data),
      signature
    )
  end

  #
  # XOR
  #
  def xor_vector(data1, data2)
    raise RuntimeError, 'length mismatch' \
      unless data1.bytesize == data2.bytesize

    data1.bytes
         .zip(data2.bytes)
         .map { |a, b| a ^ b }
         .pack('C*')
  end

  private

  def aes_bits
    @aes_key.bytesize * 8
  end

  def validate_aes_key
    unless [16, 24, 32].include?(@aes_key.bytesize)
      raise RuntimeError, 'invalid key size'
    end

    unless @aes_iv.bytesize == 16
      raise RuntimeError, 'invalid iv size'
    end
  end

  def generate_subkeys
    l = ecb_crypt("\x00" * BLOCK_SIZE)

    k1 = left_shift(l)

    if (l.getbyte(0) & 0x80) != 0
      k1 =
        xor_vector(
          k1,
          ("\x00" * 15) + [RB].pack('C')
        )
    end

    k2 = left_shift(k1)

    if (k1.getbyte(0) & 0x80) != 0
      k2 =
        xor_vector(
          k2,
          ("\x00" * 15) + [RB].pack('C')
        )
    end

    [k1, k2]
  end

  def left_shift(data)
    carry = 0

    result =
      data.bytes.reverse.map do |b|
        next_carry =
          (b & 0x80) >> 7

        value =
          ((b << 1) & 0xff) |
          carry

        carry = next_carry

        value
      end.reverse

    result.pack('C*')
  end
end
