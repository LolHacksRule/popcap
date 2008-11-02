# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions(opts):
    configs.AddOptions (opts)

def Configure(env):
    configs.Configure (env)
    env.AppendUnique (CPPDEFINES = ['WIN32'])
    env.AppendUnique (CFLAGS = ['/EHsc'],
                      CXXFLAGS = ['/EHsc'],
                      LINKFLAGS = [])

    if env['debug']:
        env.AppendUnique (CFLAGS = ['/DEBUG'],
                          CXXFLAGS = ['/DEBUG'],
                          LINKFLAGS = ['/DEBUG', '/PROFILE',
                                      '/PDB:${TARGET}.pdb'])
        env.Replace (CCPDBFLAGS = '/Zi /Fd${TARGET}.pdb')

    if env['release']:
        env.AppendUnique (CFLAGS = ['/O2'],
                          CXXFLAGS = ['/O2'],
                          LINKFLAGS = [])
