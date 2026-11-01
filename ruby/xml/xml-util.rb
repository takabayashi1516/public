require 'rexml/document'

class XmlUtil

  def self.from_xml(xml_str)
    doc = REXML::Document.new(xml_str)
    root = doc.root
    { root.name.to_sym => parse_element(root) }
  end

  def self.to_xml(hash, root_name: 'root')
    doc = REXML::Document.new
    root = doc.add_element(root_name)
    build_element(root, hash[root_name] || hash)
    out = ""
    doc.write(out, 2)
    out
  end

  private

  def self.build_element(parent, data)
    data.each do |key, value|
      case key
      when :__text
        parent.text = value.to_s

      when :__attrs
        value.each { |attr_key, attr_val| parent.add_attribute(attr_key.to_s, attr_val.to_s) }

      else
        # 子要素
        if value.is_a?(Array)
          # 配列 → 同名要素を複数追加
          value.each do |child|
            elem = parent.add_element(key.to_s)
            build_element(elem, child)
          end

        elsif value.is_a?(Hash)
          # Hash → 子要素
          elem = parent.add_element(key.to_s)
          build_element(elem, value)

        else
          # String → テキスト要素
          elem = parent.add_element(key.to_s)
          elem.text = value.to_s
        end
      end
    end
  end

  def self.parse_element(elem)
    result = {}

    # 属性
    unless elem.attributes.empty?
      result[:__attrs] = {}
      elem.attributes.each do |k, v|
        result[:__attrs][k.to_sym] = v
      end
    end

    # テキスト（空白だけの場合は無視）
    text = elem.text&.strip
    result[:__text] = text if text && !text.empty?

    # 子要素
    children = elem.elements.to_a
    unless children.empty?
      children.each do |child|
        key = child.name.to_sym
        value = parse_element(child)

        if result[key]
          # 既に同名キーがある → 配列化
          result[key] = [result[key]] unless result[key].is_a?(Array)
          result[key] << value
        else
          result[key] = value
        end
      end
    end
    result
  end

end
