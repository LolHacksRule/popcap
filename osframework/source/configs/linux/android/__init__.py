# -*- coding: utf-8 -*-
# Author: Luo Jinghua

# Spec file for Google Android.

import sys
import os.path
import configs.linux
import SCons

def AddOptions (opts):
    configs.linux.AddOptions (opts)
    from SCons.Variables.PathVariable import PathVariable
    from SCons.Variables.EnumVariable import EnumVariable
    if 'android_sdk_path' in opts.keys ():
        return
    opts.Add (PathVariable ('android_sdk_path',
                            'Path to android sdk installed directory',
                            '',
                            PathVariable.PathAccept))

    opts.Add (PathVariable ('android_ndk_path',
                            'Path to android native sdk installed directory',
                            '',
                            PathVariable.PathAccept))
    opts.Add (('android_abi', 'Build binary with specified abi', 'arm-eabi-4.2.1'))
    opts.Add (EnumVariable('android_platform', 'Build binary for specified android platform',
                           '5', ('4', '5', '8')))

def Configure (env):
    import re
    configs.linux.Configure (env)
    sdkroot = os.path.expanduser (env ['android_sdk_path'])
    ndkroot = os.path.expanduser (env ['android_ndk_path'])
    toolchain = os.path.join(ndkroot, 'build', 'prebuilt')
    if sys.platform == 'darwin':
        toolchain = os.path.join(toolchain, 'darwin-x86')
    else:
        toolchain = os.path.join(toolchain, 'linux-darwin')
    tcdir = os.path.join (toolchain, env['android_abi'], 'bin')
    api = re.sub('-[0-9.]*$', '', env['android_abi'])
    arch = api[:api.find('-')]
    archdir = os.path.join(ndkroot, 'build', 'platforms',
                           'android-' + env['android_platform'],
                           'arch-' + arch)
    env['PKG_CONFIG_LIBDIR'] = os.path.join (archdir, 'usr', 'lib', 'pkgconfig')
    env['CC'] = os.path.join (tcdir, api + '-gcc')
    env['CXX'] = os.path.join (tcdir, api + '-g++')
    env['LINK'] = os.path.join (tcdir, api + '-g++')
    env['AR'] = os.path.join (tcdir, api + '-ar')
    env['STRIP'] = os.path.join (tcdir, api + '-strip')
    env['RANLIB'] = os.path.join (tcdir, api + '-ranlib')
    env['LD'] = os.path.join (tcdir, api + '-ld')
    env['SHLIBSUFFIX'] = '.so'
    env.AppendUnique (CPPDEFINES = ['SEXY_ANDROID'])
    env.AppendUnique (CPPPATH = [os.path.join(archdir, 'usr', 'include')],
                      LIBPATH = [os.path.join(archdir, 'usr', 'lib')],
                      LIBS = [])

    env['PKGCONFIG'] = 'PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR pkg-config'
