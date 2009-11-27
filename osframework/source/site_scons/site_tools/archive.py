import SCons.Action
import SCons.Builder
import SCons.Defaults
import SCons.Node.FS
import SCons.Util
import os
import fnmatch

SUFFIX = {
    'gztar': '.tar.gz',
    'bztar': '.tar.bz2',
    'ztar':  '.Z',
    'tar':   '.tar',
    'zip':   '.zip'
    }

def MakeZipfile (env, file_name, format, root_dir, base_dir):
    """Create a zip file from all the files under 'base_dir'.  The output
    zip file will be named 'base_dir' + ".zip".  Uses either the "zipfile"
    Python module (if available) or the InfoZIP "zip" utility (if installed
    and found on the default search path).
    """
    try:
        import zipfile
    except ImportError:
        return 1

    zip_filename = file_name

    z = zipfile.ZipFile(zip_filename, "w",
                        compression = zipfile.ZIP_DEFLATED)

    src_dir = os.path.join(root_dir, base_dir)
    for dirpath, dirnames, filenames in os.walk(src_dir):
        for name in filenames:
            path = os.path.normpath(os.path.join(dirpath, name))
            arch_path = path[len(root_dir):]
            if os.path.isfile(path):
                z.write(path, arch_path)
    z.close()

    return 0

def MakeTarball (env, file_name, format, root_dir, base_dir):
    """Create a (possibly compressed) tar file from all the files under
    'root_dir/basedir'.  'compress' must be "gzip" (the default), "compress",
    "bzip2", or None.  Both "tar" and the compression utility named by
    'compress' must be on the default program search path, so this is
    probably Unix-specific.  The output tar file will be named 'base_dir' +
    ".tar", possibly plus the appropriate compression extension (".gz",
    ".bz2" or ".Z").
    """
    compress = {
        'gztar': 'z',
        'bztar': 'j',
        'ztar':  'Z',
        'tar':   '',
    }
    if root_dir:
        return env.Execute ('$TAR -C %s -c%sf %s %s' % (root_dir, compress[format],
                                                        file_name, base_dir))
    return env.Execute ('$TAR -c%sf %s %s -C %s' % (compress[format], file_name, base_dir))

ARCHIVER = {
    'gztar': MakeTarball,
    'bztar': MakeTarball,
    'ztar':  MakeTarball,
    'tar':   MakeTarball,
    'zip':   MakeZipfile
    }

def MakeArchive(env, file_name, format, root_dir, base_dir):
    root_dir = os.path.normpath(root_dir)
    base_dir = os.path.normpath(base_dir)
    return ARCHIVER[format](env, file_name, format, root_dir, base_dir)

def Archive (target, source, env):
    name = source[0].value
    rootdir = env.Dir(source[1].value).abspath
    basedir = source[2].value
    format = source[3].value

    for i in range(len(format)):
        status = MakeArchive(env, target[i].path, format[i], rootdir, basedir)
        if status:
            return status
    return 0

ArchiveAction = SCons.Action.Action (Archive, '$ARCHIVESTR')
ArchiveBuilder = SCons.Builder.Builder (action = ArchiveAction,
                                        target_factory = SCons.Node.FS.default_fs.Entry)
def generate (env):
    """Add Builders and construction variables for Archive to an Environment."""

    def Archive (env, target = None, source = None, **kwargs):
        for arg in ['rootdir', 'basedir']:
            if not kwargs.has_key(arg):
                raise SCons.Errors.UserError, "Missing tag %s" % arg

        ### archive name
        if kwargs.has_key('name'):
            name = kwargs['name']
        else:
            name = os.path.basename(basedir)

        ### archive format
        if kwargs.has_key('format'):
            format = kwargs['format']
        else:
            format = 'zip'

        if not SCons.Util.is_List(format):
            format = [format]
        for f in format:
            if not SUFFIX.has_key(f):
                raise SCons.Errors.UserError, "Unsupported archive format: %s" % f

        rootdir = kwargs['rootdir']
        basedir = kwargs['basedir']

        if not target:
            target = []
        if not SCons.Util.is_List(target):
            target = [target]

        if len(target) < len(format):
            base = len(target)
            for i in range(len(format) - base):
                target += [env.File(str(name) + SUFFIX[format[i + base]])]

        source = [env.Value(name), env.Value(rootdir), env.Value(basedir),
                  env.Value(format)]
        return env._Archive(target = target, source = source)

    try:
        bld = env['BUILDERS']['_Archive']
    except KeyError:
        bld = ArchiveBuilder
        env['BUILDERS']['_Archive'] = bld
        env['ARCHIVESTR'] = "Compressing: $TARGETS"
        env['BUILDERS']['Archive'] = Archive

def exists (env):
    try:
        from distutils import archive_util
        return True
    except:
        return False
