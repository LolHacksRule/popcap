# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import os.path

def AddOptions(opts):
    from SCons.Variables.BoolVariable import BoolVariable
    if 'debug' in opts.keys ():
        return
    opts.Add(BoolVariable ('debug', 'build debug version', 'False'))
    opts.Add(BoolVariable ('release', 'build release version', 'False'))
    opts.Add(BoolVariable ('builddir', 'output to an alternative directory', 'False'))

def Configure(env):
    env.AppendUnique (DRIVERS = [], LOADERS = [])
    env.AppendUnique (CPPPATH = [os.path.join ('#', 'extra', 'include')],
                      LIBPATH = [os.path.join ('#', 'extra', 'lib')])
    if not env.has_key ('PKGCONFIG'):
        env.Replace (PKGCONFIG = 'pkg-config')

def PosixModuleLoaderAddOptions (opts):
    pass

def EnablePosixModuleLoader (env):
    env.AppendUnique (LIBS = ['dl'])

def PosixModuleLoaderConfigure (env):
    env.AppendUnique (LOADERS = ['POSIXMODULELOADER'])
    dl_loader = {}
    dl_loader['ENABLE'] = EnablePosixModuleLoader
    env.AppendUnique (POSIXMODULELOADER = dl_loader)

def AudiereSoundAddOptions (opts):
    from SCons.Variables.PathVariable import PathVariable
    if 'audieredir' in opts.keys ():
        return
    opts.Add(PathVariable ('audieredir', "where the root of audiere is installed.",
                           '', PathVariable.PathAccept))
    opts.Add(PathVariable ('audiere_includes', "where the audiere includes are installed.",
                           os.path.join ('$audieredir', 'include'),
                           PathVariable.PathAccept))
    opts.Add(PathVariable ('audiere_libraries', "where the audiere library is installed.",
                           os.path.join ('$audieredir', 'lib'),
                           PathVariable.PathAccept))

def EnableAudiereSound (env):
    env.AppendUnique (LIBS = ['audiere']);

def AudiereSoundConfigure(env):
    env.AppendUnique (DRIVERS = ['AUDIERESOUND'])
    audiere_sound = {}
    audiere_sound['ENABLE'] = EnableAudiereSound
    env.AppendUnique (AUDIERESOUND = audiere_sound)

    if env.has_key ('audieredir') and env['audieredir']:
        env.AppendUnqiue (CPPPATH = ['$audiere_includes'],
                          LIBPATH = ['$audiere_libraries'])
