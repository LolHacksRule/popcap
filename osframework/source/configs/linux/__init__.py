# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions(opts):
    configs.AddOptions(opts)

def Configure(env):
    configs.Configure(env)
    env.AppendUnique(CFLAGS = ['-pthread'],
                     LINKFLAGS = ['-pthread'])
    env.AppendUnique(CFLAGS = ['-g', '-fno-unit-at-a-time', '-Wall'],
                     CXXFLAGS = ['-g', '-Wall'],
                     LINKFLAGS = ['-g', '-fno-unit-at-a-time'],
                     LIBS = ['rt', 'm'])
