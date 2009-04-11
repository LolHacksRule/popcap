# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.AddOptions (opts)
    configs.FreeTypeAddOptions (opts)

    from SCons.Variables.BoolVariable import BoolVariable
    if 'build_32bit' in opts.keys ():
        return
    opts.Add (BoolVariable('build_32bit',
                           "build binaries as 32-bit on 64-bit platform.",
                           'False'))

def Configure (env):
    configs.Configure (env)
    if env['build_32bit']:
        env.AppendUnique (CCFLAGS = ['-m32'],
                          LINKFLAGS = ['-m32'])
    env.AppendUnique (CFLAGS = ['-pthread'],
                      LINKFLAGS = ['-pthread'])
    env.AppendUnique (CFLAGS = ['-g', '-fno-unit-at-a-time', '-Wall'],
                      CXXFLAGS = ['-g', '-Wall'],
                      LINKFLAGS = ['-g', '-fno-unit-at-a-time'],
                      LIBS = ['rt', 'm'])
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

def GstSoundAddOptions (opts):
    pass

def EnableGstSound (env):
    env.ParseConfig('$PKGCONFIG gstreamer-base-0.10 gstreamer-0.10 --cflags --libs')

def GstSoundConfigure(env):
    env.AppendUnique (DRIVERS = ['GSTSOUND'])
    gst_sound = {}
    gst_sound['ENABLE'] = EnableGstSound
    env.AppendUnique (GSTSOUND = gst_sound)
