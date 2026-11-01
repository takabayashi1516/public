class HexUtil
  # ------------------------------------------------------------
  # HEX文字列 → byte配列
  # "41 42 43" / "414243" どちらも対応
  # ------------------------------------------------------------
  def self.hex_to_bytes(hex_str, delimiter = " ")
    pattern = Regexp.new("[^0-9A-Fa-f#{Regexp.escape(delimiter)}]")
    cleaned = hex_str.gsub(pattern, "")
    cleaned.gsub(delimiter, "")  # delimiter を除去
    cleaned.scan(/../).map { |h| h.to_i(16) }
  end

  # ------------------------------------------------------------
  # byte配列 → HEX文字列
  # ------------------------------------------------------------
  def self.bytes_to_hex(bytes, delimiter = " ")
    bytes.map { |b| "%02X" % b }.join(delimiter)
  end

  # ------------------------------------------------------------
  # byte配列 → バイナリ文字列
  # ------------------------------------------------------------
  def self.bytes_to_bin(bytes)
    bytes.pack("C*").force_encoding("BINARY")
  end

  # ------------------------------------------------------------
  # バイナリ文字列 → byte配列
  # ------------------------------------------------------------
  def self.bin_to_bytes(bin_str)
    bin_str.bytes
  end

  # ------------------------------------------------------------
  # HEX文字列 → バイナリ文字列
  # ------------------------------------------------------------
  def self.hex_to_bin(hex_str, delimiter = " ")
    bytes = hex_to_bytes(hex_str, delimiter)
    bytes_to_bin(bytes)
  end

  # ------------------------------------------------------------
  # バイナリ文字列 → HEX文字列
  # ------------------------------------------------------------
  def self.bin_to_hex(bin_str, delimiter = " ")
    bytes_to_hex(bin_to_bytes(bin_str), delimiter)
  end
end
