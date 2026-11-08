#!/usr/bin/env ruby

require_relative 'crypt'
require_relative 'derived_ctr_cipher'

def dump_hex(name, data)
  print "#{name} (#{data.bytesize} bytes): "
  puts data.unpack1('H*').upcase
end

key = [
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08,
  0x09, 0x0A, 0x0B, 0x0C,
  0x0D, 0x0E, 0x0F, 0x10,
  0x11, 0x12, 0x13, 0x14,
  0x15, 0x16, 0x17, 0x18,
  0x19, 0x1A, 0x1B, 0x1C,
  0x1D, 0x1E, 0x1F, 0x20
].pack('C*')

iv = [
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08,
  0x09, 0x0A, 0x0B, 0x0C,
  0x0D, 0x0E, 0x0F, 0x10
].pack('C*')

aes = Crypto.new(key, iv)

text =
  "Hello OpenSSL\n" \
  "Hello OpenSSL\n" \
  "Hello OpenSSL\n" \
  "Hello OpenSSL\n"

encrypted = aes.ctr_crypt(text)
dump_hex('encrypted', encrypted)

decrypted = aes.ctr_crypt(encrypted)
dump_hex('decrypted', decrypted)

begin
  mac = aes.cmac(text)
  dump_hex('mac', mac)
rescue => e
  puts "CMAC skipped: #{e.message}"
end

plain = text.b

counter = iv.dup

key_stream = aes.ecb_crypt(counter)
key_stream = key_stream.byteslice(0, text.bytesize)

encrypted_ecb_xor =
  aes.xor_vector(
    plain,
    key_stream.ljust(text.bytesize, "\0")
  )

dump_hex(
  'encrypted(ecb + xor)',
  encrypted_ecb_xor
)

puts '=== DerivedCtrCipher Test ==='

context = [
  0x43, 0x54, 0x58, 0x31
].pack('C*')

nonce = [
  0xA1, 0xA2, 0xA3, 0xA4,
  0xA5, 0xA6, 0xA7, 0xA8,
  0xA9, 0xAA, 0xAB, 0xAC
].pack('C*')

cntr = 1

ky = [
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08,
  0x19, 0x1A, 0x1B, 0x1C,
  0x1D, 0x1E, 0x1F, 0x20
].pack('C*')

drv = DerivedCtrCipher.new(
  ky,
  context,
  nonce
)

begin
  drv_encrypted =
    drv.encrypt(
      text.b,
      cntr
    )

  dump_hex(
    'derived encrypted',
    drv_encrypted
  )
rescue => e
  puts "exception: #{e.message}"
end

begin
  drv_decrypted =
    drv.encrypt(
      drv_encrypted,
      cntr
    )

  dump_hex(
    'derived decrypted',
    drv_decrypted
  )

  puts "derived text: #{drv_decrypted}"
rescue => e
  puts "exception: #{e.message}"
end
