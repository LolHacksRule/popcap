# -*- coding: utf-8 -*-
# Author: KanKer

# Spec file for Intel Canmore board.

import os.path
import configs.linux

def AddOptions (opts):
    configs.linux.AddOptions (opts)

def Configure (env):
    configs.linux.Configure (env)
    home =  os.path.expanduser("~")
    version = '1.1050'
    pdkroot = os.path.join (home, 'Canmore-' + version)
    #pdkroot = '/opt/Intel-Canmore'
    prefix = os.path.join (pdkroot, 'i686-linux-elf')
    tcdir = os.path.join (prefix, 'bin')
    env['prefix'] = prefix
    env['CROSS'] = os.path.join (tcdir, 'i686-cm-linux')
    env['PKG_CONFIG_LIBDIR'] = os.path.join (prefix, 'lib', 'pkgconfig')
    env['CC'] = os.path.join (tcdir, 'i686-cm-linux-gcc')
    env['CXX'] = os.path.join (tcdir, 'i686-cm-linux-g++')
    env['LINK'] = os.path.join (tcdir, 'i686-cm-linux-g++')
    env['AR'] = os.path.join (tcdir, 'i686-cm-linux-ar')
    env['STRIP'] = os.path.join (tcdir, 'i686-cm-linux-strip')
    env['RANLIB'] = os.path.join (tcdir, 'i686-cm-linux-ranlib')
    env['LD'] = os.path.join (tcdir, 'i686-cm-linux-ld')
    env.AppendUnique (CPPDEFINES = ['SEXY_INTEL_CANMORE'])
    env.AppendUnique (CPPPATH = [os.path.join(env['prefix'], 'include')],
                      LIBPATH = [os.path.join(env['prefix'], 'lib')],
                      LIBS = [])
    env.AppendUnique (LINKFLAGS = ['-export-dynamic'])

    env.Replace (PKGCONFIG = 'PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR pkg-config')
