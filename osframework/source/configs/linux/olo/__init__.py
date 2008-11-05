# -*- coding: utf-8 -*-
# Author: KanKer

# Spec file for Intel Olo board.

import os.path
import configs.linux

def AddOptions (opts):
    configs.linux.AddOptions (opts)

def Configure (env):
    configs.linux.Configure (env)
    prefix = '/usr/local/arm-linux'
    tcdir = os.path.join(prefix, 'bin')
    olo_src = '/opt/Intel-Olo/arm-linux-elf'
    env['prefix'] = prefix
    env['PKG_CONFIG_LIBDIR'] = os.path.join (prefix, 'lib', 'pkgconfig')
    env['CC'] = os.path.join (tcdir, 'arm-linux-gcc')
    env['CXX'] = os.path.join (tcdir, 'arm-linux-g++')
    env['LINK'] = os.path.join (tcdir, 'arm-linux-g++')
    env['AR'] = os.path.join (tcdir, 'arm-linux-ar')
    env['STRIP'] = os.path.join (tcdir, 'arm-linux-strip')
    env['RANLIB'] = os.path.join (tcdir, 'arm-linux-ranlib')
    env['LD'] = os.path.join (tcdir, 'arm-linux-ld')
    env.AppendUnique (CPPDEFINES = ['SEXY_INTEL_OLO'])
    env.AppendUnique (CPPPATH = [os.path.join (prefix, 'include')],
                      LIBPATH = [os.path.join (prefix, 'lib'),
                                 os.path.join (olo_src, 'lib')],
                      LIBS = [])
    env.AppendUnique (LINKFLAGS = ['-export-dynamic'])

    env.Replace (PKGCONFIG = 'PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR pkg-config')
