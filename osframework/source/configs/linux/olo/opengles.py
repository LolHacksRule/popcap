# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.linux.olo
import os.path

def AddOptions (opts):
    configs.linux.olo.AddOptions (opts)

def EnableGDLGLES (env):
    env.PrependUnique (CPPDEFINES = ['SEXY_OPENGLES'],
                       CPPPATH = [os.path.join ('#', 'configs', 'linux',
                                                'olo', 'include'),
                                  os.path.join ('#', 'configs', 'linux',
                                                'olo', 'include', 'GLES')],
                       LIBS = ['GLES_CM', 'IMGegl', 'srv_um_usrports'])

def Configure (env):
    configs.linux.olo.Configure (env)
    env.AppendUnique (DRIVERS = ['GDLGLES'])
    gles = {}
    gles['ENABLE'] = EnableGDLGLES
    env.AppendUnique(GDLGLES = gles)

    ### enable genaric linux input driver.
    configs.linux.LinuxInputConfigure (env)
    configs.linux.UdpInputConfigure (env)
