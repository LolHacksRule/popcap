# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

def AddOptions(opts):
    from SCons.Variables.BoolVariable import BoolVariable
    if 'debug' in opts.keys():
        return
    opts.Add(BoolVariable('debug', 'build debug version', 'no'))
    opts.Add(BoolVariable('release', 'build release version', 'no'))

def Configure(env):
    env.AppendUnique(DRIVERS = [])
