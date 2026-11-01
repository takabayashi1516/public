require_relative 'crypt.rb'

def dump_hex(name, data)
  puts "#{name} (#{data.bytesize} bytes): #{data.unpack1('H*').upcase}"
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

text = 'Hello OpenSSL'

#
# AES-CTR
#
encrypted = aes.ctr_crypt(text)
dump_hex('encrypted', encrypted)

decrypted = aes.ctr_crypt(encrypted)
dump_hex('decrypted', decrypted)

#
# AES-CMAC
#
mac = aes.cmac(text)
dump_hex('mac', mac)

#
# CTR = ECB(counter) XOR plaintext
#
plain = text.b

counter = iv.dup

key_stream = aes.ecb_crypt(counter)

key_stream = key_stream[0, text.bytesize]

encrypted_ecb_xor =
  aes.xor_vector(
    plain,
    key_stream
  )

dump_hex(
  'encrypted(ecb + xor)',
  encrypted_ecb_xor
)

#
# ECDSA
#
ecc = Crypto.new(:p256)

pkey = ecc.generate_key_pair

sig = ecc.sign(pkey, text)

dump_hex('sig', sig)

rc = ecc.verify(
  pkey,
  text,
  sig
)

puts "verify: #{rc ? 'OK' : 'NG'}"

#
# 同一データを複数回署名
#
3.times do |i|
  sig = ecc.sign(pkey, text)
  puts "signature#{i}: len=#{sig.bytesize}"
  dump_hex("signature#{i}", sig)
end
