# -*- coding: utf-8 -*-
# Author: Luo Jinghua

# Spec file for Intel Canmore board.

import os.path
import configs.linux
import SCons

def AddOptions (opts):
    configs.linux.AddOptions (opts)
    from SCons.Variables.PathVariable import PathVariable
    if 'pdk_path' in opts.keys ():
        return
    opts.Add (PathVariable ('pdk_path',
                            'Path to pdk installed directory',
                            '',
                            PathVariable.PathAccept))

def Configure (env):
    if 'i686-cm-linux-gcc' in env['CC']:
        return
    configs.linux.Configure (env)
    if env.has_key ('pdk_path') and env['pdk_path']:
        pdkroot = os.path.expanduser (env ['pdk_path'])
    else:
        if os.path.exists ('/opt/Intel-Canmore'):
            pdkroot = '/opt/Intel-Canmore'
        else:
            home = os.path.expanduser ("~")
            pdkroot = None
            for version in ('1.1078', '1.1073', '1.1066',
                            '1.1050', '1.586'):
                pdkroot = os.path.join (home, 'Canmore-' + version)
                if os.path.exists (pdkroot):
                    break;
    if not pdkroot:
        print "PDK for Canmore not found, please set the pak_path " \
              "to an appropriate path."
        return
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

    env['PKGCONFIG'] = 'PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR pkg-config'
