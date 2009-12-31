# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import os.path
import sys

opts_default = {}

def SetOptionsDefault(key, value):
    opts_default[key] = value

def GetOptionsDefault(key, default_value = None):
    if opts_default.has_key(key):
        return opts_default[key]
    return default_value

def AddOptions(opts):
    from SCons.Variables.BoolVariable import BoolVariable
    from SCons.Variables.ListVariable import ListVariable

    if 'debug' in opts.keys ():
        return

    opts.Add('config_file', 'the path to Config.py', 'Config.py')
    opts.Add('config', 'configuration used to build the framework', '')
    opts.Add(BoolVariable ('debug', 'build debug version', False))
    opts.Add(BoolVariable ('release', 'build release version', False))
    opts.Add(BoolVariable ('optimize', 'build optimize version', False))
    opts.Add(ListVariable ('optimize_level', 'the level of optimizing',
                           '2', ['0', 's', '1', '2']))
    opts.Add(BoolVariable ('builddir', 'output to an alternative directory', True))
    opts.Add(BoolVariable ('static', 'build the framework as a static library', True))
    opts.Add(BoolVariable ('strip', 'strip debug informantion from objects', True))
    opts.Add(BoolVariable ('colorize_output', 'cmake like colorize output message', True))
    opts.Add(BoolVariable ('keyboard', 'support changing focus by pressing arrow keys', True))
    opts.Add(ListVariable ('package_format', 'the format of game packages(zip, tar.gz, tar.bz2..)',
                           'none', ['zip', 'gztar', 'bztar', 'tar']))
    opts.Add('language', 'specify the language of games', 'en_US')
    opts.Add('oem', "the name of oem", 'default')
    opts.Add('target', "the name of oem target for example(olo, canmore)", '')
    opts.Add('install_prefix', 'install games into its subdirectory', '')
    opts.Add('otherdirs', 'build other components in different directories(seperated by comma)', '')

    FreeTypeAddOptions(opts)

def Configure(env):
    env['WIN_PROG_FLAGS'] = ''
    if not env.has_key('TARGET_OS'):
        env['TARGET_OS'] = sys.platform
    if not env.has_key('TARGET_PLATFORM'):
        env['TARGET_PLATFORM'] = 'pc'
    if not env.has_key('TARGET_CHIP'):
        env['TARGET_CHIP'] = 'default'
    if not env.has_key('STDCXX_LIBS'):
        env.Append(STDCXX_LIBS = [])
    if not env.has_key('MATH_LIBS'):
        env.AppendUnique(MATH_LIBS = [])
    env.AppendUnique (DRIVERS = [], LOADERS = [], CPPDEFINES = [])
    env.AppendUnique (BUILD_PACKAGES = [], PACKAGES_INFO = {}, ENABLED_PACKAGES = [])
    env.AppendUnique (CPPPATH = [os.path.join ('#', 'extra', 'include')],
                      LIBPATH = [os.path.join ('#', 'extra', 'lib')],
		      LIBS = [], CPPDEFINES = [])
    if not env.has_key ('PKGCONFIG'):
        env.Replace (PKGCONFIG = 'pkg-config')
    if env['debug']:
        env.AppendUnique(CPPDEFINES = ['SEXY_DEBUG'])
    SetupColorizeOutput(env)
    FreeTypeConfigure(env)
    fontforgeConfigure(env)

def fontforgeConfigure(env):
    if not env.WhereIs('fontforge'):
        rootdir = env.Dir('#').abspath
        if sys.platform == 'linux2':
            env.AppendENVPath('PATH',
                              os.path.join(rootdir, 'tools', 'fontforge', 'linux'))
        elif sys.platform == 'win32':
            env.AppendENVPath('PATH',
                              os.path.join(rootdir, 'tools', 'fontforge', 'win32'))

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
    pass

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
    from SCons.Variables.BoolVariable import BoolVariable

    if 'freetype' in opts.keys ():
        return
    opts.Add(BoolVariable ('freetype', 'build the freetype font driver',
                           GetOptionsDefault('freetype', False)))
    opts.Add ('freetype_ccflags', "c/c++ compiler flags for freetype.", '')
    opts.Add ('freetype_ldflags', "link flags for freetype", '')

def EnableFreeType (env):
    env.AppendUnique (LIBS = ['freetype']);
    env.AppendUnique (CCFLAGS = env['freetype_ccflags'].split(','),
                      LINKFLAGS = env['freetype_ldflags'].split(','))
    if not env['freetype_ccflags'] and not env['freetype_ldflags'] and env['FREETYPECONFIG']:
        env.ParseConfig('$FREETYPECONFIG --cflags --libs')

