# -*- coding: utf-8 -*-
# Author: KanKer

# Spec file for Intel Olo board.

import os.path
import configs.linux

def AddOptions (opts):
    configs.linux.AddOptions (opts)

def Configure (env):
    configs.linux.Configure (env)
    versions = ["2.3", "2.2"]
    for version in versions:
        prefix = '/opt/STM/STLinux-' + version + '/devkit/sh4'
        if os.path.exists(prefix):
            break
    tcdir = os.path.join (prefix, 'bin')
    targetdir = os.path.join (prefix, 'target')
    env['prefix'] = prefix
    env['PKG_CONFIG_LIBDIR'] = os.path.join (prefix, 'lib', 'pkgconfig')
    env['PKG_CONFIG_LIBDIR'] += ":" + os.path.join (targetdir,
                                                    'usr', 'lib', 'pkgconfig')
    env['CC'] = os.path.join (tcdir, 'sh4-linux-gcc')
    env['CXX'] = os.path.join (tcdir, 'sh4-linux-g++')
    env['LINK'] = os.path.join (tcdir, 'sh4-linux-g++')
    env['AR'] = os.path.join (tcdir, 'sh4-linux-ar')
    env['STRIP'] = os.path.join (tcdir, 'sh4-linux-strip')
    env['RANLIB'] = os.path.join (tcdir, 'sh4-linux-ranlib')
    env['LD'] = os.path.join (tcdir, 'sh4-linux-ld')
    env.AppendUnique (CPPDEFINES = ['SEXY_ST_SH4'])
    env.AppendUnique (CPPPATH = [os.path.join (prefix, 'include'),
                                 os.path.join (targetdir, 'include'),
                                 os.path.join (targetdir, 'usr', 'include')],
                      LIBPATH = [os.path.join (prefix, 'lib'),
                                 os.path.join (targetdir, 'lib'),
                                 os.path.join (targetdir, 'usr', 'lib')],
                      LIBS = [])
    env.AppendUnique (LINKFLAGS = ['-export-dynamic'])

    env.Replace (PKGCONFIG = 'PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR pkg-config')
