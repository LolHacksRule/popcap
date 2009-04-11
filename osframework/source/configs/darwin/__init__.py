# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.AddOptions (opts)
    configs.FreeTypeAddOptions (opts)

def Configure (env):
    configs.Configure (env)
    env.AppendUnique (CPPDEFINES = ['SEXY_DARWIN'],
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