def FreeTypeConfigure(env):
    if not env['freetype']:
        return
    env.AppendUnique (DRIVERS = ['FREETYPEFONT'])
    freetype_font = {}
    freetype_font['ENABLE'] = EnableFreeType
    env['FREETYPECONFIG'] = 'freetype-config'
    env.AppendUnique (FREETYPEFONT = freetype_font)

def SetPackageInfo(envs, name, **kwargs):
    if not type(envs) is list:
        envs = [envs]

    for env in envs:
        info = env['PACKAGES_INFO'][name] = kwargs.copy()
        ### set defaults
        for key in ['CPPDEFINES', 'CPPPATH', 'LIBS', 'LIBPATH', 'TARGETS',
                    'OBJECTS', 'DEPENDS', 'CCFLAGS', 'CXXFLAGS', 'LINKFLAGS']:
            if not info.has_key(key):
                info[key] = []

def EnablePackage(env, name, what = None, append = True, unique = False, **kwargs):
    ### set env
    if not type(env) is list:
        envs = [env]
    else:
        envs = env

    ### get package info
    info = GetPackageInfo(envs[0], name)
    if not info:
        return
    if what is None:
        what = info.keys()
    args = {}
    for key in what:
        if key not in info:
            continue
        if key in ['TARGETS']:
            continue
        args[key] = info[key]

    ### setup package variables
    for env in envs:
        if append:
            if unique:
                env.AppendUnique(**args)
            else:
                env.Append(**args)
        else:
            if unique:
                env.PrependUnique(**args)
            else:
                env.Prepend(**args)

        env.AppendUnique(ENABLED_PACKAGES = [name])

def EnablePackages(envs, packages, what = None, append = True, unique = False):
    if not type(packages) is list:
        packages = [packages]
    for package in packages:
        EnablePackage(envs, package, what, append, unique)

def GetPackageTargets(env, packages):
    if not type(packages) is list:
        packages = [packages]

    targets = []
    for package in packages:
        comp = GetPackageInfo(env, package)
        if not comp:
            continue
        targets += comp['TARGETS']
    return targets

def GetEnabledPackageTargets(env):
    return GetPackageTargets(env, env['ENABLED_PACKAGES'])

def GetPackageDepends(env, packages):
    if not type(packages) is list:
        packages = [packages]
    libs = []
    libpath = []
    cpppath = []
    cppdefines = []
    ccflags = []
    cxxflags = []
    linkflags = []
    depends = []
    targets = []

    comps = []
    for package in packages:
        comps.append(GetPackageInfo(env, package))
        if comps[-1]:
            depends.append(package)

    for comp in comps:
        if not comp:
            continue
        libs += comp['LIBS']
        libpath += comp['LIBPATH']
        cpppath += comp['CPPPATH']
        cppdefines += comp['CPPDEFINES']
        ccflags += comp['CCFLAGS']
        cxxflags += comp['CXXFLAGS']
        linkflags += comp['LINKFLAGS']
        depends += comp['DEPENDS']
        targets += comp['TARGETS']

    result = { 'LIBS': libs, 'LIBPATH': libpath,
               'CPPDEFINES': cppdefines, 'CPPPATH': cpppath,
               'CCFLAGS': ccflags, 'CXXFLAGS': cxxflags,
               'LINKFLAGS': linkflags, 'DEPENDS': depends }
    return result, targets

def GetPackageInfo(env, name):
    if not env['PACKAGES_INFO'].has_key(name):
        return {}
    return env['PACKAGES_INFO'][name]

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

    game_gen_ver_message = '%sGenerating version file:%s "%s$TARGET%s" %s($SOURCES)%s' % \
       (colors['red'], colors['end'], colors['purple'], colors['end'],
        colors['yellow'], colors['end'])
    archive_message = '%sCompressing:%s "%s$TARGETS%s"' % \
       (colors['red'], colors['end'], colors['purple'], colors['end'])
    md5sum_message = '%sCaculating md5sum:%s "%s$TARGETS%s"' % \
       (colors['red'], colors['end'], colors['purple'], colors['end'])

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
    env['GAMEGENVERSIONSTR'] = game_gen_ver_message
    env['ARCHIVESTR'] = archive_message
    env['MD5SUMSTR'] = md5sum_message

def QuoteStr(s):
    return '\\"' + s + '\\"'
