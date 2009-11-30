#!/usr/bin/python

from string import Template
import stat
import SCons

def md5sum(filename):
    import hashlib
    f = file(filename,'rb')
    return hashlib.md5(f.read()).hexdigest()

def md5sum_action(target, source, env):
    for i in range(len(source)):
        digest = md5sum(source[i].abspath)
        content = digest + ' ' + source[i].name + '\n'
        file(target[i].abspath, 'w').write(content)
    return 0

def md5sum_emitter(target, source, env):
    if len(target) < len(source):
        diff = len(source) - len(target)
        offset = len(target)
        for i in range(diff):
            s = source[offset + i]
            target.append(env.File(s.abspath + '.md5sum'))
    return (target, source)

def generate(env, **kw):
    try:
        env['BUILDERS']['MD5SUMSTR']
        env['BUILDERS']['MD5SUM']
    except KeyError:
        md5str = "Caculating md5sum: $TARGETS"
        action = SCons.Action.Action(md5sum_action, '$MD5SUMSTR')
        env['MD5SUMSTR'] = md5str
        env['BUILDERS']['MD5SUM'] = env.Builder(action = action,
                                                emitter = md5sum_emitter,
                                                suffix = '.md5sum')

def exists(env):
    try:
        import hashlib
        return True
    except:
        return False
