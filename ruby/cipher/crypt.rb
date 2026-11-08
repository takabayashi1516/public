require 'openssl'

class Crypto

  def initialize(aes_key = nil, aes_iv = nil)
    @aes_key = aes_key
    @aes_iv  = aes_iv

    validate_aes_key if @aes_key
  end

  #
  # AES-CMAC
  #
  def cmac(data)
    validate_aes_key

    unless defined?(OpenSSL::CMAC)
      raise "#{__method__}: OpenSSL::CMAC not available"
    end

    OpenSSL::CMAC.digest(
      cipher_name,
      @aes_key,
      data
    )
  end

  #
  # AES-CTR
  #
  def ctr_crypt(data)
    cipher_crypt(data, ctr_cipher)
  end

  #
  # AES-ECB
  #
  def ecb_crypt(data)
    cipher_crypt(data, ecb_cipher)
  end

  #
  # XOR
  #
  def xor_vector(data1, data2)
    raise "#{__method__}: size mismatch" \
      unless data1.bytesize == data2.bytesize

    result = ''.b

    data1.bytes.zip(data2.bytes) do |a, b|
      result << (a ^ b)
    end

    result
  end

  private

  def validate_aes_key

    case @aes_key.bytesize
    when 16, 24, 32
      #
    else
      raise "#{__method__}: invalid key size"
    end

    unless @aes_iv.nil?
      if @aes_iv.bytesize != 16
        raise "#{__method__}: invalid iv size"
      end
    end
  end

  def ctr_cipher
    case @aes_key.bytesize
    when 16
      'AES-128-CTR'
    when 24
      'AES-192-CTR'
    when 32
      'AES-256-CTR'
    else
      raise "#{__method__}"
    end
  end

  def ecb_cipher
    case @aes_key.bytesize
    when 16
      'AES-128-ECB'
    when 24
      'AES-192-ECB'
    when 32
      'AES-256-ECB'
    else
      raise "#{__method__}"
    end
  end

  def cipher_name
    case @aes_key.bytesize
    when 16
      'AES-128-CBC'
    when 24
      'AES-192-CBC'
    when 32
      'AES-256-CBC'
    else
      raise "#{__method__}"
    end
  end

  def cipher_crypt(data, cipher_type)

    validate_aes_key

    cipher = OpenSSL::Cipher.new(cipher_type)

    cipher.encrypt

    cipher.key = @aes_key

    if cipher.iv_len > 0
      cipher.iv = @aes_iv
    end

    cipher.update(data) + cipher.final
  end

end
