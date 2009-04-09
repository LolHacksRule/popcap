# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import os.path

def AddOptions(opts):
    from SCons.Variables.BoolVariable import BoolVariable
    if 'debug' in opts.keys ():
        return
    opts.Add('config', 'configuration used to build the framework', '')
    opts.Add(BoolVariable ('debug', 'build debug version', 'False'))
    opts.Add(BoolVariable ('release', 'build release version', 'False'))
    opts.Add(BoolVariable ('builddir', 'output to an alternative directory', 'False'))
    opts.Add(BoolVariable ('static', 'build the framework as a static library', 'True'))

def Configure(env):
    env['WIN_PROG_FLAGS'] = ''
    env.AppendUnique (DRIVERS = [], LOADERS = [], CPPDEFINES = [], BUILD_PACKAGES = [])
    env.AppendUnique (CPPPATH = [os.path.join ('#', 'extra', 'include')],
                      LIBPATH = [os.path.join ('#', 'extra', 'lib')])
    if not env.has_key ('PKGCONFIG'):
        env.Replace (PKGCONFIG = 'pkg-config')

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
