import SCons.Action
import SCons.Builder
import SCons.Defaults
import SCons.Node.FS
import SCons.Util
import os
import fnmatch

def UntarPrefix (target):
    targets = [ t.path for t in target ]
    prefix = os.path.commonprefix(targets)
    return os.path.normpath(prefix)

def UntarMatch (name, filter_list):
    for i in filter_list:
        if fnmatch.fnmatch (name, i):
            return True
    return False

def strippath(path, basedir, strip):
    import os.path
    path = os.path.normpath(path)
    if strip:
        path = os.path.sep.join(path.split(os.path.sep)[strip:])
    if basedir == '.':
        return path
    return os.path.join(basedir, path)

def detectstrip(prefix, p):
    """auto detect patch strip level"""
    basedir = prefix
    for strip in range(7):
        for fileno, filename in enumerate(p.source):
            f2patch = strippath(filename, basedir, strip)
            if not os.path.exists(f2patch):
                f2patch = strippath(p.target[fileno], basedir, strip)
                if not os.path.exists(f2patch):
                    continue
            if not os.path.isfile(f2patch):
                continue
            return strip
    return 0

def applypatch(prefix, filename):
    """apply a patch"""
    import patch

    p = patch.fromfile(filename)
    p.apply(prefix, detectstrip(prefix, p))
    return 0

def Untar (target, source, env):
    import tarfile

    tar = tarfile.open (str (source[0]), "r")
    if len (source) > 1:
        include = source[1].value
    else:
        include = []
    if len (source) > 2:
        exclude = source[2].value
    else:
        exclude = []

    if len(source) > 3:
        patches = source[3:]
    else:
        patches = []

    prefix = UntarPrefix (target)
    extract_dir = os.path.split(prefix)[0]
    try:
        for member in tar.getmembers ():
            if len(include) and not UntarMatch (member.name, include):
                continue
            if len(exclude) and UntarMatch (member.name, exclude):
                continue
            tar.extract(member, extract_dir)
    finally:
        tar.close()

    for filename in patches:
        print prefix, filename.path
        applypatch(prefix, filename.path)
    return 0

def UntarStr (target, source, env):
    prefix = os.path.split(UntarPrefix (target))[0]
    print("Untarring: %s to %s" % (source[0].path, prefix))

def UntarEmitter (target, source, env):
    import tarfile

    tar = tarfile.open (str(source[0]), "r")
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
        for member in tar.getmembers ():
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

UntarAction = SCons.Action.Action (Untar, UntarStr)
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

    def Untar(source, include = [], exclude = [], patches = []):
        if SCons.Util.is_List (include):
            include = SCons.Node.Python.Value (include)
        else:
            include = SCons.Node.Python.Value ([include])
        if SCons.Util.is_List (exclude):
            exclude = SCons.Node.Python.Value (exclude)
        else:
            exclude = SCons.Node.Python.Value ([exclude])
        if not SCons.Util.is_List (patches):
            patches = [patches]
        source = [ source, include, exclude ]
        source += patches
        return env._Untar([], source)

    env.Untar = Untar

def exists(env):
    try:
        import tarfile
        import patch
        return True
    except:
        return False
