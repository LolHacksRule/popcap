# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs
import SCons

def AddOptions(opts):
    configs.AddOptions(opts)

res_action = SCons.Action.Action('$RCCOM', '$RCCOMSTR')
res_builder = SCons.Builder.Builder(action = res_action, suffix = '.res.o',
                                    source_scanner=SCons.Tool.SourceFileScanner)

def SetupCompiler(env):
    env.Tool('mingw')
    prefix = 'i686-pc-mingw32-'
    env['BUILDERS']['RES'] = res_builder
    env['CC'] = prefix + 'gcc'
    env['CXX'] = prefix + 'g++'
    env['AR'] = prefix + 'ar'
    env['AS'] = prefix + 'as'
    env['LINK'] = prefix + 'gcc'
    env['RANLIB'] = prefix + 'ranlib'
    env['RC'] = prefix + 'windres'
    env['OBJSUFFIX'] = '.o'
    env['LIBPREFIX'] = 'lib'
    env['LIBSUFFIX'] = '.a'
    env['SHLIBSUFFIX'] = '.dll'
    env['SHLIBPREFIX'] = ''
    env['PROGSUFFIX'] = '.exe'
    env['SHOBJSUFFIX'] = '.o'
    env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
    env['SHCCFLAGS'] = SCons.Util.CLVar('$CCFLAGS')

def Configure(env):
    SetupCompiler(env)
    configs.Configure(env)
    env['TARGETOS'] = 'windows'
    env['WIN_PROG_FLAGS'] = '-mwindows'
    env.AppendUnique(CPPDEFINES = ['WIN32'])
    env.AppendUnique(CFLAGS = ['-g', '-fno-unit-at-a-time', '-Wall'],
                     CXXFLAGS = ['-g', '-Wall'],
                     LINKFLAGS = ['-g', '-fno-unit-at-a-time'],
                     LIBS = ['stdc++', 'm'])
    configs.Win32ModuleLoaderConfigure (env)
