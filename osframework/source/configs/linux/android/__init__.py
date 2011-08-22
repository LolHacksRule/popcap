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
    from SCons.Variables.BoolVariable import BoolVariable
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
    opts.Add (('android_compiler', 'Build binary for specified compiler', 'default'))
    opts.Add (('android_libabi',
               'Install native libraries into the lib/$android_libabi directory of apk',
               'default'))
    opts.Add (EnumVariable('android_platform', 'Build binary for specified android platform',
                           'default', ('default', '4', '5', '7', '8', '9')))

    opts.Add (EnumVariable('android_fpu', 'Build binary for specified float point unit',
                           'default', ('default', 'softfloat', 'hardfloat', 'vfp', 'vfpv3',
                                       'vfpv3-16', 'neon')))
    opts.Add (EnumVariable('android_cpu', 'Build binary for specified cpu',
                           'generic', ('generic', 'tcc89xx', 'armv7-a', 'cortex-a8', 'cortex-a9',
                                       'mips32r2')))

    opts.Add (PathVariable ('apk_sign_keystore',
                            'keystore to sign apks',
                            os.path.join('#android', 'keystore', 'apk.transmension.com'),
                            PathVariable.PathAccept))
    opts.Add (('apk_sign_key', 'key to sign apks', 'apk.transmension.com'))
    opts.Add (('apk_sign_keystore_pass', 'keystore password', 'apk.transmension.com'))
    opts.Add (('apk_sign_key_pass', 'key password', 'apk.transmension.com'))
    opts.Add (BoolVariable('android_filesystem',
                           'Enable/disable the file system driver ' + \
                           'which use android api to open and load ' + \
                           'files in apk but it might be not efficient as ' + \
                           'the zzip driver.',
                           False))
    opts.Add (BoolVariable('embed_game_launcher', 'Embed the game launcher into apk',
                           True))

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
                 os.path.join(archdir, 'usr', 'lib', "crtbegin_dynamic.o")]
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
                 '-L' + os.path.join(archdir, 'usr', 'lib')]

    if env['stdcpplib']:
        from SCons.Util import is_List
        lib = env.File(env['stdcpplib'])
        lib.attributes.shared = True
        if not is_List(source):
            source = [source]
        source.append(lib)

    if 'SHLINKFLAGS' in kwargs:
        linkflags += kwargs['SHLINKFLAGS']
    else:
        linkflags += env['SHLINKFLAGS']
    linkflags = copy.copy(linkflags)
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

    dsoh = env.get('dso_handle', None)
    if dsoh:
        from SCons.Util import is_List
        if not is_List(source):
            source = [source]
        source += [dsoh]
    return env.OldSharedLibrary(target, source, **kwargs)

def GetCompilerVersion(c):
    import re
    api = re.sub('-[0-9.]*$', '', c)
    version = c.replace(api + '-', '')
    vers = version.split('.')
    vers = map(lambda v: int(v), vers)
    if len(vers) < 3:
        vers += [0]
    if len(vers) < 4:
        vers += [0]
    v = vers[0] * 1000000 + vers[1] * 10000 + vers[2] * 100 + vers[3]
    return v

def CompilerCompare(c1, c2):
    v1 = GetCompilerVersion(c1)
    v2 = GetCompilerVersion(c2)
    if v1 < v2:
        return -1
    elif v1 > v2:
        return 1
    return 0

def DetectCompilers(ndkroot, host):
    toolchain = os.path.join(ndkroot, 'build', 'prebuilt')
    result = []
    if os.path.exists(toolchain):
        toolchain = os.path.join(toolchain, host)
    else:
        toolchain = os.path.join(ndkroot, 'toolchains')
    if not os.path.exists(toolchain):
        return []
    result = os.listdir(toolchain)
    result = filter(lambda f: os.path.isdir(os.path.join(toolchain, f)), result)
    result.sort(CompilerCompare)
    return result

