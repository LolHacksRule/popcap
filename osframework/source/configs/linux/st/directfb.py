# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import os.path
import configs.linux.st

def AddOptions (opts):
    configs.linux.st.AddOptions (opts)

def EnableDirecFB (env):
    env.AppendUnique (LIBS = ['directfb', 'z', 'fusion', 'direct', 'pthread', 'dl'],
                      CPPPATH = [os.path.join (env['prefix'], 'target', 'usr',
                                               'include', 'directfb')],
                      LIBPATH = [os.path.join (env['prefix'], 'target',
                                               'usr', 'lib')])

def Configure (env):
    configs.linux.st.Configure (env)
    env.AppendUnique (DRIVERS = ['DIRECTFB'])
    directfb = {}
    directfb['ENABLE'] = EnableDirecFB
    env.AppendUnique (DIRECTFB = directfb)

    configs.linux.LinuxInputConfigure (env)
    configs.linux.UdpInputConfigure (env)

