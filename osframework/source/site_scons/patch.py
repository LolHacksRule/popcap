""" Patch utility to apply unified diffs """
""" Brute-force line-by-line parsing

    Project home: http://code.google.com/p/python-patch/

    $Id$
    $HeadURL$
"""

__author__ = "techtonik.rainforce.org"
__version__ = "9.08-1"

import copy
import logging
import re
# cStringIO doesn't support unicode in 2.5
from StringIO import StringIO
from logging import debug, info, warning

from os.path import exists, isfile
from os import unlink

debugmode = False



def fromfile(filename):
  """ Parse patch file and return Patch() object
  """

  info("reading patch from file %s" % filename)
  fp = open(filename, "rb")
  patch = Patch(fp)
  fp.close()
  return patch


def fromstring(s):
  """ Parse text string and return Patch() object
  """

  return Patch(
           StringIO.StringIO(s)
         )



class HunkInfo(object):
  """ Parsed hunk data container (hunk starts with @@ -R +R @@) """

  def __init__(self):
    self.startsrc=None
    self.linessrc=None
    self.starttgt=None
    self.linestgt=None
    self.invalid=False
    self.text=[]

  def copy(self):
    return copy.copy(self)

#  def apply(self, estream):
#    """ write hunk data into enumerable stream
#        return strings one by one until hunk is
#        over
#
#        enumerable stream are tuples (lineno, line)
#        where lineno starts with 0
#    """
#    pass



