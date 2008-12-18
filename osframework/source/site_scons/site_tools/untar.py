import SCons.Action
import SCons.Builder
import SCons.Defaults
import SCons.Node.FS
import SCons.Util
import os

def UntarPrefix(target):
    targets = [ t.path for t in target ]
    prefix = os.path.commonprefix(targets)
    return os.path.split(os.path.normpath(prefix))[0]

def UntarMatch(name, filter_list):
    for i in filter_list:
        ### XXX more powerful matcher?
        ### such as fnmacth or regular expression?
        if i in name:
            return True
    return False

def Untar(target, source, env):
    import tarfile

    tar = tarfile.open(str(source[0]), "r")
    if len (source) > 1:
        include = source[1].value
    else:
        include = []
    if len (source) > 2:
        exclude = source[2].value
    else:
        exclude = []

    prefix = UntarPrefix(target)
    try:
        for member in tar.getmembers():
            if len(include) and not UntarMatch (member.name, include):
                continue
            if len(exclude) and UntarMatch (member.name, exclude):
                continue
            tar.extract(member, prefix)
    finally:
        tar.close()

    return 0

def UntarStr(target, source, env):
    prefix = UntarPrefix(target)
    print("Untarring: %s to %s" % (source[0].path, prefix))

def UntarEmitter(target, source, env):
    import tarfile

    tar = tarfile.open(str(source[0]), "r")
    if len (source) > 1:
        include = source[1].value
    else:
        include = []
    if len (source) > 2:
        exclude = source[2].value
    else:
        exclude = []

    target = []
    try:
        for member in tar.getmembers():
            if member.type == tarfile.DIRTYPE:
                continue
            if len(include) and not UntarMatch (member.name, include):
                continue
            if len(exclude) and UntarMatch (member.name, exclude):
                continue
            entry = env.fs.Entry(member.name)
            target = target + [entry]
    finally:
        tar.close()

    ## for t in target:
    ##     print ("Target(" + repr(t) + "): "+ t.path)

    ## for s in source:
    ##     print ("Source(" + repr(s) + "): "+ s.path)

    return (target, source)

UntarAction = SCons.Action.Action(Untar, UntarStr)
UntarBuilder = SCons.Builder.Builder (action = UntarAction,
                                      emitter = UntarEmitter,
                                      target_factory = SCons.Node.FS.default_fs.Entry)
def generate(env):
    """Add Builders and construction variables for tar to an Environment."""

    try:
        bld = env['BUILDERS']['_Untar']
    except KeyError:
        bld = UntarBuilder
        env['BUILDERS']['_Untar'] = bld

    def Untar(source, include = [], exclude = []):
        if SCons.Util.is_List (include):
            include = SCons.Node.Python.Value (include)
        else:
            include = SCons.Node.Python.Value ([include])
        if SCons.Util.is_List (exclude):
            exclude = SCons.Node.Python.Value (exclude)
        else:
            exclude = SCons.Node.Python.Value ([exclude])
        source = [ source, include, exclude ]
        return env._Untar([], source)

    env.Untar = Untar

def exists(env):
    try:
        import tarfile
        return True
    except:
        return False
