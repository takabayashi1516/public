'''
import sys
sys.path.append('/path/to/escape')
'''

from escape import escape_string

# 入力例
original_string = "これは改行を含む文字列です。\nそしてタブ\tも含みます。"
# エスケープ処理
escaped_string = escape_string(original_string, True)

print("元の文字列:")
print(original_string)
print("\n★★★エスケープ処理後の文字列:")
print(escaped_string)


# 入力例
escaped_string = "これは改行を含む文字列です。\\nそしてタブ\\tも含みます。"

# デコード処理
original_string = escape_string(escaped_string, False)

print("\nエスケープ文字列:")
print(escaped_string)
print("\n★★★元の制御文字を含む文字列:")
print(original_string)
