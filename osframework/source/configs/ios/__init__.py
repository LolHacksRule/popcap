# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs
import os

def AddOptions (opts):
    configs.AddOptions (opts)
    configs.FreeTypeAddOptions (opts)

    from SCons.Variables import PathVariable, EnumVariable
    opts.Add(EnumVariable('ios_platform', 'the target of iPhoneOS', 'iPhoneOS', allowed_values = ['iPhoneOS', 'iPhoneSimulator']))
    opts.Add(PathVariable('ios_devroot', 'the path to IOS Developer', '/Developer/Platforms/${ios_platform}.platform/Developer'))
    opts.Add(('ios_sdkver', 'the IOS SDK version', '3.1.3'))
    opts.Add(PathVariable('ios_sdkroot', 'the path to IOS SDK', '$ios_devroot/SDKs/${ios_platform}${ios_sdkver}.sdk'))
    opts.Add(EnumVariable('ios_arch', 'target architecture', 'default', allowed_values = ['default', 'i386', 'armv6', 'armv7']))

def Configure (env):
    configs.Configure (env)
    tcdir = os.path.join('$ios_devroot', 'usr', 'bin') 
    env['CC'] = os.path.join (tcdir, 'gcc')
    env['CXX'] = os.path.join (tcdir, 'g++')
    env['LINK'] = os.path.join (tcdir, 'g++')
    env['AR'] = os.path.join (tcdir, 'ar')
    env['STRIP'] = os.path.join (tcdir, 'strip')
    env['RANLIB'] = os.path.join (tcdir, 'ranlib')
    env['LD'] = os.path.join (tcdir, 'ld')

    if env['ios_arch'] == 'default':
        if env['ios_platform'] == 'iPhoneOS':
            env['ios_arch'] = 'armv6'
        else:
            env['ios_arch'] = 'i386'

    if env['ios_platform'] == 'iPhoneSimulator':
        env.AppendUnique(CCFLAGS = ['-miphoneos-version-min=3.0', '-fobjc-abi-version=2',
				    '-fobjc-legacy-dispatch'],
                         LINKFLAGS = ['-miphoneos-version-min=3.0', '-fobjc-abi-version=2'],
                         CPPDEFINES = [('__IPHONE_OS_VERSION_MIN_REQUIRED', 30000)])
    
    env.PrependUnique(CPPPATH = [os.path.join('$ios_sdkroot', 'usr', 'lib', 'gcc', 'arm-apple-darwin9', '4.2.1', 'include'),
                                os.path.join('$ios_sdkroot', 'usr', 'include'), os.path.join('$ios_devroot', 'usr', 'include')],
                     LIBPATH = [os.path.join('$ios_sdkroot', 'usr', 'lib'), os.path.join('$ios_devroot', 'usr', 'lib')],
                     CCFLAGS = [('-arch', '$ios_arch'), ('-isysroot', '$ios_sdkroot')],
                     LINKFLAGS = [('-arch', '$ios_arch'), ('--sysroot', '$ios_sdkroot')],
		     FRAMEWORKPATH = [os.path.join('$ios_sdkroot', 'System', 'Library', 'Frameworks')])
    env['TARGET_OS'] = 'darwin'
    env.AppendUnique (CPPDEFINES = ['SEXY_DARWIN', 'SEXY_IOS'],
                      CCFLAGS = ['-pthread'],
                      LINKFLAGS = ['-pthread'])
    env.AppendUnique (CFLAGS = ['-g', '-fno-unit-at-a-time', '-Wall'],
                      CXXFLAGS = ['-g', '-Wall'],
                      LINKFLAGS = ['-g', '-fno-unit-at-a-time'],
                      LIBS = ['m'])
    if env['optimize']:
        env.AppendUnique(CCFLAGS = ['-O$optimize_level'])

    configs.PosixModuleLoaderConfigure (env)
    configs.FreeTypeConfigure (env)

def UdpInputAddOptions (opts):
    pass

def EnableUdpInput (env):
    pass

def UdpInputConfigure(env):
    env.AppendUnique (DRIVERS = ['UDPINPUT'])
    udp_input = {}
    udp_input['ENABLE'] = EnableUdpInput
    env.AppendUnique (UDPINPUT = udp_input)
