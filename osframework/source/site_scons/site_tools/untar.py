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

def Untar(target, source, env):
    import tarfile

    tar = tarfile.open(str(source[0]), "r")

    prefix = UntarPrefix(target)
    try:
        try:
            tar.extractall(prefix)
        except AttributeError, e:
            for member in tar.getmembers():
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

    target = []
    try:
        for member in tar.getmembers():
            if member.type == tarfile.DIRTYPE:
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
        bld = env['BUILDERS']['Untar']
    except KeyError:
        bld = UntarBuilder
        env['BUILDERS']['Untar'] = bld

def exists(env):
    try:
        import tarfile
        return True
    except:
        return False
