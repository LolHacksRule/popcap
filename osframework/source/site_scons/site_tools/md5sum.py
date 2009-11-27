#!/usr/bin/python

from string import Template
import stat

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

def md5sum_string(target, source, env):
    return "Caculating md5sum '%s' from '%s'" % (str(target[0]), str(source[0]))

def md5sum_emitter(target, source, env):
    target = []
    for s in source:
        target.append(env.File(s.abspath + '.md5sum'))
    return (target, source)

def generate(env, **kw):
    action = env.Action(md5sum_action, md5sum_string)
    env['BUILDERS']['MD5SUM'] = env.Builder(action = action,
                                            emitter = md5sum_emitter,
                                            suffix = '.md5sum')

def exists(env):
    try:
        import hashlib
        return True
    except:
        return False
