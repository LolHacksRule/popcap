#!/bin/env python
import encodings

maps =  ['cp037', 'cp1250', 'cp1254', 'cp1258', 'cp737', 'cp855', 'cp861', 'cp865', 'cp875',
         'iso8859_10', 'iso8859_15', 'iso8859_3', 'iso8859_7', 'cp1006', 'cp1251', 'cp1255',
         'cp424', 'cp775', 'cp856', 'cp862', 'cp866', 'cp932', 'iso8859_11', 'iso8859_16',
         'iso8859_4', 'iso8859_8', 'cp1026', 'cp1252', 'cp1256', 'cp437', 'cp850', 'cp857',
         'cp863', 'cp869', 'cp949', 'iso8859_13', 'iso8859_1', 'iso8859_5', 'iso8859_9',
         'cp1140', 'cp1253', 'cp1257', 'cp500', 'cp852', 'cp860', 'cp864', 'cp874', 'cp950',
         'iso8859_14', 'iso8859_2', 'iso8859_6']

def unichar2str(u):
    if not u:
        return '0x0'
    if type(u) is not unicode:
        return '0x%x' % u
    return '0x%x' % ord(u)

mappings = {}
def genmaptable(m, encoding_map, decoding_table):
    #print encoding_map, decoding_table
    f = file(m + '-table.c', 'wb')
    f.write('#include <singlebytecodec.h>\n\n')
    f.write('static const int %s_decoding_table[%d] = {\n' % (m, len(decoding_table)))
    for i in decoding_table:
        f.write('\t0x%x,\n' % ord(i))
    f.write('};\n\n')
    f.write('static const struct encoding_map %s_encoding_map[] = {\n' % m)
    keys = encoding_map.keys()
    keys.sort()
    for k in keys:
        v = encoding_map[k]
        f.write('\t{ %s, 0x%x },\n' % (unichar2str(k), v))
    f.write('};\n\n')
    f.write('SingleByteCodecState __uniconv_%s_state = {\n' % m)
    f.write('\t"%s", %s_decoding_table, %d, %s_encoding_map, %d\n' % \
            (m, m, len(decoding_table), m, len(keys)))
    f.write('};\n\n')
    f.close()

def gensinglebytemaps(table):
    fp = file('singlebytetables.c', 'wb')
    fp.write('#include <singlebytecodec.h>\n\n')

    table.sort()
    for t in table:
        fp.write('extern SingleByteCodecState __uniconv_%s_state;\n' % t)
    fp.write('\n')
    fp.write('static SingleByteCodecState *singlebytecodecs[] = {\n')
    for t in table:
        fp.write('\t&__uniconv_%s_state,\n' % t)
    fp.write('\tNULL,\n')
    fp.write('};\n\n')
    fp.write('SingleByteCodecState** __uniconv_get_single_byte_codecs()\n')
    fp.write('{\n\treturn singlebytecodecs;\n}\n\n')
    fp.close()

encodings_table = []
for m in maps:
    encoding = getattr(__import__('encodings.%s' % m), m)
    decoding_map = {}
    decoding_table = []
    encoding_map = {}
    if hasattr(encoding, 'decoding_map'):
        decoding_map = encoding.decoding_map
    if hasattr(encoding, 'decoding_table'):
        decoding_table = list(encoding.decoding_table)
    if hasattr(encoding, 'encoding_map'):
        encoding_map =  encoding.encoding_map
    if not decoding_table and decoding_map:
        keys = decoding_map.keys()
        keys.sort()
        for k in keys:
            decoding_table[i] = decoding_map[k]
    if not encoding_map and decoding_table:
        for i in range(len(decoding_table)):
            encoding_map[decoding_table[i]] = i
    if decoding_table and encoding_map:
        mappings[m] = (encoding_map, decoding_table)

def get_ascii_mapping():
    encoding_map = {}
    decoding_table = [unicode(chr(c)) for c in range(128)]
    for i in range(128):
        encoding_map[i] = i
    return (encoding_map, decoding_table)
mappings['ascii'] = get_ascii_mapping()
#print mappings

for k, v in mappings.items():
    genmaptable(k, *v)
    encodings_table.append(k)

gensinglebytemaps(encodings_table)
