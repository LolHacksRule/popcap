# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import os.path
import sys

def AddOptions(opts):
    from SCons.Variables.BoolVariable import BoolVariable
    if 'debug' in opts.keys ():
        return
    opts.Add('config', 'configuration used to build the framework', '')
    opts.Add(BoolVariable ('debug', 'build debug version', False))
    opts.Add(BoolVariable ('release', 'build release version', False))
    opts.Add(BoolVariable ('builddir', 'output to an alternative directory', False))
    opts.Add(BoolVariable ('static', 'build the framework as a static library', True))
    opts.Add(BoolVariable ('strip', 'strip debug informantion from objects', True))
    opts.Add(BoolVariable ('colorize_output', 'cmake like colorize output message', True))
    opts.Add(BoolVariable ('keyboard', 'support changing focus by pressing arrow keys', True))
    opts.Add('language', 'specify the language of games', 'en_US')
    opts.Add('oem', "the name of oem", 'default')
    opts.Add('target', "the name of oem target for example(olo, canmore)", '')
    opts.Add('install_prefix', 'install games into its subdirectory', '')
    opts.Add('otherdirs', 'build other components in different directories(seperated by comma)', '')

def Configure(env):
    env['WIN_PROG_FLAGS'] = ''
    env['TARGET_OS'] = sys.platform
    env['TARGET_PLATFORM'] = 'pc'
    env.AppendUnique (DRIVERS = [], LOADERS = [], CPPDEFINES = [], BUILD_PACKAGES = [])
    env.AppendUnique (CPPPATH = [os.path.join ('#', 'extra', 'include')],
                      LIBPATH = [os.path.join ('#', 'extra', 'lib')])
    if not env.has_key ('PKGCONFIG'):
        env.Replace (PKGCONFIG = 'pkg-config')
    if env['debug']:
        env.AppendUnique(CPPDEFINES = ['SEXY_DEBUG'])
    SetupColorizeOutput(env)

def PosixModuleLoaderAddOptions (opts):
    pass

def EnablePosixModuleLoader (env):
    env.AppendUnique (LIBS = ['dl'])

def PosixModuleLoaderConfigure (env):
    env.AppendUnique (LOADERS = ['POSIX_MODULE_LOADER'])
    dl_loader = {}
    dl_loader['ENABLE'] = EnablePosixModuleLoader
    env.AppendUnique (POSIX_MODULE_LOADER = dl_loader)

def Win32ModuleLoaderAddOptions (opts):
    pass

def EnableWin32ModuleLoader (env):
    env.AppendUnique (LIBS = ['dl'])

def Win32ModuleLoaderConfigure (env):
    env.AppendUnique (LOADERS = ['WIN32_MODULE_LOADER'])
    dl_loader = {}
    dl_loader['ENABLE'] = EnableWin32ModuleLoader
    env.AppendUnique (WIN32_MODULE_LOADER = dl_loader)

def AudiereSoundAddOptions (opts):
    from SCons.Variables.PathVariable import PathVariable
    from SCons.Variables.BoolVariable import BoolVariable
    if 'audiere_ccflags' in opts.keys ():
        return
    opts.Add ('audiere_ccflags', "c/c++ compiler flags for audiere.", '')
    opts.Add ('audiere_ldflags', "link flags for audiere", '')
    opts.Add(BoolVariable ('static_audiere', 'build audiere as a static library', 'False'))

def EnableAudiereSound (env):
    env.AppendUnique (LIBS = ['audiere']);
    if env['static_audiere']:
        env.AppendUnique (LIBS = ['vorbisfile', 'vorbis', 'ogg'])
        if 'linux' in env['config']:
            env.AppendUnique (LIBS = ['asound'])
        elif 'darwin' in env['config']:
            env.AppendUnique (CPPDEFINES = ['HAVE_CORE_AUDIO'],
                              LINKFLAGS = [('-framework', 'CoreAudio'),
                                           ('-framework', 'Cocoa'),
                                           ('-framework', 'AudioUnit'),
                                           ('-framework', 'AudioToolbox')],
                              CCFLAGS = [('-framework', 'CoreAudio')])
        elif 'win32' in env['config'] or 'mingw' in env['config']:
            pass
    else:
        env.AppendUnique (CCFLAGS = env['audiere_ccflags'].split(','),
                          LINKFLAGS = env['audiere_ldflags'].split(','))

def AudiereSoundConfigure(env):
    env.AppendUnique (DRIVERS = ['AUDIERESOUND'])
    audiere_sound = {}
    audiere_sound['ENABLE'] = EnableAudiereSound
    env.AppendUnique (AUDIERESOUND = audiere_sound)

def FreeTypeAddOptions (opts):
    from SCons.Variables.PathVariable import PathVariable
    if 'freetype_ccflags' in opts.keys ():
        return
    opts.Add ('freetype_ccflags', "c/c++ compiler flags for freetype.", '')
    opts.Add ('freetype_ldflags', "link flags for freetype", '')

def EnableFreeType (env):
    env.AppendUnique (LIBS = ['freetype']);
    env.AppendUnique (CCFLAGS = env['freetype_ccflags'].split(','),
                      LINKFLAGS = env['freetype_ldflags'].split(','))
    if not env['freetype_ccflags'] and not env['freetype_ldflags'] and env['FREETYPECONFIG']:
        env.ParseConfig('$FREETYPECONFIG --cflags --libs')

