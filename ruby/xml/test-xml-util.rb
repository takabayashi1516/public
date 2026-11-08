require './xml-util.rb'
require '../util/hex.rb'

xml = <<~XML
<book id="123">
  <title>吾輩は猫である</title>
  <author>夏目漱石</author>
  <tags>
    <tag>文学</tag>
    <tag>日本</tag>
  </tags>
</book>
XML

hash = XmlUtil.from_xml(xml)
pp hash


data = {
  book: {
    __attrs: { id: "123" },
    title: "吾輩は猫である",
    author: "夏目漱石",
    tags: [
      { tag: "\x11\x22\x33" },
      { tag: "日本" }
    ]
  }
}

puts XmlUtil.to_xml(data, root_name: 'book')

p HexUtil.bin_to_bytes(data[:book][:tags][0][:tag])
p HexUtil.bin_to_hex(data[:book][:tags][0][:tag])
p data[:book][:tags].map { |t| t[:tag] }



