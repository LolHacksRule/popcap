
# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions(opts):
    configs.SetOptionsDefault('freetype', True)

    configs.AddOptions (opts)
    configs.Win32ModuleLoaderAddOptions (opts)

def Configure(env):
    configs.Configure (env)
    env['TARGET_OS'] = 'win32'
    env.AppendUnique (CPPDEFINES = ['WIN32'])
    env.AppendUnique (CFLAGS = ['/EHsc'],
                      CXXFLAGS = ['/EHsc'],
                      LINKFLAGS = [])

    if env['debug']:
        env.AppendUnique (CFLAGS = ['/DEBUG'],
                          CXXFLAGS = ['/DEBUG'],
                          LINKFLAGS = ['/DEBUG', '/PROFILE'])
        env['PDB'] = '${TARGET.base}.pdb'

    if not env['static']:
        env.AppendUnique (CFLAGS = ['/MD'],
                          CXXFLAGS = ['/MD'])

    if env['release']:
        env.AppendUnique (CFLAGS = ['/O2'],
                          CXXFLAGS = ['/O2'],
                          LINKFLAGS = [])
    configs.Win32ModuleLoaderConfigure (env)

    ### static link with freetype
    env['FREETYPECONFIG'] = None
    env.AppendUnique(BUILD_PACKAGES = ['freetype'])
