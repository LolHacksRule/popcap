# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.linux.android

def AddOptions (opts):
    configs.linux.android.AddOptions (opts)
    #configs.AudiereSoundAddOptions (opts)

def EnableAndroidGLES (env):
    env.PrependUnique (CPPDEFINES = ['SEXY_OPENGLES'],
                       LIBS = ['GLESv1_CM'])

def Configure (env):
    configs.linux.android.Configure (env)
    env.AppendUnique (DRIVERS = ['ANDROIDGLES'])
    gles = {}
    gles['ENABLE'] = EnableAndroidGLES
    env.AppendUnique(ANDROIDGLES = gles)

    ### gstreamer sound manager
    #configs.AudiereSoundConfigure (env)

    env.AppendUnique(BUILD_PACKAGES = ['freetype'])
    env['FREETYPECONFIG'] = None
