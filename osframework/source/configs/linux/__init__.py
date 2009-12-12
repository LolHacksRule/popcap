# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.AddOptions (opts)

    from SCons.Variables.BoolVariable import BoolVariable
    if 'build_32bit' in opts.keys ():
        return
    opts.Add (BoolVariable('build_32bit',
                           "build binaries as 32-bit on 64-bit platform.",
                           'False'))
    LinuxInputAddOptions (opts)

def Configure (env):
    configs.Configure (env)
    env['TARGET_OS'] = 'linux'
    if env['build_32bit']:
        env.AppendUnique (CCFLAGS = ['-m32'],
                          LINKFLAGS = ['-m32'])
    env.AppendUnique (CFLAGS = ['-pthread'],
                      LINKFLAGS = ['-pthread'])
    env.AppendUnique (CFLAGS = ['-g', '-fno-unit-at-a-time', '-Wall'],
                      CXXFLAGS = ['-g', '-Wall'],
                      LINKFLAGS = ['-g', '-fno-unit-at-a-time', '-export-dynamic'],
                      LIBS = ['rt', 'm'])
    if env['optimize']:
        env.AppendUnique (CCFLAGS = ['-O$optimize_level'])

    configs.linux.EnableLinuxUdpInputServer (env)
    configs.PosixModuleLoaderConfigure (env)
    configs.linux.LinuxInputConfigure (env)
    configs.linux.UdpInputConfigure (env)

def LinuxInputAddOptions (opts):
    from SCons.Variables.BoolVariable import BoolVariable
    if 'linux_input_grab_device' in opts.keys ():
        return
    opts.Add(BoolVariable ('linux_input_grab_device',
                           'Grab the input event device',
                           'True'))

def EnableLinuxInput (env):
    if env['linux_input_grab_device']:
        env.AppendUnique (CPPDEFINES = [('SEXY_LINUX_INPUT_GRAB_DEVICE', 1)])

def LinuxInputConfigure(env):
    env.AppendUnique (DRIVERS = ['LINUXINPUT'])
    linux_input = {}
    linux_input['ENABLE'] = EnableLinuxInput
    env.AppendUnique (LINUXINPUT = linux_input)

def UdpInputAddOptions (opts):
    pass

def EnableUdpInput (env):
    pass

def UdpInputConfigure(env):
    env.AppendUnique (DRIVERS = ['UDPINPUT'])
    udp_input = {}
    udp_input['ENABLE'] = EnableUdpInput
    env.AppendUnique (UDPINPUT = udp_input)

def SMInputAddOptions (opts):
    pass

def EnableSMInput (env):
    pass

def SMInputConfigure(env):
    pass

def EnableLinuxUdpInputServer(env):
    env['LUIS'] = True

def GstSoundAddOptions (opts):
    pass

def EnableGstSound (env):
    env.ParseConfig('$PKGCONFIG gstreamer-base-0.10 gstreamer-0.10 --cflags --libs')

def GstSoundConfigure(env):
    env.AppendUnique (DRIVERS = ['GSTSOUND'])
    gst_sound = {}
    gst_sound['ENABLE'] = EnableGstSound
    env.AppendUnique (GSTSOUND = gst_sound)
