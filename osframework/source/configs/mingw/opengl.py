# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.mingw

def AddOptions(opts):
    configs.mingw.AddOptions(opts)
    configs.AudiereSoundAddOptions(opts)
    configs.FreeTypeAddOptions (opts)

def EnableWGL(env):
    env.PrependUnique (LIBS = ['user32', 'opengl32', 'gdi32'])

def EnableFreeType (env):
    configs.EnableFreeType()

def Configure(env):
    configs.mingw.Configure(env)
    env.AppendUnique(DRIVERS = ['WGL'])
    wgl = {}
    wgl['ENABLE'] = EnableWGL
    env.AppendUnique(WGL = wgl)

    ### freetype
    env.AppendUnique(BUILD_PACKAGES = ['freetype'])
    configs.FreeTypeConfigure (env)
    env['FREETYPECONFIG'] = None
