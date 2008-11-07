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
