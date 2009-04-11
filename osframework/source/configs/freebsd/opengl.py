# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.freebsd.AddOptions (opts)
    configs.AudiereSoundAddOptions (opts)

def EnableGLX(env):
    env.PrependUnique (LIBS = ['GL', 'X11'])

def Configure(env):
    configs.freebsd.Configure (env)
    env.AppendUnique (DRIVERS = ['GLX'])
    glx = {}
    glx['ENABLE'] = EnableGLX
    env.AppendUnique (GLX = glx)

    ### enable genaric freebsd input driver.
    configs.freebsd.UdpInputConfigure (env)

    ### audiere sound manager
    configs.AudiereSoundConfigure (env)
