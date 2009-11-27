#!/usr/bin/python

from string import Template
import stat

def replace_action(target, source, env):
    if len(source) > 1:
        d = source[1].value
    else:
        d = env
    open(str(target[0]), 'w').write(Template(open(str(source[0]), 'r').read()).safe_substitute(d))
    st = env.fs.stat(source[0].path)
    env.fs.chmod(target[0].path, stat.S_IMODE(st[stat.ST_MODE]) | stat.S_IWRITE)

    return 0

def replace_string(target, source, env):
    return "building '%s' from '%s'" % (str(target[0]), str(source[0]))

def generate(env, **kw):
    action = env.Action(replace_action, replace_string)
    env['BUILDERS']['ScanReplace'] = env.Builder(action = action,
                                                 src_suffix = '.in')

def exists(env):
    return True

