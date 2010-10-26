#!/usr/bin/python
import os
import sys
import polib
from xml.dom.minidom import Document
from xml.dom import minidom

def write_entry(entry, doc, root, compat = False):
    msg = doc.createElement("message")

    ### obsolete
    if entry.obsolete:
        msg.setAttribute('obsolete', 'true')

    ### occurrences
    if not compat:
        for occur in entry.occurrences:
            occurrence = doc.createElement("occurrence")
            text_node = doc.createTextNode(occur[0])
            occurrence.appendChild(text_node)
            occurrence.setAttribute("linenum", str(occur[1]))
            msg.appendChild(occurrence)

    ### comment
    if not compat and entry.comment:
        comment = doc.createElement("comment")
        text_node = doc.createTextNode(entry.comment)
        comment.appendChild(text_node)
        msg.appendChild(comment)

    ### tcomment
    if not compat and entry.tcomment:
        comment = doc.createElement("tcomment")
        text_node = doc.createTextNode(entry.tcomment)
        comment.appendChild(text_node)
        msg.appendChild(comment)

    ### flags
    if not compat and entry.flags:
        for flag in entry.flags:
            flag_node = doc.createElement("flag")
            text_node = doc.createTextNode(flag)
            flag_node.appendChild(text_node)
            msg.appendChild(flag_node)

    ### msgctxt
    if not compat and entry.msgctxt:
        msgctxt = doc.createElement("msgctxt")
        text_node = doc.createTextNode(entry.msgctxt)
        msgctxt.appendChild(text_node)
        msg.appendChild(msgctxt)

    ### msgid
    msgid = doc.createElement("msgid")
    #text_node = doc.createTextNode(entry.msgid)
    text_node = doc.createCDATASection(entry.msgid)
    msgid.appendChild(text_node)
    msg.appendChild(msgid)

    ### msgid_plural
    if entry.msgid_plural:
        msgid = doc.createElement("msgid_plural")
        #text_node = doc.createTextNode(entry.msgid_plural)
        text_node = doc.createCDATASection(entry.msgid_plural)
        msgid.appendChild(text_node)
        msg.appendChild(msgid)

    if entry.msgstr_plural:
        ### msgstr_plural
        msgstrs = entry.msgstr_plural
        keys = list(msgstrs)
        keys.sort()
        for index in keys:
            msgstr = doc.createElement("msgstr")
            msgstr.setAttribute('index', str(index))
            #text_node = doc.createTextNode(msgstrs[index])
            text_node = doc.createCDATASection(msgstrs[index])
            msgstr.appendChild(text_node)
            msg.appendChild(msgstr)
    else:
        ### msgstr
        msgstr = doc.createElement("msgstr")
        #text_node = doc.createTextNode(entry.msgstr)
        text_node = doc.createCDATASection(entry.msgstr)
        msgstr.appendChild(text_node)
        msg.appendChild(msgstr)

    ### append the message element
    root.appendChild(msg)

def po2xml(f, t):
    po = polib.pofile (f)

    # Create the <po> base element
    doc = Document()
    root = doc.createElement("po")
    doc.appendChild(root)

    ### header
    header = doc.createElement("header")
    text_node = doc.createCDATASection(po.header)
    header.appendChild(text_node)
    root.appendChild(header)

    ### metadata and entries
    entries = [po.metadata_as_entry()]
    entries += [e for e in po if not e.obsolete]
    for entry in entries:
        write_entry(entry, doc, root)
    for entry in po.obsolete_entries():
        write_entry(entry, doc, root)

    file(t, 'wb').write(doc.toprettyxml(indent = "  ", encoding = 'utf-8'))

def findCDataChildNode(node):
    #from minidom import Node
    for child in node.childNodes:
        if child.nodeType == minidom.Node.CDATA_SECTION_NODE:
            return child
    return None

def findTexthildNode(node):
    #from minidom import Node
    for child in node.childNodes:
        if child.nodeType == minidom.Node.TEXT_NODE:
            return child
    return None

def read_entry(node):
    entry = polib.POEntry()

    ### comment
    comments = node.getElementsByTagName('comment')
    if comments:
        data = findTexthildNode(comments[0]).data.strip()
        entry.comment = data

    ### tcomment
    tcomments = node.getElementsByTagName('tcomment')
    if tcomments:
        data = findTexthildNode(tcomments[0]).data.strip()
        entry.tcomment = data

    ### occurrences
    occurs = node.getElementsByTagName('occurrence')
    for occur in occurs:
        linenum = int(occur.getAttribute('linenum'))
        filepath = findTexthildNode(occur).data.strip()
        entry.occurrences.append((filepath, linenum))

    ### flags
    flags = node.getElementsByTagName('flag')
    for flag in flags:
        data = findTexthildNode(flag).data.strip()
        entry.flags.append(data)

    ### msgctxt
    msgctxts = node.getElementsByTagName('msgctxt')
    if msgctxts:
        data = findTexthildNode(msgctxts[0]).data.strip()
        entry.msgctxt = data

    ### msgid
    msgids = node.getElementsByTagName('msgid')
    if msgids and findCDataChildNode(msgids[0]):
        data = findCDataChildNode(msgids[0]).data.strip()
        entry.msgid = data

    ### msgid_plural
    msgids = node.getElementsByTagName('msgid_plural')
    if msgids:
        data = findCDataChildNode(msgids[0]).data.strip()
        entry.msgid_plural = data

    ### msgstr
    msgstrs = node.getElementsByTagName('msgstr')
    if msgstrs:
        if not msgstrs[0].hasAttribute('index'):
            child = findCDataChildNode(msgstrs[0])
            if not child:
                data = ''
            else:
                data = child.data.strip()
            entry.msgstr = data
        else:
            for msgstr in msgstrs:
                assert (msgstr.hasAttribute('index'))
                index = int(msgstr.getAttribute('index'))
                child = findCDataChildNode(msgstr)
                if not child:
                    data = ''
                else:
                    data = child.data.strip()
                entry.msgstr_plural[index] = data

    return entry

def xml2po(f, t):
    doc = minidom.parse(f)
    pofile = polib.POFile()

    ### read header
    node = doc.getElementsByTagName('header')
    cdata = findCDataChildNode(node[0])
    pofile.header = cdata.data

    ### read messages
    messages = doc.getElementsByTagName('message')
    for node in messages:
        pofile.append(read_entry(node))

    ### handle metedata
    firstentry = pofile[0]
    if firstentry.msgid == '':
            # remove the entry
            firstentry = pofile.pop(0)
            pofile.metadata_is_fuzzy = firstentry.flags
            key = None
            for msg in firstentry.msgstr.splitlines():
                try:
                    key, val = msg.split(':', 1)
                    pofile.metadata[key] = val.strip()
                except:
                    if key is not None:
                        pofile.metadata[key] += '\n'+ msg.strip()
    pofile.save(t)

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        f = sys.argv[1]
        if len(sys.argv) > 2:
            t = sys.argv[2]
        else:
            t = None
            fn, ext = os.path.splitext(f)
            if ext == '.po':
                t = fn + '.xml'
            elif ext == '.xml':
                t = fn + '.po'
            if not t:
                print 'Bad agument'
                os.exit(-1)
        print 'converting %s to %s' % (f, t)
        if f.find('.po') >= 0:
            po2xml(f, t)
        else:
            xml2po(f, t)
    else:
        print 'Usage: po2xml from [to]'

    sys.exit (0)

