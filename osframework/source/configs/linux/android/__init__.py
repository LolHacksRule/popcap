# -*- coding: utf-8 -*-
# Author: Luo Jinghua

# Spec file for Google Android.

import sys
import os.path
import configs.linux
import SCons
import copy

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
    opts.Add (('android_compiler', 'Build binary for specified abi', 'arm-eabi-4.4.0'))
    opts.Add (('android_libabi',
               'Install native libraries into the lib/$android_libabi directory of apk',
               'default'))
    opts.Add (EnumVariable('android_platform', 'Build binary for specified android platform',
                           '5', ('4', '5', '8')))

    opts.Add (EnumVariable('android_fpu', 'Build binary for specified float point unit',
                           'default', ('default', 'softfloat', 'hardfloat', 'vfp', 'vfpv3',
                                       'vfpv3-16', 'neon')))
    opts.Add (EnumVariable('android_cpu', 'Build binary for specified cpu',
                           'generic', ('generic', 'tcc89xx', 'cortex-a9')))

    opts.Add (PathVariable ('apk_sign_keystore',
                            'keystore to sign apks',
                            os.path.join('#android', 'keystore', 'apk.jinghua.com'),
                            PathVariable.PathAccept))
    opts.Add (('apk_sign_key', 'key to sign apks', 'apk.jinghua.com'))
    opts.Add (('apk_sign_keystore_pass', 'keystore password', 'apk.jinghua.com'))
    opts.Add (('apk_sign_key_pass', 'key password', 'apk.jinghua.com'))

def AndroidProgram(env, target, source, **kwargs):
    archdir = env['archdir']
    linkflags = [#"-Bdynamic",
                 "-Wl,-Bsymbolic",
                 "-Wl,-dynamic-linker,/system/bin/linker",
                 "-Wl,--gc-sections", "-Wl,-z,nocopyreloc",
                 "-Wl,--no-undefined", "-nostdlib",
                 '-L' + os.path.join(archdir, 'usr', 'lib'),
                 '-L' + os.path.join(env['gccdir'], env['gccabi'], 'lib'),
                 '-L' + os.path.join(env['gccdir'], 'lib', 'gcc', env['gccabi'],
                                     env['gccver'], 'interwork'),
                 os.path.join(archdir, 'usr', 'lib', "crtend_android.o"),
                 os.path.join(archdir, 'usr', 'lib', "crtbegin_dynamic.o"),
                 "-lc", '-ldl']
    libc = os.path.join(archdir, 'usr', 'lib', 'libc.a')
    source += [libc]
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
    libs += ['gcc', 'c', 'm', 'dl']
    kwargs['LIBS'] = libs
    return env.OldProgram(target, source, **kwargs)

def AndroidSharedLibrary(env, target, source, **kwargs):
    archdir = env['archdir']
    linkflags = ["-Wl,--gc-sections", "-Wl,-z,nocopyreloc",
                 "-Wl,--no-undefined", "-nostdlib",
                 '-L' + os.path.join(archdir, 'usr', 'lib'),
                 "-lc", '-ldl']

    if 'mips' in env['android_compiler']:
        linkflags.insert(0, '-Wl,-T,ldscripts/mipself_android.xsc')

    if 'SHLINKFLAGS' in kwargs:
        linkflags += kwargs['SHLINKFLAGS']
    else:
        linkflags += env['SHLINKFLAGS']
    linkflags = copy.copy(linkflags)
    if env['stdcpplib']:
        linkflags.append(env['stdcpplib'])
    kwargs['SHLINKFLAGS'] = linkflags

    if 'POSTSHLINKFLAGS' in kwargs:
        postlinkflags = kwargs['POSTSHLINKFLAGS']
    else:
        postlinkflags = env['POSTSHLINKFLAGS']

    if 'LIBS' in kwargs:
        libs = kwargs['LIBS']
    else:
        libs = env['LIBS']
    libs = copy.copy(libs)
    libs += ['c', 'm', 'dl', 'gcc']
    kwargs['LIBS'] = libs

    if 'mips' in env['android_compiler'] and 'GameLauncher' in libs:
        while 'GameLauncher' in libs:
            libs.remove('GameLauncher')
        postlinkflags.insert(-3, '-lGameLauncher')
        kwargs['POSTSHLINKFLAGS'] = postlinkflags
    return env.OldSharedLibrary(target, source, **kwargs)

