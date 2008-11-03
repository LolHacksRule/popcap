# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import os.path
import configs.linux.canmore

def AddOptions (opts):
    configs.linux.canmore.AddOptions (opts)

def EnableDirecFB (env):
    env.AppendUnique (LIBS = ['directfb', 'z', 'fusion', 'direct', 'pthread', 'dl'],
                      CPPPATH = [os.path.join (env['prefix'], 'usr', 'local',
                                               'include', 'directfb')],
                      LIBPATH = [os.path.join (env['prefix'], 'usr', 'local', 'lib')])

def Configure (env):
    configs.linux.canmore.Configure (env)
    env.AppendUnique (DRIVERS = ['DIRECTFB'])
    directfb = {}
    directfb['ENABLE'] = EnableDirecFB
    env.AppendUnique (DIRECTFB = directfb)
