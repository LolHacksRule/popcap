# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.AddOptions (opts)

def Configure (env):
    configs.Configure (env)
    env.AppendUnique (CFLAGS = ['-pthread'],
                      LINKFLAGS = ['-pthread'])
    env.AppendUnique (CFLAGS = ['-g', '-fno-unit-at-a-time', '-Wall'],
                      CXXFLAGS = ['-g', '-Wall'],
                      LINKFLAGS = ['-g', '-fno-unit-at-a-time'],
                      LIBS = ['rt', 'm'])
    configs.linux.EnableLinuxUdpInputServer (env)
    configs.PosixModuleLoaderConfigure (env)

def LunuxInputAddOptions (opts):
    pass

def EnableLinuxInput (env):
    pass

def LinuxInputConfigure(env):
    configs.linux.Configure (env)
    env.AppendUnique (DRIVERS = ['LINUXINPUT'])
    linux_input = {}
    linux_input['ENABLE'] = EnableLinuxInput
    env.AppendUnique (LINUXINPUT = linux_input)

def UdpInputAddOptions (opts):
    pass

def EnableUdpInput (env):
    pass

def UdpInputConfigure(env):
    configs.linux.Configure (env)
    env.AppendUnique (DRIVERS = ['UDPINPUT'])
    udp_input = {}
    udp_input['ENABLE'] = EnableUdpInput
    env.AppendUnique (UDPINPUT = udp_input)

def EnableLinuxUdpInputServer(env):
    env.AppendUnique(CPPDEFINES = ['LINUXUDPINPUTSERVER'])

def GstSoundAddOptions (opts):
    pass

def EnableGstSound (env):
    env.ParseConfig('$PKGCONFIG gstreamer-base-0.10 gstreamer-0.10 --cflags --libs')

def GstSoundConfigure(env):
    configs.linux.Configure (env)
    env.AppendUnique (DRIVERS = ['GSTSOUND'])
    gst_sound = {}
    gst_sound['ENABLE'] = EnableGstSound
    env.AppendUnique (GSTSOUND = gst_sound)
