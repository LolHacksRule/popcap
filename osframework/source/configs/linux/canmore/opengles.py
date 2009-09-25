# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.linux.canmore

def AddOptions (opts):
    configs.linux.canmore.AddOptions (opts)
    configs.AudiereSoundAddOptions (opts)

def EnableCEGLES (env):
    env.PrependUnique (CPPDEFINES = ['SEXY_OPENGLES'],
                       LIBS = ['GLES_CM', 'EGL', 'IMGegl', 'srv_um', 'gdl', 'osal'])

def Configure (env):
    configs.linux.canmore.Configure (env)
    env.AppendUnique (DRIVERS = ['CEGLES'])
    gles = {}
    gles['ENABLE'] = EnableCEGLES
    env.AppendUnique(CEGLES = gles)

    ### gstreamer sound manager
    configs.linux.GstSoundConfigure (env)
    configs.AudiereSoundConfigure (env)
