# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

def AddOptions(opts):
    from SCons.Variables.BoolVariable import BoolVariable
    if 'debug' in opts.keys ():
        return
    opts.Add(BoolVariable ('debug', 'build debug version', 'no'))
    opts.Add(BoolVariable ('release', 'build release version', 'no'))

def Configure(env):
    env.AppendUnique (DRIVERS = [], LOADERS = [])

def PosixModuleLoaderAddOptions (opts):
    pass

def EnablePosixModuleLoader (env):
    env.AppendUnique (LIBS = ['dl'])

def PosixModuleLoaderConfigure (env):
    env.AppendUnique (LOADERS = ['POSIXMODULELOADER'])
    dl_loader = {}
    dl_loader['ENABLE'] = EnablePosixModuleLoader
    env.AppendUnique (POSIXMODULELOADER = dl_loader)
