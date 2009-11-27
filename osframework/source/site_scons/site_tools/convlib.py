# -*- coding: utf-8 -*-
# -*- python -*-
import SCons

def generate(env, **kwargs):
    """This is a utility function that creates the ConvenienceLibrary
    Builder in an Environment if it is not there already.

    If it is already there, we return the existing one.

    Based on the stock StaticLibrary and SharedLibrary builders.
    """
    try:
        convenience_lib = env['BUILDERS']['ConvenienceLibrary']
    except KeyError:
        action_list = [ SCons.Action.Action ("$ARCOM", "$ARCOMSTR") ]
        if env['BUILDERS'].has_key('RANLIB'):
            ranlib_action = SCons.Action.Action("$RANLIBCOM", "$RANLIBCOMSTR")
            action_list.append(ranlib_action)

        convenience_lib = SCons.Builder.Builder (action = action_list,
                                                 emitter = '$LIBEMITTER',
                                                 prefix = '$LIBPREFIX',
                                                 suffix = '$LIBSUFFIX',
                                                 src_suffix = '$SHOBJSUFFIX',
                                                 src_builder = 'SharedObject')
        env['BUILDERS']['ConvenienceLibrary'] = convenience_lib

def exists(env):
    return True