class Patch(object):

  def __init__(self, stream=None):

    # define Patch data members
    # table with a row for every source file

    #: list of source filenames
    self.source=None
    self.target=None
    #: list of lists of hunks
    self.hunks=None
    #: file endings statistics for every hunk
    self.hunkends=None

    if stream:
      self.parse(stream)

  def copy(self):
    return copy.copy(self)

  def parse(self, stream):
    """ parse unified diff """
    self.source = []
    self.target = []
    self.hunks = []
    self.hunkends = []

    # define possible file regions that will direct the parser flow
    header = False    # comments before the patch body
    filenames = False # lines starting with --- and +++

    hunkhead = False  # @@ -R +R @@ sequence
    hunkbody = False  #
    hunkskip = False  # skipping invalid hunk mode

    header = True
    lineends = dict(lf=0, crlf=0, cr=0)
    nextfileno = 0
    nexthunkno = 0    #: even if index starts with 0 user messages number hunks from 1

    # hunkinfo holds parsed values, hunkactual - calculated
    hunkinfo = HunkInfo()
    hunkactual = dict(linessrc=None, linestgt=None)

    fe = enumerate(stream)
    for lineno, line in fe:

      # analyze state
      if header and line.startswith("--- "):
        header = False
        # switch to filenames state
        filenames = True
      #: skip hunkskip and hunkbody code until you read definition of hunkhead
      if hunkbody:
        # process line first
        if re.match(r"^[- \+\\]", line):
            # gather stats about line endings
            if line.endswith("\r\n"):
              self.hunkends[nextfileno-1]["crlf"] += 1
            elif line.endswith("\n"):
              self.hunkends[nextfileno-1]["lf"] += 1
            elif line.endswith("\r"):
              self.hunkends[nextfileno-1]["cr"] += 1

            if line.startswith("-"):
              hunkactual["linessrc"] += 1
            elif line.startswith("+"):
              hunkactual["linestgt"] += 1
            elif not line.startswith("\\"):
              hunkactual["linessrc"] += 1
              hunkactual["linestgt"] += 1
            hunkinfo.text.append(line)
            # todo: handle \ No newline cases
        else:
            warning("invalid hunk no.%d at %d for target file %s" % (nexthunkno, lineno+1, self.target[nextfileno-1]))
            # add hunk status node
            self.hunks[nextfileno-1].append(hunkinfo.copy())
            self.hunks[nextfileno-1][nexthunkno-1]["invalid"] = True
            # switch to hunkskip state
            hunkbody = False
            hunkskip = True

        # check exit conditions
        if hunkactual["linessrc"] > hunkinfo.linessrc or hunkactual["linestgt"] > hunkinfo.linestgt:
            warning("extra hunk no.%d lines at %d for target %s" % (nexthunkno, lineno+1, self.target[nextfileno-1]))
            # add hunk status node
            self.hunks[nextfileno-1].append(hunkinfo.copy())
            self.hunks[nextfileno-1][nexthunkno-1]["invalid"] = True
            # switch to hunkskip state
            hunkbody = False
            hunkskip = True
        elif hunkinfo.linessrc == hunkactual["linessrc"] and hunkinfo.linestgt == hunkactual["linestgt"]:
            self.hunks[nextfileno-1].append(hunkinfo.copy())
            # switch to hunkskip state
            hunkbody = False
            hunkskip = True

            # detect mixed window/unix line ends
            ends = self.hunkends[nextfileno-1]
            if ((ends["cr"]!=0) + (ends["crlf"]!=0) + (ends["lf"]!=0)) > 1:
              warning("inconsistent line ends in patch hunks for %s" % self.source[nextfileno-1])
            if debugmode:
              debuglines = dict(ends)
              debuglines.update(file=self.target[nextfileno-1], hunk=nexthunkno)
              debug("crlf: %(crlf)d  lf: %(lf)d  cr: %(cr)d\t - file: %(file)s hunk: %(hunk)d" % debuglines)

      if hunkskip:
        match = re.match("^@@ -(\d+)(,(\d+))? \+(\d+)(,(\d+))?", line)
        if match:
          # switch to hunkhead state
          hunkskip = False
          hunkhead = True
        elif line.startswith("--- "):
          # switch to filenames state
          hunkskip = False
          filenames = True
          if debugmode and len(self.source) > 0:
            debug("- %2d hunks for %s" % (len(self.hunks[nextfileno-1]), self.source[nextfileno-1]))

      if filenames:
        if line.startswith("--- "):
          if nextfileno in self.source:
            warning("skipping invalid patch for %s" % self.source[nextfileno])
            del self.source[nextfileno]
            # double source filename line is encountered
            # attempt to restart from this second line
          re_filename = "^--- ([^\t]+)"
          match = re.match(re_filename, line)
          # todo: support spaces in filenames
          if match:
            self.source.append(match.group(1).strip())
          else:
            warning("skipping invalid filename at line %d" % lineno)
            # switch back to header state
            filenames = False
            header = True
        elif not line.startswith("+++ "):
          if nextfileno in self.source:
            warning("skipping invalid patch with no target for %s" % self.source[nextfileno])
            del self.source[nextfileno]
          else:
            # this should be unreachable
            warning("skipping invalid target patch")
          filenames = False
          header = True
        else:
          if nextfileno in self.target:
            warning("skipping invalid patch - double target at line %d" % lineno)
            del self.source[nextfileno]
            del self.target[nextfileno]
            nextfileno -= 1
            # double target filename line is encountered
            # switch back to header state
            filenames = False
            header = True
          else:
            re_filename = "^\+\+\+ ([^\t]+)"
            match = re.match(re_filename, line)
            if not match:
              warning("skipping invalid patch - no target filename at line %d" % lineno)
              # switch back to header state
              filenames = False
              header = True
            else:
              self.target.append(match.group(1))
              nextfileno += 1
              # switch to hunkhead state
              filenames = False
              hunkhead = True
              nexthunkno = 0
              self.hunks.append([])
              self.hunkends.append(lineends.copy())
              continue

      if hunkhead:
        match = re.match("^@@ -(\d+)(,(\d+))? \+(\d+)(,(\d+))?", line)
        if not match:
          if nextfileno-1 not in self.hunks:
            warning("skipping invalid patch with no hunks for file %s" % self.target[nextfileno-1])
            # switch to header state
            hunkhead = False
            header = True
            continue
          else:
            # switch to header state
            hunkhead = False
            header = True
        else:
          hunkinfo.startsrc = int(match.group(1))
          hunkinfo.linessrc = int(match.group(3) if match.group(3) else 1)
          hunkinfo.starttgt = int(match.group(4))
          hunkinfo.linestgt = int(match.group(6) if match.group(6) else 1)
          hunkinfo.invalid = False
          hunkinfo.text = []

          hunkactual["linessrc"] = hunkactual["linestgt"] = 0

          # switch to hunkbody state
          hunkhead = False
          hunkbody = True
          nexthunkno += 1
          continue
    else:
      if not hunkskip:
        warning("patch file incomplete - %s" % filename)
        # sys.exit(?)
      else:
        # duplicated message when an eof is reached
        if debugmode and len(self.source) > 0:
            debug("- %2d hunks for %s" % (len(self.hunks[nextfileno-1]), self.source[nextfileno-1]))

    info("total files: %d  total hunks: %d" % (len(self.source), sum(len(hset) for hset in self.hunks)))

  def apply(self, basedir = '.', strip = 0):
    """ apply parsed patch """

    def strippath(path, basedir, strip):
      import os.path
      path = os.path.normpath(path)
      if strip:
        path = os.path.sep.join(path.split(os.path.sep)[strip:])
      if basedir == '.':
        return path
      return os.path.join(basedir, path)

    total = len(self.source)
    for fileno, filename in enumerate(self.source):

      f2patch = strippath(filename, basedir, strip)
      if not exists(f2patch):
        f2patch = strippath(self.target[fileno], basedir, strip)
        if not exists(f2patch):
          warning("source/target file does not exist\n--- %s\n+++ %s" % (filename, self.target[fileno]))
          continue
      if not isfile(f2patch):
        warning("not a file - %s" % f2patch)
        continue
      filename = f2patch

      info("processing %d/%d:\t %s" % (fileno+1, total, filename))

      # validate before patching
      f2fp = open(filename)
      hunkno = 0
      hunk = self.hunks[fileno][hunkno]
      hunkfind = []
      hunkreplace = []
      validhunks = 0
      canpatch = False
      for lineno, line in enumerate(f2fp):
        if lineno+1 < hunk.startsrc:
          continue
        elif lineno+1 == hunk.startsrc:
          hunkfind = [x[1:].rstrip("\r\n") for x in hunk.text if x[0] in " -"]
          hunkreplace = [x[1:].rstrip("\r\n") for x in hunk.text if x[0] in " +"]
          #pprint(hunkreplace)
          hunklineno = 0

          # todo \ No newline at end of file

        # check hunks in source file
        if lineno+1 < hunk.startsrc+len(hunkfind)-1:
          if line.rstrip("\r\n") == hunkfind[hunklineno]:
            hunklineno+=1
          else:
            debug("hunk no.%d doesn't match source file %s" % (hunkno+1, filename))
            # file may be already patched, but we will check other hunks anyway
            hunkno += 1
            if hunkno < len(self.hunks[fileno]):
              hunk = self.hunks[fileno][hunkno]
              continue
            else:
              break

        # check if processed line is the last line
        if lineno+1 == hunk.startsrc+len(hunkfind)-1:
          debug("file %s hunk no.%d -- is ready to be patched" % (filename, hunkno+1))
          hunkno+=1
          validhunks+=1
          if hunkno < len(self.hunks[fileno]):
            hunk = self.hunks[fileno][hunkno]
          else:
            if validhunks == len(self.hunks[fileno]):
              # patch file
              canpatch = True
              break
      else:
        if hunkno < len(self.hunks[fileno]):
          warning("premature end of source file %s at hunk %d" % (filename, hunkno+1))

      f2fp.close()

      if validhunks < len(self.hunks[fileno]):
        if self.checkfile(filename, self.hunks[fileno]):
          warning("already patched  %s" % filename)
        else:
          warning("source file is different - %s" % filename)
      if canpatch:
        backupname = filename+".orig"
        if exists(backupname):
          warning("can't backup original file to %s - aborting" % backupname)
        else:
          import shutil
          shutil.move(filename, backupname)
          if self.writehunks(backupname, filename, self.hunks[fileno]):
            warning("successfully patched %s" % filename)
            unlink(backupname)
          else:
            warning("error patching file %s" % filename)
            shutil.copy(filename, filename+".invalid")
            warning("invalid version is saved to %s" % filename+".invalid")
            # todo: proper rejects
            shutil.move(backupname, filename)

    # todo: check for premature eof


  def checkfile(self, filename, hunks):
    """ Check if file is alrady patched """
    matched = True
    fp = open(filename)

    class NoMatch(Exception):
      pass

    lineno = 1
    line = fp.readline()
    hno = None
    try:
      if not len(line):
        raise NoMatch
      for hno, h in enumerate(hunks):
        # skip to line just before hunk starts
        while lineno < h.starttgt-1:
          line = fp.readline()
          lineno += 1
          if not len(line):
            raise NoMatch
        for hline in h.text:
          # todo: \ No newline at the end of file
          if not hline.startswith("-") and not hline.startswith("\\"):
            line = fp.readline()
            lineno += 1
            if not len(line):
              raise NoMatch
            if line.rstrip("\r\n") != hline[1:].rstrip("\r\n"):
              warning("file is not patched - failed hunk: %d" % (hno+1))
              raise NoMatch
    except NoMatch:
      matched = False
      # todo: display failed hunk, i.e. expected/found

    fp.close()
    return matched


  def patchstream(self, instream, hunks):
    """ Generator that yields stream patched with hunks iterable

        Converts lineends in hunk lines to the best suitable format
        autodetected from input
    """

    # todo: At the moment substituted lineends may not be the same
    #       at the start and at the end of patching. Also issue a
    #       warning/throw about mixed lineends (is it really needed?)

    hunks = iter(hunks)

    srclineno = 1

    lineends = {'\n':0, '\r\n':0, '\r':0}
    def get_line():
      """
      local utility function - return line from source stream
      collecting line end statistics on the way
      """
      line = instream.readline()
        # 'U' mode works only with text files
      if line.endswith("\r\n"):
        lineends["\r\n"] += 1
      elif line.endswith("\n"):
        lineends["\n"] += 1
      elif line.endswith("\r"):
        lineends["\r"] += 1
      return line

    for hno, h in enumerate(hunks):
      debug("hunk %d" % (hno+1))
      # skip to line just before hunk starts
      while srclineno < h.startsrc:
        yield get_line()
        srclineno += 1

      for hline in h.text:
        # todo: check \ No newline at the end of file
        if hline.startswith("-") or hline.startswith("\\"):
          get_line()
          srclineno += 1
          continue
        else:
          if not hline.startswith("+"):
            get_line()
            srclineno += 1
          line2write = hline[1:]
          # detect if line ends are consistent in source file
          if sum([bool(lineends[x]) for x in lineends]) == 1:
            newline = [x for x in lineends if lineends[x] != 0][0]
            yield line2write.rstrip("\r\n")+newline
          else: # newlines are mixed
            yield line2write

    for line in instream:
      yield line


  def writehunks(self, srcname, tgtname, hunks):
    src = open(srcname, "rb")
    tgt = open(tgtname, "wb")

    debug("processing target file %s" % tgtname)

    tgt.writelines(self.patchstream(src, hunks))

    tgt.close()
    src.close()
    return True






from optparse import OptionParser
from os.path import exists
import sys

if __name__ == "__main__":
  opt = OptionParser(usage="%prog [options] unipatch-file", version="python-patch %s" % __version__)
  opt.add_option("-d", action="store_true", dest="debugmode", help="debug mode")
  opt.add_option("-p", dest="strip", type="int", default=0, help="trip NUM leading components from file names")
  opt.add_option("-C", dest="basedir", default=".", help="Change to specified directory before applying the patch")
  (options, args) = opt.parse_args()

  if not args:
    opt.print_version()
    print("")
    opt.print_help()
    sys.exit()
  debugmode = options.debugmode
  patchfile = args[0]
  if not exists(patchfile) or not isfile(patchfile):
    sys.exit("patch file does not exist - %s" % patchfile)


  if debugmode:
    logging.basicConfig(level=logging.DEBUG, format="%(levelname)8s %(message)s")
  else:
    logging.basicConfig(level=logging.INFO, format="%(message)s")



  patch = fromfile(patchfile)
  #pprint(patch)
  patch.apply(options.basedir, options.strip)

  # todo: document and test line ends handling logic - patch.py detects proper line-endings
  #       for inserted hunks and issues a warning if patched file has incosistent line ends
