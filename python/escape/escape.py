
def escape_string(inp_str: str, is_escape: bool) -> str:
  convert_table = [
      ['\\', r'\\'],
      ['\r', r'\r'],
      ['\n', r'\n'],
      ['\t', r'\t'],
    ]
  outp_str = inp_str; i0 = 0; i1 = 1
  if (not is_escape):
    convert_table = reversed(convert_table)
    i0 = 1; i1 = 0
  for cnv in convert_table:
    #print("cnv={}: i0={}, i1={}".format(cnv, i0, i1))
    outp_str = outp_str.replace(cnv[i0], cnv[i1])
  return outp_str

