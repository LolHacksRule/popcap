#!/usr/bin/python
import os
from SCons.Defaults import *

def GetCurrentSrcDir(env):
    srcdir = env.Dir('.')
    if srcdir.srcdir:
        srcdir = srcdir.srcdir.abspath
    else:
        srcdir = srcdir.abspath
    return srcdir

def GetGameDistDir(env, game):
    language = env['language']
    if language:
        destdir = os.path.join(env['distdir'], language, game.lower())
    else:
        destdir = os.path.join(env['distdir'], game.lower())
    if env['install_prefix']:
        destdir = os.path.join(destdir, env['install_prefix'])
    return destdir

def AddExtraInstallableObject(env, objs):
    env['extras_objs'] += FilterInstallableObject(env, objs)

def GetInstallableObjectSuffix(env, additional_suffixes = []):
    suffixes = additional_suffixes
    suffixes += [env['SHLIBSUFFIX'], env['PROGSUFFIX']]
    ### hack for pdb
    if 'cl' in env['CC'] and not env['strip']:
        suffixes += ['.pdb']
    return suffixes

def StrHasSuffix(s, suf):
    if len(suf) > len(s):
        return False
    return s[len(s) - len(suf):] == suf

def FilterInstallableObjectAndSuffixes(env, sources, additional_suffixes = []):
    suffixes = GetInstallableObjectSuffix(env, additional_suffixes)
    result = []
    result_suffixes = []
    if type(sources) is not list and os.path.splitext(str(sources)) == '':
        return [sources], ['']
    for source in sources:
        for suffix in suffixes:
            if not suffix or StrHasSuffix(str(source), suffix):
                result += [source]
                result_suffixes += [suffix]
                break
    return result, result_suffixes

def FilterInstallableObject(env, sources, additional_suffixes = []):
    result = FilterInstallableObjectAndSuffixes(env, sources, additional_suffixes)[0]
    print sources, result
    return result

__stripped_suffix = '-stripped'
def StripObject(env, sources, sources_suffixes):
    targets = []
    if env['strip'] and env.has_key('STRIP'):
        renamed = []
        for i in range(len(sources)):
            if sources_suffixes[i]:
                renamed += [sources[i].name.replace(sources_suffixes[i],
                                                    __stripped_suffix + \
                                                    sources_suffixes[i])]
            else:
                renamed += [sources[i].name + __stripped_suffix]
        #print [str(s) for s in sources]
        #print renamed
        for i in range(len(sources)):
            targets += env.Command(renamed[i], sources[i],
                                   env['STRIP'] + ' -o ' + '$TARGET' + ' $SOURCE')
    else:
        return sources
    return targets

def InstallObject(env, target, sources, additional_suffixes = [],
                  *args, **kwargs):
    sources, sources_suffixes = FilterInstallableObjectAndSuffixes(env, sources,
                                                                   additional_suffixes)
    stripped = StripObject(env, sources, sources_suffixes)
    if stripped != sources:
        targets = []
        for i in range(len(sources)):
            targets += [os.path.join(target,
                                     stripped[i].name.replace(__stripped_suffix, ''))]
        return env.InstallAs(targets, stripped, *args, **kwargs)
    return env.Install(target, stripped, *args, **kwargs)

def __InstallGameExras(env, game, destdir, extras, basedir):
    targets = []
    basedir = os.path.abspath(basedir)
    for extra in extras:
        if extra.name[-3:] == '.in':
            filename = extra.name[:len(extra.name) - 3]
            result = env.ScanReplace(target = filename,
                                     source = extra)
            targets += env.Install(destdir, result)
        else:
            base = extra.abspath[len(basedir) + 1:]
            target = os.path.join(destdir, base)
            targets += env.InstallAs(target, extra)
    return targets

def GenerateVersion(env, target, source):
    import time
    f = file(target[0].abspath, 'w')
    verstr = time.strftime('%y.%m.%d.')
    verstr += '_'.join([str(s) for s in source]) + '.'
    verstr += env['oem'] + '\n'
    f.write(verstr)
    f.close()

def GetGitVersion(srcdir):
    olddir = os.getcwd()

    os.chdir(os.path.realpath(srcdir))
    gitcmd = 'git rev-parse HEAD'
    if sys.platform == 'win32':
        gitcmd += ' 2>null'
    else:
        gitcmd += ' 2>/dev/null'
    rev = os.popen(gitcmd).read()
    if not rev:
        print "Couldn't get repo version for ", srcdir

    os.chdir(olddir)
    if not rev:
        return 'UNKOWN'
    return rev[:8]

def Md5sum(filename):
    import hashlib
    f = file(filename,'rb')
    return hashlib.md5(f.read()).hexdigest()

def ListDirFiles(path, list_dir = False):
    result = []
    for root, dirs, files in os.walk(path):
        if list_dir:
            result += dirs
        for f in files:
            result.append(os.path.join(root, f))
    return result

def InstallGameExtras(env, game, destdir, targets = []):
    ### install extra objects
    targets += InstallObject(env, destdir, env['extras_objs'])

    ### install extra files
    for extra in env['extras']:
        targets += env.Install(destdir, extra)

    ### install oem specific files
    root = env.Dir('#').abspath
    dirs = ['default', game]
    for d in dirs:
        extra_dir = os.path.join(root, 'oem', env['oem'], d, env['TARGET_PLATFORM'])
        #print 'extra_dir = ', extra_dir
        extras = env.Glob(os.path.join(extra_dir, '*'))
        targets += __InstallGameExras(env, game, destdir, extras, extra_dir)

    ### generate version file
    srcdir = GetCurrentSrcDir(env)
    framework_rev = GetGitVersion(root)
    app_rev = GetGitVersion(srcdir)

    if not env.has_key('GAMEGENVERSIONSTR'):
        env['GAMEGENVERSIONSTR'] = 'Generating version file: $TARGET ($SOURCES)'
    targets += env.Command(os.path.join(destdir, 'version.txt'),
                           [env.Value(framework_rev), env.Value(app_rev)],
                           [SCons.Action.Action(GenerateVersion,
                                                '$GAMEGENVERSIONSTR')])
    return targets

def PackageGame(env, package_name, rootdir, targets = [], archive_format = None):
    ### setup default package format
    if not archive_format:
        if env['package_format']:
            archive_format = env['package_format']
        else:
            if env.has_key('TARGET_OS') and env['TARGET_OS'] == 'win32':
                archive_format = 'zip'
            else:
                archive_format = ['gztar', 'bztar']

    ### always use lower package name
    package_dir = os.path.dirname(package_name)
    package_base = os.path.basename(package_name)
    package_name = os.path.join(package_dir, package_base.lower())

    destdir = os.path.join(env['distdir'], env['language'], 'packages')
    tarball = env.Archive(name = package_name,
                          rootdir = os.path.dirname(rootdir),
                          basedir = os.path.basename(rootdir),
                          format = archive_format,
                          depends = targets)
    md5sum = env.MD5SUM(target = [], source = tarball)
    return env.Install(destdir, tarball + md5sum)

def InstallGame(env, name, prog, destdir, files, targets = []):
    targets += InstallObject(env, destdir, prog)
    targets += env.Install(destdir, files)

    env['GAME_EXE'] = FilterInstallableObject(env, prog)[0].name
    targets += InstallGameExtras(env, name, destdir, targets)

    ### package all the bits
    if env['release']:
        targets += PackageGame(env, name, destdir, targets)

    env.Alias('install', targets)

    return targets
