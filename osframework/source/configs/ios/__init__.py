# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs
import os

def AddOptions (opts):
    configs.AddOptions (opts)
    configs.FreeTypeAddOptions (opts)

    from SCons.Variables import PathVariable, EnumVariable
    opts.Add(EnumVariable('iosplatform', 'the target of iPhoneOS', 'iPhoneOS', allowed_values = ['iPhoneOS', 'iPhoneSimulator']))
    opts.Add(PathVariable('devroot', 'the path to IOS Developer', '/Developer/Platforms/${iosplatform}.platform/Developer'))
    opts.Add(('sdkver', 'the IOS SDK version', '3.1.3'))
    opts.Add(PathVariable('sdkroot', 'the path to IOS SDK', '$devroot/SDKs/${iosplatform}${sdkver}.sdk'))
    opts.Add(EnumVariable('arch', 'target architecture', 'default', allowed_values = ['default', 'i386', 'armv6', 'armv7']))

def Configure (env):
    configs.Configure (env)
    tcdir = os.path.join('$devroot', 'usr', 'bin') 
    env['CC'] = os.path.join (tcdir, 'gcc')
    env['CXX'] = os.path.join (tcdir, 'g++')
    env['LINK'] = os.path.join (tcdir, 'g++')
    env['AR'] = os.path.join (tcdir, 'ar')
    env['STRIP'] = os.path.join (tcdir, 'strip')
    env['RANLIB'] = os.path.join (tcdir, 'ranlib')
    env['LD'] = os.path.join (tcdir, 'ld')

    if env['arch'] == 'default':
        if env['iosplatform'] == 'iPhoneOS':
            env['arch'] = 'armv6'
        else:
            env['arch'] = 'i386'

    if env['iosplatform'] == 'iPhoneSimulator':
        env.AppendUnique(CCFLAGS = ['-miphoneos-version-min=3.0'],
                         LINKFLAGS = ['-miphoneos-version-min=3.0'],
                         CPPDEFINES = [('__IPHONE_OS_VERSION_MIN_REQUIRED', 30000)])
    
    env.AppendUnique(CPPPATH = [os.path.join('$sdkroot', 'usr', 'lib', 'gcc', 'arm-apple-darwin9', '4.2.1', 'include'),
                                os.path.join('$sdkroot', 'usr', 'include'), os.path.join('$devroot', 'usr', 'include')],
                     LIBPATH = [os.path.join('$sdkroot', 'usr', 'lib'), os.path.join('$devroot', 'usr', 'lib')],
                     CCFLAGS = [('-arch', '$arch'), ('-isysroot', '$sdkroot')],
                     LINKFLAGS = [('-arch', '$arch'), ('--sysroot', '$sdkroot')],
		     FRAMEWORKPATH = [os.path.join('$sdkroot', 'System', 'Library', 'Frameworks')])
    env['TARGET_OS'] = 'darwin'
    env.AppendUnique (CPPDEFINES = ['SEXY_DARWIN', 'SEXY_IOS'],
                      CCFLAGS = ['-pthread'],
                      LINKFLAGS = ['-pthread'])
    env.AppendUnique (CFLAGS = ['-g', '-fno-unit-at-a-time', '-Wall'],
                      CXXFLAGS = ['-g', '-Wall'],
                      LINKFLAGS = ['-g', '-fno-unit-at-a-time'],
                      LIBS = ['m'])
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
