# frozen_string_literal: true

require 'openssl'

class ASN1Parser

  SUPPORTED_ENC_TYPES = %i[
    der
    ber
  ].freeze

  attr_reader :hash
  attr_reader :bin
  attr_reader :hex
  attr_reader :bytes
  attr_reader :enc_type

  def initialize(data = nil, enc_type: :der)
    self.enc_type = enc_type

    self.data = data unless data.nil?
  end

  #
  # public
  #

  def enc_type=(value)
    value = value.to_sym

    unless SUPPORTED_ENC_TYPES.include?(value)
      raise ArgumentError,
            "Unsupported enc_type: #{value}"
    end

    @enc_type = value
  end

  def data=(value)
    case value

    when Hash
      encode_from_hash(value)

    when String
      if hex_string?(value)
        decode([value].pack('H*'))
      else
        decode(value.b)
      end

    when Array
      unless value.all? { |v| v.is_a?(Integer) && v.between?(0, 255) }
        raise ArgumentError,
              'Invalid byte array'
      end

      decode(value.pack('C*'))

    else
      raise ArgumentError,
            "Unsupported type #{value.class}"
    end
  end

  private

  #
  # ENCODE
  #

  def encode_from_hash(hash)
    asn1 = build_asn1(hash)

    encoded =
      case @enc_type
      when :der
        asn1.to_der

      when :ber
        #
        # OpenSSL Ruby は BER エンコード API を持たない
        # 現状は DER と同一出力
        #
        asn1.to_der

      else
        raise "Unsupported enc_type #{@enc_type}"
      end

    @hash  = hash
    @bin   = encoded
    @hex   = encoded.unpack1('H*').upcase
    @bytes = encoded.bytes
  end

  #
  # DECODE
  #

  def decode(bin)
    asn1 = OpenSSL::ASN1.decode(bin)

    @hash  = asn1_to_hash(asn1)
    @bin   = bin
    @hex   = bin.unpack1('H*').upcase
    @bytes = bin.bytes
  end

  #
  # HASH -> ASN1
  #

  def build_asn1(hash)
    type =
      hash[:type] ||
      hash['type']

    value =
      hash[:value] ||
      hash['value']

    case type.to_s.upcase

    when 'INTEGER'
      OpenSSL::ASN1::Integer.new(value)

    when 'BOOLEAN'
      OpenSSL::ASN1::Boolean.new(value)

    when 'UTF8STRING'
      OpenSSL::ASN1::UTF8String.new(value)

    when 'OCTETSTRING'
      OpenSSL::ASN1::OctetString.new(value)

    when 'OBJECTID'
      OpenSSL::ASN1::ObjectId.new(value)

    when 'SEQUENCE'

      children =
        (value || []).map do |child|
          build_asn1(child)
        end

      OpenSSL::ASN1::Sequence.new(children)

    when 'SET'

      children =
        (value || []).map do |child|
          build_asn1(child)
        end

      OpenSSL::ASN1::Set.new(children)

    else
      raise ArgumentError,
            "Unsupported type #{type}"
    end
  end

  #
  # ASN1 -> HASH
  #

  def asn1_to_hash(obj)

    hash = {
      type: obj.class.name.split('::').last.upcase
    }

    hash[:tag] = obj.tag if obj.respond_to?(:tag)

    if obj.respond_to?(:tag_class)
      hash[:tag_class] = obj.tag_class
    end

    if obj.respond_to?(:infinite_length)
      hash[:infinite_length] =
        obj.infinite_length
    end

    hash[:value] = decode_value(obj)

    hash
  end

  def decode_value(obj)

    case obj

    when OpenSSL::ASN1::Sequence,
         OpenSSL::ASN1::Set

      obj.value.map do |child|
        asn1_to_hash(child)
      end

    when OpenSSL::ASN1::ObjectId

      obj.oid

    else

      obj.value
    end
  end

  #
  # UTIL
  #

  def hex_string?(str)
    str.match?(/\A[0-9a-fA-F]+\z/) &&
      str.length.even?
  end

end
