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

    opts.Add (PathVariable ('apk_sign_keystore',
                            'keystore to sign apks',
                            os.path.join('#android', 'keystore', 'apk.jinghua.org'),
                            PathVariable.PathAccept))
    opts.Add (('apk_sign_key', 'key to sign apks', 'apk.jinghua.org'))
    opts.Add (('apk_sign_keystore_pass', 'keystore password', 'apk.jinghua.org'))
    opts.Add (('apk_sign_key_pass', 'key password', 'apk.jinghua.org'))

def AndroidProgram(env, target, source, **kwargs):
    archdir = env['archdir']
    linkflags = [#"-Bdynamic",
                 "-Wl,-Bsymbolic",
                 "-Wl,-dynamic-linker,/system/bin/linker",
                 "-Wl,--gc-sections", "-Wl,-z,nocopyreloc",
                 "-Wl,--no-undefined", "-nostdlib",
                 '-L' + os.path.join(archdir, 'usr', 'lib'),
                 '-L' + os.path.join(env['gccdir'], env['gccabi'], 'lib', 'interwork'),
                 '-L' + os.path.join(env['gccdir'], env['gccabi'], 'lib'),
                 os.path.join(archdir, 'usr', 'lib', "crtend_android.o"),
                 os.path.join(archdir, 'usr', 'lib', "crtbegin_dynamic.o"),
                 "-lc", '-ldl']
    source += [os.path.join(env['gccdir'], 'lib', 'gcc', env['gccabi'],
                            env['gccver'], 'interwork', 'libgcc.a'),
               os.path.join(archdir, 'usr', 'lib', 'libc.a')]
    if 'LINKFLAGS' in kwargs:
        linkflags += kwargs['LINKFLAGS']
    else:
        linkflags += env['LINKFLAGS']
    kwargs['LINKFLAGS'] = linkflags
    return env.OldProgram(target, source, **kwargs)

def AndroidSharedLibrary(env, target, source, **kwargs):
    archdir = env['archdir']
    linkflags = ["-Wl,--gc-sections", "-Wl,-z,nocopyreloc",
                 "-Wl,--no-undefined", "-Wl,-shared,-Bsymbolic",
                 "-nostdlib",
                 '-L' + os.path.join(archdir, 'usr', 'lib'),
                 '-L' + os.path.join(env['gccdir'], env['gccabi'], 'lib', 'interwork'),
                 '-L' + os.path.join(env['gccdir'], env['gccabi'], 'lib')]
    if 'LINKFLAGS' in kwargs:
        linkflags += kwargs['LINKFLAGS']
    else:
        linkflags += env['LINKFLAGS']
    kwargs['LINKFLAGS'] = linkflags

    libs = []
    if 'LIBS' in kwargs:
        libs += kwargs['LIBS']
    else:
        libs += env['LIBS']
    libs += ['gcc']
    kwargs['LIBS'] = libs
    return env.OldSharedLibrary(target, source, **kwargs)

def Configure (env):
    import re
    configs.linux.Configure (env)
    env['TARGET_PLATFORM'] = 'android'

    env['LINKCOM'] += ' $POSTLINKFLAGS'
    env['POSTLINKFLAGS'] = SCons.Util.CLVar('')
    env['SHLINKCOM'] += ' $POSTSHLINKFLAGS'
    env['POSTSHLINKFLAGS'] = SCons.Util.CLVar('')

    sdkroot = os.path.expanduser (env ['android_sdk_path'])
    ndkroot = os.path.expanduser (env ['android_ndk_path'])
    toolchain = os.path.join(ndkroot, 'build', 'prebuilt')
    if sys.platform == 'darwin':
        toolchain = os.path.join(toolchain, 'darwin-x86')
    else:
        toolchain = os.path.join(toolchain, 'linux-x86')
    gccdir = os.path.join (toolchain, env['android_abi'])
    tcdir = os.path.join (toolchain, env['android_abi'], 'bin')
    api = re.sub('-[0-9.]*$', '', env['android_abi'])
    arch = api[:api.find('-')]
    archdir = os.path.join(ndkroot, 'build', 'platforms',
                           'android-' + env['android_platform'],
                           'arch-' + arch)
    env['gccdir'] = gccdir
    env['gccver'] = env['android_abi'].replace(api + '-', '')
    env['gccabi'] = api
    env['toolchain'] = toolchain
    env['archdir'] = archdir
    env['PKG_CONFIG_LIBDIR'] = os.path.join (archdir, 'usr', 'lib', 'pkgconfig')
    env['CC'] = os.path.join (tcdir, api + '-gcc')
    env['CXX'] = os.path.join (tcdir, api + '-g++')
    env['LINK'] = os.path.join (tcdir, api + '-g++')
    env['AR'] = os.path.join (tcdir, api + '-ar')
    env['STRIP'] = os.path.join (tcdir, api + '-strip')
    env['RANLIB'] = os.path.join (tcdir, api + '-ranlib')
    env['LD'] = os.path.join (tcdir, api + '-ld')
    env['SHLIBSUFFIX'] = '.so'
    env.AppendUnique (CPPDEFINES = ['ANDROID', 'SEXY_ANDROID'])
    env.AppendUnique (CPPPATH = [os.path.join(archdir, 'usr', 'include')],
                      CCFLAGS = ["-mthumb-interwork"],
                      LINKFLAGS = [('--sysroot', os.path.join (toolchain, env['android_abi']))],
                      SHLINKFLAGS = [],
                      POSTSHLINKFLAGS = ['-lc', '-ldl'],
                      LIBPATH = [os.path.join(archdir, 'usr', 'lib')],
                      LIBS = ['stdc++'])

    ### replace the Program()
    env['BUILDERS']['OldSharedLibrary'] = env['BUILDERS']['SharedLibrary']
    env['BUILDERS']['SharedLibrary'] = AndroidSharedLibrary
    env['BUILDERS']['OldProgram'] = env['BUILDERS']['Program']
    env['BUILDERS']['Program'] = AndroidProgram

    ### andriod doesn't have librt.so
    bad_flags = {
        'LIBS' : ['rt'],
        'CCFLAGS' : ['-pthread'],
        'LINKFLAGS' : ['-pthread', '-export-dynamic']
    }
    for var in bad_flags.keys():
        for flag in bad_flags[var]:
            if flag in env[var]:
                env[var].remove(flag)
    if True:
        env.AppendUnique(CCFLAGS = ["-march=armv5te", "-mtune=xscale", "-msoft-float"])