def Configure (env):
    import re
    configs.linux.Configure (env)
    env['PKGCONFIG'] = '/path/to/pkgconfig'
    env['FREETYPE_CONFIG'] = ''

    env['TARGET_PLATFORM'] = 'android'

    env['LINKCOM'] += ' $POSTLINKFLAGS'
    env['POSTLINKFLAGS'] = SCons.Util.CLVar('')
    env['SHLINKCOM'] += ' $POSTSHLINKFLAGS'
    env['POSTSHLINKFLAGS'] = SCons.Util.CLVar('')

    if sys.platform == 'darwin':
        tcarch = 'darwin-x86'
    else:
        tcarch = 'linux-x86'
    sdkroot = os.path.expanduser (env ['android_sdk_path'])
    ndkroot = os.path.expanduser (env ['android_ndk_path'])
    env ['android_sdk_root'] = sdkroot
    env ['android_ndk_root'] = ndkroot
    api = re.sub('-[0-9.]*$', '', env['android_compiler'])
    arch = api[:api.find('-')]
    postshlinkflags = ['-lstdc++', '-llog']
    postlinkflags =  ['-nostdlib', "-Wl,--gc-sections", "-Wl,-z,nocopyreloc",
                      "-Wl,--no-undefined"]
    if arch == 'mips':
        postshlinkflags += ['-lgcc_eh']
        postlinkflags += ['-lgcc_eh']
    postshlinkflags += ['-lgcc', '-lc', '-lm', '-ldl']
    ### ndk-r4
    stdcpplib = ''
    toolchain = os.path.join(ndkroot, 'build', 'prebuilt')
    abimap = {
        'arm' : 'armeabi',
        'mips' : 'mips'
    }
    if  env['android_libabi'] in ('', 'default'):
        env['android_libabi'] = abimap[arch]

    ### ndk r4
    if os.path.exists(toolchain):
        toolchain = os.path.join(toolchain, tcarch)
        gccdir = os.path.join (toolchain, env['android_compiler'])
        tcdir = os.path.join (toolchain, env['android_compiler'], 'bin')
        archdir = os.path.join(ndkroot, 'build', 'platforms',
                               'android-' + env['android_platform'],
                               'arch-' + arch)
        sysroot = os.path.join (toolchain, env['android_compiler'])
    else:
        ### ndk-r5
        toolchain = os.path.join(ndkroot, 'toolchains')
        gccdir = os.path.join (toolchain, env['android_compiler'],
                               'prebuilt', tcarch)
        tcdir = os.path.join (gccdir, 'bin')
        archdir = os.path.join(ndkroot, 'platforms',
                               'android-' + env['android_platform'],
                               'arch-' + arch)
        env.AppendUnique(CPPPATH = [ os.path.join(ndkroot, 'sources', 'cxx-stl',
                                                  'gnu-libstdc++', 'include')])
        env.AppendUnique(CPPPATH = [ os.path.join(ndkroot, 'sources', 'cxx-stl',
                                                  'gnu-libstdc++', 'libs', abimap[arch],
                                                  'include')])

        stdcpplib = os.path.join(ndkroot, 'sources', 'cxx-stl', 'gnu-libstdc++',
                                 'libs', abimap[arch], 'libstdc++.a')
        postshlinkflags.insert(0, stdcpplib)
        sysroot = archdir
    env['gccdir'] = gccdir
    env['gccver'] = env['android_compiler'].replace(api + '-', '')
    env['gccabi'] = 'arm' in api and 'armeabi' or api
    env['toolchain'] = toolchain
    env['archdir'] = archdir
    env['stdcpplib'] = stdcpplib
    env['android_arch'] = arch
    #env['android_libabi'] = abimap[arch]
    env['TARGET_SUBARCH'] = abimap[arch]
    env['PKG_CONFIG_LIBDIR'] = os.path.join (archdir, 'usr', 'lib', 'pkgconfig')
    env['CC'] = os.path.join (tcdir, api + '-gcc')
    env['CXX'] = os.path.join (tcdir, api + '-g++')
    env['LINK'] = os.path.join (tcdir, api + '-g++')
    env['AR'] = os.path.join (tcdir, api + '-ar')
    env['STRIP'] = os.path.join (tcdir, api + '-strip')
    env['RANLIB'] = os.path.join (tcdir, api + '-ranlib')
    env['LD'] = os.path.join (tcdir, api + '-ld')
    env['SHLIBSUFFIX'] = '.so'
    env.AppendUnique (CPPDEFINES = ['ANDROID', '__ANDROID__', 'SEXY_ANDROID'])
    env.AppendUnique (CPPPATH = [os.path.join(archdir, 'usr', 'include')],
                      LINKFLAGS = [('--sysroot', sysroot)],
                      POSTLINKFLAGS = postlinkflags,
                      POSTSHLINKFLAGS = postshlinkflags,
                      LIBPATH = [os.path.join(archdir, 'usr', 'lib')],
                      LIBS = ['stdc++'])
    env.AppendUnique(CCFLAGS = ['-g'], LINKFLAGS = ['-g'])

    fpumap = {
        'cortex-a9' : 'vfpv3-d16',
        'tcc89xx' : 'vfp'
    }
    fpumodemap = {
        'vfpv3-d16' : ['-mfpu=vfpv3-d16', '-mfloat-abi=softfp'],
        'vfpv3' : ['-mfpu=vfpv3', '-mfloat-abi=softfp'],
        'neon' : ['-mfpu=neon', '-mfloat-abi=softfp'],
        'vfp' : ['-mfpu=vfp', '-mfloat-abi=softfp'],
    }
    cpumap = {
        'cortex-a9' : 'cortex-a9',
    }
    archmap = {
        'tcc89xx' : 'armv6',
    }
    cpu = env['android_cpu']
    cpuflags = []
    if cpu in cpumap:
        cpuflags += ['-mcpu=%s' % cpumap[cpu],
                     '-mtune=%s' % cpumap[cpu]]
    elif cpu in archmap:
        cpuflags += ['-march=%s' % archmap[cpu]]
    elif arch == 'arm':
        cpuflags += ["-march=armv5te", "-mtune=xscale", '-mthumb']
        env.AppendUnique(CPPDEFINES = ['__ARM_ARCH_5__', '__ARM_ARCH_5T__',
                                       '__ARM_ARCH_5E__', '__ARM_ARCH_5TE__'])
    env['android_cpu_flags'] = cpuflags

    fpu = env['android_fpu']
    if fpu == 'default' and cpu in fpumap:
        fpu = fpumap[cpu]
    fpuflags = []
    if fpu in fpumodemap:
        fpuflags += fpumodemap[fpu]
    if not fpuflags:
        default_fpuflags = {
            'arm' : '-msoft-float',
            'mips' : '-mhard-float'
        }
        android_fpu = env['android_fpu']
        if android_fpu == 'default' and android_fpu in default_fpuflags:
            fpuflags = default_fpuflags[android_fpu]
        elif android_fpu == 'hardfloat':
            fpuflags += ['-mhard-float']
        else:
            fpuflags += ['-msoft-float']
    env['android_fpu_flags'] = fpuflags

    env.AppendUnique(CCFLAGS = cpuflags + fpuflags,
                     LINKFLAGS = cpuflags + fpuflags)

    if arch == 'mips':
        env.AppendUnique(CXXFLAGS = ['-fexceptions', '-frtti', '-fno-use-cxa-atexit'],
                         LINKFLAGS = ['-fno-use-cxa-atexit'])
        env.AppendUnique(CCFLAGS = ['-EL', '-march=mips32'],
                         LINKFLAGS = ['-EL', '-march=mips32'])
    env.AppendUnique(CPPDEFINES = ['_GLIBCXX_USE_WCHAR_T'])
    env.AppendUnique(CCFLAGS = ['-fno-short-enums'])

    ### replace the Program()
    env['BUILDERS']['OldSharedLibrary'] = env['BUILDERS']['SharedLibrary']
    env['BUILDERS']['SharedLibrary'] = AndroidSharedLibrary
    env['BUILDERS']['OldProgram'] = env['BUILDERS']['Program']
    env['BUILDERS']['Program'] = AndroidProgram

    ### andriod doesn't have librt.so
    bad_flags = {
        'LIBS' : ['rt'],
        'CFLAGS' : ['-pthread'],
        'CXXFLAGS' : ['-pthread'],
        'CCFLAGS' : ['-pthread'],
        'LINKFLAGS' : ['-pthread', '-export-dynamic']
    }
    for var in bad_flags.keys():
        for flag in bad_flags[var]:
            if flag in env[var]:
                env[var].remove(flag)
