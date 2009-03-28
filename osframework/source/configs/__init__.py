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

def Configure(env):
    env.AppendUnique (DRIVERS = [], LOADERS = [], CPPDEFINES = [])
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
    if 'audiere_ccflags' in opts.keys ():
        return
    opts.Add ('audiere_ccflags', "c/c++ compiler flags for audiere.", '')
    opts.Add ('audiere_ldflags', "link flags for audiere", '')

def EnableAudiereSound (env):
    env.AppendUnique (LIBS = ['audiere']);
    env.AppendUnique (CCFLAGS = env['audiere_ccflags'].split(','),
                      LINKFLAGS = env['audiere_ldflags'].split(','))

def AudiereSoundConfigure(env):
    env.AppendUnique (DRIVERS = ['AUDIERESOUND'])
    audiere_sound = {}
    audiere_sound['ENABLE'] = EnableAudiereSound
    env.AppendUnique (AUDIERESOUND = audiere_sound)

