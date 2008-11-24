# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import os.path

def AddOptions(opts):
    from SCons.Variables.BoolVariable import BoolVariable
    if 'debug' in opts.keys ():
        return
    opts.Add(BoolVariable ('debug', 'build debug version', 'no'))
    opts.Add(BoolVariable ('release', 'build release version', 'no'))

def Configure(env):
    env.AppendUnique (DRIVERS = [], LOADERS = [])
    env.AppendUnique (CPPPATH = [os.path.join ('#', 'extra', 'lib')],
                      LIBPATH = [os.path.join ('#', 'extra', 'include')])
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
    pass

def EnableAudiereSound (env):
    env.AppendUnique (LIBS = ['audiere']);

def AudiereSoundConfigure(env):
    env.AppendUnique (DRIVERS = ['AUDIERESOUND'])
    audiere_sound = {}
    audiere_sound['ENABLE'] = EnableAudiereSound
    env.AppendUnique (AUDIERESOUND = audiere_sound)