def SetupCPUFlags(env):
    arch = env['android_cpu_arch']
    subarch = arch
    fpumap = {
        'cortex-a9' : 'vfpv3-d16',
        'cortex-a8' : 'vfpv3-d16',
        'armv7-a' : 'vfp',
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
        'cortex-a8' : 'cortex-a8',
    }
    archmap = {
        'tcc89xx' : 'armv6',
        'armv7-a' : 'armv7-a',

    }
    subarchmap = {
        'cortex-a9' : 'armv7-a',
        'cortex-a8' : 'armv7-a',
        'armv7-a' : 'armv7-a',
        'mips32r2' : 'mips-r2'
    }
    cpu = env['android_cpu']
    if cpu in subarchmap:
        subarch = subarchmap[cpu]
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

    if arch == 'mips':
        if cpu == 'mips32r2':
            cpuflags += ['-EL', '-march=mips32r2']
        else:
            cpuflags += ['-EL', '-march=mips32']

    env['android_cpu_subarch'] = subarch
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
        if android_fpu == 'default' and arch in default_fpuflags:
            fpuflags += [default_fpuflags[arch]]
        elif android_fpu == 'hardfloat':
            fpuflags += ['-mhard-float']
        elif arch != 'x86':
            fpuflags += ['-msoft-float']
    env['android_fpu_flags'] = fpuflags
    env.AppendUnique(CCFLAGS = cpuflags + fpuflags,
                     LINKFLAGS = cpuflags + fpuflags)

    if 'arm' in arch:
        env.AppendUnique(CCFLAGS = ['-Wno-psabi'])

