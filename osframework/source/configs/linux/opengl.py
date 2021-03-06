# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.SetOptionsDefault('freetype', True)
    configs.linux.AddOptions (opts)
    configs.AudiereSoundAddOptions (opts)

def EnableGLX(env):
    env.PrependUnique (LIBS = ['GL', 'X11'])

def Configure(env):
    configs.linux.Configure (env)
    env.AppendUnique (DRIVERS = ['GLX'])
    glx = {}
    glx['ENABLE'] = EnableGLX
    env.AppendUnique (GLX = glx)

    ### gstreamer sound manager
    configs.linux.GstSoundConfigure (env)
    configs.AudiereSoundConfigure (env)
