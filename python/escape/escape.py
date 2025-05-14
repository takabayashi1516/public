import re

def escape_string(inp_str: str, is_escape: bool) -> str:
  convert_table = [
      ['\\', r'\\'],
      ['\r', r'\r'],
      ['\n', r'\n'],
      ['\t', r'\t'],
    ]
  return conv_from_list(convert_table, inp_str, direct_lr = is_escape)

def conv_from_list(convert_table: list[list[str]], inp_str: str,
    direct_lr: bool, is_regex: bool = False) -> str:
  i0, i1 = (0, 1) if direct_lr else (1, 0)
  table = convert_table if direct_lr else list(reversed(convert_table))
  outp_str = inp_str
  for cnv in table:
    # print("cnv={}: i0={}, i1={}".format(cnv, i0, i1))
    if is_regex:
      outp_str = re.sub(cnv[i0], cnv[i1], outp_str)
    else:
      outp_str = outp_str.replace(cnv[i0], cnv[i1])
  return outp_str

