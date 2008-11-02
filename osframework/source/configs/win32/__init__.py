# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs
import SCons

def AddOptions(opts):
    configs.AddOptions(opts)

def Configure(env):
    configs.Configure(env)
    env.AppendUnique(CPPDEFINES = ['WIN32'])
    env.AppendUnique(CFLAGS = ['/EHsc', '/DEBUG'],
                     CXXFLAGS = ['/EHsc', '/DEBUG'],
                     LINKFLAGS = [])