def FreeTypeConfigure(env):
    env.AppendUnique (DRIVERS = ['FREETYPEFONT'])
    freetype_font = {}
    freetype_font['ENABLE'] = EnableFreeType
    env['FREETYPECONFIG'] = 'freetype-config'
    env.AppendUnique (FREETYPEFONT = freetype_font)

colors = {}
colors['cyan']   = '\033[36m'
colors['purple'] = '\033[35m'
colors['blue']   = '\033[34m'
colors['green']  = '\033[32m'
colors['yellow'] = '\033[33m'
colors['red']    = '\033[31m'
colors['end']    = '\033[0m'

if sys.platform == 'win32':
  try:
      import ctypes
      has_win32_color_console = True
  except:
      has_win32_color_console = False
else:
    has_win32_color_console = False

if has_win32_color_console:
  # Constants from the Windows API
  STD_OUTPUT_HANDLE     = -11
  FOREGROUND_BLUE       = 1
  FOREGROUND_GREEN	= 2
  FOREGROUND_RED	= 4
  FOREGROUND_INTENSITY	= 8
  BACKGROUND_BLUE	= 16
  BACKGROUND_GREEN	= 32
  BACKGROUND_RED	= 64
  BACKGROUND_INTENSITY	= 128

  def get_csbi_attributes(handle):
    # Based on IPython's winconsole.py, written by Alexander Belchenko
    import struct
    csbi = ctypes.create_string_buffer(22)
    res = ctypes.windll.kernel32.GetConsoleScreenBufferInfo(handle, csbi)
    if not res:
        return None

    (bufx, bufy, curx, cury, wattr,
     left, top, right, bottom, maxx, maxy) = struct.unpack("hhhhHhhhhhh", csbi.raw)
    return wattr

  handle = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
  reset = get_csbi_attributes(handle)

  if reset is None:
      has_win32_color_console = False
  else:
      seqmap = {}
      seqmap[colors['red']] = FOREGROUND_RED
      seqmap[colors['green']] = FOREGROUND_GREEN
      seqmap[colors['blue']] = FOREGROUND_BLUE | FOREGROUND_INTENSITY
      seqmap[colors['end']] = reset
      seqmap[colors['cyan']] = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
      seqmap[colors['purple']] = FOREGROUND_RED | FOREGROUND_BLUE
      seqmap[colors['yellow']] = FOREGROUND_RED | FOREGROUND_GREEN

def Win32PrintCmdFunc(s, target, source, env):
    import re
    startpos = 0
    for m in re.finditer(r'\033\[\d*m', s):
        ###print the string
        sys.stdout.write(s[startpos:m.start()])
        ###set attribute
        seq = m.group()
        if seqmap.has_key(seq):
            ctypes.windll.kernel32.SetConsoleTextAttribute(handle, seqmap[seq])
        startpos = m.end()
    if startpos == 0:
        sys.stdout.write(s)
    sys.stdout.write('\n')

def SetupColorizeOutput(env):
    if not env['colorize_output']:
        return

    #If the output is not a terminal, remove the colors
    supported_terms = ['xterm', 'xterm-color', 'msys']
    color_terminal = True
    if sys.stdout.isatty():
        if sys.platform == 'win32':
            if not has_win32_color_console and \
               not os.getenv('TERM') in supported_terms:
                color_terminal = False
        elif not os.getenv('TERM') in supported_terms:
            color_terminal = False
    else:
        color_terminal = False
    if not color_terminal:
        for key, value in colors.iteritems():
            colors[key] = ''

    if sys.stdout.isatty() and has_win32_color_console:
        env['PRINT_CMD_LINE_FUNC'] = Win32PrintCmdFunc

    compile_source_message = '%sCompiling object %s==> %s$SOURCE%s' % \
       (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

    compile_shared_source_message = '%sCompiling shared object %s==> %s$SOURCE%s' % \
       (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

    link_program_message = '%sLinking Program %s==> %s$TARGET%s' % \
       (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    link_library_message = '%sLinking Static Library %s==> %s$TARGET%s' % \
       (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    ranlib_library_message = '%sRanlib Library %s==> %s$TARGET%s' % \
       (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    link_shared_library_message = '%sLinking Shared Library %s==> %s$TARGET%s' % \
       (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    java_library_message = '%sCreating Java Archive %s==> %s$TARGET%s' % \
       (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    install_message = '%sInstall file:%s "%s$SOURCE%s" as "%s$TARGET%s"' % \
       (colors['cyan'], colors['end'], colors['purple'], colors['end'],
        colors['yellow'], colors['end'])

    env['CXXCOMSTR'] = compile_source_message,
    env['CCCOMSTR'] = compile_source_message,
    env['SHCCCOMSTR'] = compile_shared_source_message,
    env['SHCXXCOMSTR'] = compile_shared_source_message,
    env['ARCOMSTR'] = link_library_message,
    env['RANLIBCOMSTR'] = ranlib_library_message,
    env['SHLINKCOMSTR'] = link_shared_library_message,
    env['LINKCOMSTR'] = link_program_message,
    env['JARCOMSTR'] = java_library_message,
    env['JAVACCOMSTR'] = compile_source_message
    env['INSTALLSTR'] = install_message

