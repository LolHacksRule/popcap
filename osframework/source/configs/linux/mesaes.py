# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs
import SCons
import os

def AddOptions (opts):
    configs.SetOptionsDefault('freetype', True)
    configs.linux.AddOptions (opts)
    configs.AudiereSoundAddOptions (opts)

    from SCons.Variables.PathVariable import PathVariable
    if 'mesaes_path' in opts.keys ():
        return
    opts.Add (PathVariable ('mesaes_path',
                            'Path to mesa opengles installed directory',
                            '',
                            PathVariable.PathAccept))

def EnableXGLES(env):
    includedir = os.path.join (env['mesaes_path'], 'include')
    libdir = os.path.join (env['mesaes_path'], 'lib')
    env.PrependUnique (CPPDEFINES = ['SEXY_OPENGLES'],
                       CPPPATH = [includedir],
                       LIBPATH = [libdir],
                       LIBS = ['GLESv1_CM', 'EGL', 'X11'])

def Configure(env):
    env['mesaes_path'] = os.path.expanduser (env['mesaes_path'])
    configs.linux.Configure (env)
    env.AppendUnique (DRIVERS = ['XGLES'])
    gles = {}
    gles['ENABLE'] = EnableXGLES
    env.AppendUnique (XGLES = gles)

    ### gstreamer sound manager
    #configs.linux.GstSoundConfigure (env)
    configs.AudiereSoundConfigure (env)

    env.AppendUnique(BUILD_PACKAGES = ['freetype'])
    env['FREETYPECONFIG'] = None
