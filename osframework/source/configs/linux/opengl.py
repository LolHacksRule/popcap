# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.linux.AddOptions (opts)

def EnableGLX(env):
    env.PrependUnique (LIBS = ['GL', 'X11'])

def Configure(env):
    configs.linux.Configure (env)
    env.AppendUnique (DRIVERS = ['GLX'])
    glx = {}
    glx['ENABLE'] = EnableGLX
    env.AppendUnique (GLX = glx)

    ### enable genaric linux input driver.
    configs.linux.LinuxInputConfigure (env)

    ### gstreamer sound manager
    configs.linux.GstSoundConfigure (env)
