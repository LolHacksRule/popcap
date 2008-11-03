# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.linux.canmore

def AddOptions (opts):
    configs.linux.canmore.AddOptions (opts)

def EnableGDLGLES (env):
    env.PrependUnique (CPPDEFINES = ['SEXY_OPENGLES'],
                       LIBS = ['GLES_CM', 'IMGegl', 'srv_um', 'gdl', 'osal'])

def Configure (env):
    configs.linux.canmore.Configure (env)
    env.AppendUnique (DRIVERS = ['GDLGLES'])
    gles = {}
    gles['ENABLE'] = EnableGDLGLES
    env.AppendUnique(GDLGLES = gles)
