# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs
import SCons
import os

def AddOptions (opts):
    configs.linux.AddOptions (opts)
    configs.AudiereSoundAddOptions (opts)

    from SCons.Variables.PathVariable import PathVariable
    if 'pdk_path' in opts.keys ():
        return
    opts.Add (PathVariable ('powervr_sdk',
                            'Path to pdk installed directory',
                            '',
                            PathVariable.PathAccept))

def EnableXGLES(env):
    includedir = os.path.join (env['powervr_sdk'], 'Builds',
                               'OGLES', 'Include')
    libdir = os.path.join (env['powervr_sdk'], 'Builds',
                           'OGLES', 'LinuxPC', 'Lib')
    env.PrependUnique (CPPDEFINES = ['SEXY_OPENGLES'],
                       CPPPATH = [includedir],
                       LIBPATH = [libdir],
                       LIBS = ['GLES_CM', 'EGL', 'X11'])

def Configure(env):
    env['powervr_sdk'] = os.path.expanduser (env['powervr_sdk'])
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