def Configure (env):
    import re

    ### abi map
    abimap = {
        'arm' : 'armeabi',
        'mips' : 'mips',
        'x86' : 'x86'
    }
    subabimap = {
        'armv7-a' : 'armeabi-v7a',
        'mips-r2' : 'mips-r2',
    }

    ### default platform map
    platformmap = {
        'arm' : '8',
        'mips' : '8',
        'x86' : '9'
    }

    configs.linux.Configure (env)
    env['PKGCONFIG'] = '/path/to/pkgconfig'
    env['FREETYPE_CONFIG'] = ''

    env['TARGET_PLATFORM'] = 'android'

    env['LINKCOM'] += ' $POSTLINKFLAGS'
    env['POSTLINKFLAGS'] = SCons.Util.CLVar('')
    env['SHLINKCOM'] += ' $POSTSHLINKFLAGS'
    env['POSTSHLINKFLAGS'] = SCons.Util.CLVar('')

    ### host platform
    if sys.platform == 'darwin':
        tcarch = 'darwin-x86'
    else:
        tcarch = 'linux-x86'
    
    ### replace -dynamiclib with -shared
    env['SHLINKFLAGS'] = SCons.Util.CLVar('$LINKFLAGS -shared')
    if tcarch == 'darwin-x86':
        env.AppendUnique(LINKFLAGS = ['-Wl,-z,muldefs'])

    ### expand ~ in path
    sdkroot = os.path.expanduser (env ['android_sdk_path'])
    ndkroot = os.path.expanduser (env ['android_ndk_path'])
    env ['android_sdk_root'] = sdkroot
    env ['android_ndk_root'] = ndkroot

    ### detect available compilers
    compilers = DetectCompilers(ndkroot, tcarch)
    print 'android: Available compilers:', compilers

    compiler = env['android_compiler']
    if not env['android_compiler'] or env['android_compiler'] == 'default':
        env['android_compiler'] = compilers[-1]
        compiler = env['android_compiler']

    if compiler and compilers and compiler not in compilers:
        compilers = filter(lambda c: compiler in c, compilers)
        if compilers:
            env['android_compiler'] = compilers[-1]
            compiler = env['android_compiler']

    print 'android: Using compiler:', compiler

    ### extract cpu arch from compiler name
    api = re.sub('-[0-9.]*$', '', env['android_compiler'])
    if api.find('-') < 0:
        arch = api
    else:
        arch = api[:api.find('-')]
    env['android_cpu_arch'] = arch

    ### setup default platform
    if env['android_platform'] == 'default':
        if arch in platformmap:
            platform = platformmap[arch]
        else:
            platform = '9'
        env['android_platform'] = platform

    postshlinkflags = ['-lstdc++']
    postlinkflags =  ['-nostdlib', "-Wl,--gc-sections", "-Wl,-z,nocopyreloc",
                      "-Wl,--no-undefined"]
    #if arch == 'mips':
    #    postshlinkflags += ['-lgcc_eh']
    #    postlinkflags += ['-lgcc_eh']
    postshlinkflags += ['-lgcc', '-lc', '-lm', '-ldl']

    ### ndk-r4
    stdcpplib = ''
    toolchain = os.path.join(ndkroot, 'build', 'prebuilt')

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
        ### ndk-r5/ndk-r6
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
        postlinkflags.insert(0, stdcpplib)
        postshlinkflags.insert(0, stdcpplib)
        sysroot = archdir

    gccprefix = api
    if os.path.exists(tcdir) and not os.path.exists(os.path.join(tcdir, api + '-gcc')):
        try:
            dirs = os.listdir(tcdir)
            gccprefix = os.path.commonprefix(dirs)
            gccprefix = gccprefix[0:len(gccprefix) - 1]
        except:
            pass

    env['gccdir'] = gccdir
    env['gccver'] = env['android_compiler'].replace(api + '-', '')
    env['gccabi'] = 'arm' in api and 'armeabi' or api
    env['toolchain'] = toolchain
    env['archdir'] = archdir
    env['stdcpplib'] = stdcpplib
    env['android_arch'] = arch
    env['PKG_CONFIG_LIBDIR'] = os.path.join (archdir, 'usr', 'lib', 'pkgconfig')
    env['CC'] = os.path.join (tcdir, gccprefix + '-gcc')
    env['CXX'] = os.path.join (tcdir, gccprefix + '-g++')
    #env['LINK'] = os.path.join (tcdir, gccprefix + '-g++')
    env['AR'] = os.path.join (tcdir, gccprefix + '-ar')
    env['STRIP'] = os.path.join (tcdir, gccprefix + '-strip')
    env['RANLIB'] = os.path.join (tcdir, gccprefix + '-ranlib')
    env['LD'] = os.path.join (tcdir, gccprefix + '-ld')
    env['SHLIBSUFFIX'] = '.so'
    env.AppendUnique (CPPDEFINES = ['ANDROID', '__ANDROID__', 'SEXY_ANDROID'])
    env.AppendUnique (CPPPATH = [os.path.join(archdir, 'usr', 'include')],
                      LINKFLAGS = [('--sysroot', sysroot)],
                      POSTLINKFLAGS = postlinkflags,
                      POSTSHLINKFLAGS = postshlinkflags,
                      LIBPATH = [os.path.join(archdir, 'usr', 'lib')])
    env.AppendUnique(CCFLAGS = ['-g'], LINKFLAGS = ['-g'])

    SetupCPUFlags(env)
    env['TARGET_SUBARCH'] = abimap[arch]
    if  env['android_libabi'] in ('', 'default'):
        subarch = env['android_cpu_subarch']
        if subarch in subabimap:
            env['android_libabi'] = subabimap[subarch]
            env['TARGET_SUBARCH'] = subabimap[subarch]
        else:
            env['android_libabi'] = abimap[arch]

    env.AppendUnique(CCFLAGS = ['-fno-short-enums'])
    env.AppendUnique(CPPDEFINES = ['_GLIBCXX_USE_WCHAR_T'])

    ### replace the Program() and SharedLibrary()
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

    ### fix `undefined __dso_handle' error on x86
    if arch == 'x86':
        dsoh = env.SharedObject(os.path.join(os.path.dirname(__file__), '__dso_handle.c'),
                                SHOBJSUFFIX = '.' + env['TARGET_SUBARCH'] + env['SHOBJSUFFIX'])
        env['dso_handle'] = dsoh

