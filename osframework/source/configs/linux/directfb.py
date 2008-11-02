# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.linux

def AddOptions(opts):
    configs.linux.AddOptions(opts)

def EnableDirecFB(env):
    env.ParseConfig('pkg-config directfb --cflags --libs')

def Configure(env):
    configs.linux.Configure(env)
    env.AppendUnique(DRIVERS = ['DIRECTFB'])
    directfb = {}
    directfb['ENABLE'] = EnableDirecFB
    env.AppendUnique (DIRECTFB = directfb)
