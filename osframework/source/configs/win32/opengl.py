# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.win32

def AddOptions(opts):
    configs.win32.AddOptions(opts)
    configs.FreeTypeAddOptions (opts)

def EnableWGL(env):
    env.PrependUnique (LIBS = ['user32', 'opengl32', 'gdi32'])

def Configure(env):
    configs.win32.Configure(env)
    env.AppendUnique(DRIVERS = ['WGL'])
    wgl = {}
    wgl['ENABLE'] = EnableWGL
    env.AppendUnique(WGL = wgl)

    configs.FreeTypeConfigure (env)
    ### static link with freetype
    env['FREETYPE_CONFIG'] = None
    env.AppendUnique(BUILD_PACKAGES = ['freetype'])
