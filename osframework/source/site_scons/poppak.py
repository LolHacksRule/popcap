#!/usr/bin/python

import os
import sys
import struct
import datetime

POPPAK_MAGIC = 0xbac04ac0

class PopPakFileInfo:
    def __init__ (self):
        self.time = None
        self.name = None
        self.local_name = None
        self.size = None
        self.pos = None
        self.filetime = None

class PopPak:
    def __init__ (self, filename, mode = "r"):
        self.filename = filename
        self.fileinfos = []
        self.dataoffset = 0
        self.mode = mode
        self.fp = None
        self.datasize = 0

        if mode == 'w':
            mode = 'wb'
        else:
            mode = 'rb'
        self.fp = file (self.filename, mode)

        if self.mode == 'r':
            self.read ()

    def __del__ (self):
        if self.fp:
            self.fp.close ()

    def readb (self, fp):
        s = fp.read (1);
        if not s:
            return None
        return struct.unpack ('B', s)[0] ^ 0xf7

    def readl (self, fp):
        s = fp.read (4);
        if not s:
            return None
        bytes = struct.unpack ('BBBB', s)
        integer = bytes[3] << 24 | bytes[2] << 16 | \
                  bytes[1] <<  8 | bytes[0];
        return integer ^ 0xf7f7f7f7

    def readq (self, fp):
        s = fp.read (8);
        if not s:
            return None
        bytes = struct.unpack ('BBBBBBBB', s)
        quadword = bytes[7] << 46 | bytes[6] << 48 | \
                   bytes[5] << 40 | bytes[4] << 32 | \
                   bytes[3] << 24 | bytes[2] << 16 | \
                   bytes[1] <<  8 | bytes[0];
        return quadword ^ 0xf7f7f7f7f7f7f7f7

    def readstr (self, fp, size):
        s = fp.read (size);
        result = ''
        for i in range (0, len(s)):
            result += chr (struct.unpack ('B', s[i])[0] ^ 0xf7)
        return result

    def writeb (self, fp, v):
        s = struct.pack ('B', v ^ 0xf7)
        fp.write (s)

    def writel (self, fp, v):
        s = struct.pack ('<I', v ^ 0xf7f7f7f7)
        fp.write (s)

    def writeq (self, fp, v):
        s = struct.pack ('<Q', v ^ 0xf7f7f7f7f7f7f7f7)
        fp.write (s)

    def writestr (self, fp, s):
        result = ''
        for i in range (0, len(s)):
            result += struct.pack ('B', ord(s[i]) ^ 0xf7)
        fp.write(result)

    def read (self):
        fp = self.fp
        magic = self.readl (fp)
        if magic != POPPAK_MAGIC:
            fp.close ()
            raise Exception, 'Bad magic: 0x%x' % magic
        version = self.readl (fp)
        if version != 0:
            fp.close ()
            raise Exception, 'Bad version.'

        pos = 0
        self.fileinfos = []
        while True:
            flags = self.readb (fp)
            if not flags is None and flags & 0x80:
                break

            namelength = self.readb (fp)
            name = self.readstr (fp, namelength)
            size = self.readl (fp)
            filetime = self.readq (fp)
            time = (filetime - 116444736000000000) / 10000000

            name = name.replace ('\\', '/')
            info = PopPakFileInfo()
            info.time = time
            info.name = name
            info.size = size
            info.pos = pos
            info.filetime = filetime
            self.fileinfos.append(info)

            if False:
                print 'time = ', time
                print 'namelenght = ', namelength
                print 'name = ', name
                print 'size = ', size
                print 'pos = ', pos
                print 'filetime = ', filetime

            pos += size

        print 'data offset: ', self.fp.tell()
        self.dataoffset = self.fp.tell()

    def readcontent(self, info):
        self.fp.seek (info.pos + self.dataoffset, os.SEEK_SET)
        return self.readstr (self.fp, info.size)

    def extract (self, out):
        for info in self.fileinfos:
            path = info.name
            if path.startswith('./'):
                tgt = os.path.join(out, path[2:])
            else:
                tgt = os.path.join(out, path)

            tgtdir = os.path.dirname(tgt)
            if not os.path.exists(tgtdir):
                os.makedirs(tgtdir)
            fp = open(tgt, 'wb')
            fp.write(self.readcontent(info))
            fp.close()

    def add (self, filename, altfilename = None):
        if altfilename is None:
            altfilename = filename
        st = os.stat(filename)
        info = PopPakFileInfo()
        info.time = 0
        info.name = altfilename
        info.local_name = filename
        info.size = st.st_size
        info.pos = self.datasize;
        info.filetime = 0
        self.fileinfos.append(info)
        self.datasize += info.size;

    def printdir (self):
        print "%-46s %19s %12s" % ("File Name", "Modified    ", "Size")
        for info in self.fileinfos:
            date = "%d-%02d-%02d %02d:%02d:%02d" % (1970, 1, 1, 0, 0, 0)
            print "%-46s %s %12d" % (info.name, date, info.size)

    def finish (self):
        self.fp.seek (0, os.SEEK_SET)
        self.writel (self.fp, POPPAK_MAGIC)
        self.writel (self.fp, 0)
        for info in self.fileinfos:
            name = info.name.replace ('/', '\\')
            self.writeb (self.fp, 0)
            assert len(name) <= 255
            self.writeb (self.fp, len(name))
            self.writestr (self.fp, name)
            self.writel (self.fp, info.size)
            self.writeq (self.fp, 0)
        self.writeb (self.fp, 0x80)

        print 'data offset: ', self.fp.tell()

        for info in self.fileinfos:
            fp = file(info.local_name, 'rb')
            s = fp.read(info.size)
            ret = self.writestr (self.fp, s)
            fp.close()

    def close (self):
        if self.mode == 'w':
            self.finish ()
        if self.fp:
            self.fp.close ()
        self.fp = None
        self.fileinfos = []
        self.dataoffset = 0

if __name__ == '__main__':
    import sys
    import textwrap
    USAGE=textwrap.dedent("""\
        Usage:
            poppak.py -l zipfile.pak        # Show listing of a pak
            poppak.py -e zipfile.pak target # Extract pak into target dir
            poppak.py -c zipfile.pak src ... # Create pak from sources
        """)
    args = None
    if args is None:
        args = sys.argv[1:]

    if not args or args[0] not in ('-l', '-c', '-e', '-t'):
        print USAGE
        sys.exit(1)

    if args[0] == '-l':
        if len(args) != 2:
            print USAGE
            sys.exit(1)
        zf = PopPak(args[1], 'r')
        zf.printdir()
        zf.close()

    elif args[0] == '-t':
        if len(args) != 2:
            print USAGE
            sys.exit(1)
        zf = PopPak(args[1], 'r')
        zf.close()
        print "Done testing"

    elif args[0] == '-e':
        if len(args) != 3:
            print USAGE
            sys.exit(1)

        zf = PopPak (args[1], 'r')
        zf.extract (args[2])
        zf.close()

    elif args[0] == '-c':
        if len(args) < 3:
            print USAGE
            sys.exit(1)

        def addToPak(zf, path, zippath):
            if os.path.isfile(path):
                zf.add(path, zippath)
            elif os.path.isdir(path):
                for nm in os.listdir(path):
                    addToPak(zf, os.path.join(path, nm),
                             os.path.join(zippath, nm))

        zf = PopPak(args[1], 'w')
        for src in args[2:]:
            addToPak(zf, src, os.path.basename(src))
        zf.close()


