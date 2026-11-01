require './asn1parser.rb'
require '../util/hex.rb'

parser = ASN1Parser.new(
  {
    type: 'SEQUENCE',
    value: [
      {
        type: 'INTEGER',
        value: 123
      },
      {
        type: 'UTF8STRING',
        value: 'ABC'
      }
    ]
  },
  # enc_type: :ber
)

p '-----------------------------------------------'
puts parser.hex
p '--------------------------'
p parser.bin
p '--------------------------'
p parser.bytes
p '--------------------------'
pp parser.hash

p '-----------------------------------------------'
parser.data = '300802017B0C03616263'

puts parser.hex
p '--------------------------'
p parser.bin
p '--------------------------'
p parser.bytes
p '--------------------------'
pp parser.hash

p '==============================================='
parser = ASN1Parser.new(
  '300802017B0C03414243',
  # enc_type: :ber
)
pp parser.hash


