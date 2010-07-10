# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.darwin.AddOptions (opts)
    configs.AudiereSoundAddOptions (opts)

def EnableAGL(env):
    env.PrependUnique (LINKFLAGS = [('-framework', 'OpenGL'),
                                    ('-framework', 'Cocoa')])

def Configure(env):
    configs.darwin.Configure (env)
    env.AppendUnique (DRIVERS = ['AGL'])
    agl = {}
    agl['ENABLE'] = EnableAGL
    env.AppendUnique (AGL = agl)
    configs.AudiereSoundConfigure (env)
