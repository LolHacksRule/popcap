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
    if 'cl' in env['CC']:
        suffixes += ['.pdb']
    return suffixes

def FilterInstallableObjectAndSuffixes(env, sources, additional_suffixes = []):
    suffixes = GetInstallableObjectSuffix(env, additional_suffixes)
    result = []
    result_suffixes = []
    for source in sources:
        for suffix in suffixes:
            if not suffix or suffix in str(source):
                result += [source]
                result_suffixes += [suffix]
                break
    ### hack for pdb
    if result and 'cl' in env['CC'] and env['debug'] and not env['strip']:
        result += [result[0].abspath + '.pdb']
        result_suffixes += ['.pdb']
    return result, result_suffixes

def FilterInstallableObject(env, sources, additional_suffixes = []):
    return FilterInstallableObjectAndSuffixes(env, sources, additional_suffixes)[0]

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
    verstr = 'TM.' + time.strftime('%y.%m.%d.')
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

def GetUpdateUrl(path):
    import ConfigParser
    config = ConfigParser.ConfigParser()
    config.read(os.path.join(path))
    if config.has_section('settings'):
        for item in config.items('settings'):
            if item[0] == 'url':
                return item[1]

def ListDirFiles(path, list_dir = False):
    result = []
    for root, dirs, files in os.walk(path):
        if list_dir:
            result += dirs
        for f in files:
            result.append(os.path.join(root, f))
    return result

def GenerateUpdateServiceXml(env, target, source):
    url = GetUpdateUrl(source[0].abspath)
    basedir = source[1].value
    files = source[2:]
    result = target[0].path

    ### list all files
    updates = []
    for source in files:
        if source.isdir():
            updates += ListDirFiles(source.abspath)
        else:
            updates.append(source.abspath)

    f = file(target[0].abspath, 'w')
    f.write ('<?xml version="1.0" encoding="utf-8"?>\n<updateFiles>\n')
    for up in updates:
        s = '    <file path=\"'
        file_path = up.replace(basedir + os.sep, '')
        file_path = file_path.replace(os.sep, '/')
        s += file_path + '\" url=\"' + url + '/' + file_path + '\" '
        s += 'md5sum=\"' + str(Md5sum(up)) + '\" needRestart=\"true\"/>\n'
        f.write(s)
    f.write('</updateFiles>\n')
    f.close()


def InstallGameExtras(env, game, destdir, targets = []):
    ### install extra objects
    targets += InstallObject(env, destdir, env['extras_objs'])

    ### install extra files
    for extra in env['extras']:
        targets += env.Install(destdir, extra)

    ### install oem specific files
    root = env.Dir('#').abspath
    dirs = ['default', "auth", game]
    for d in dirs:
        extra_dir = os.path.join(root, 'oem', env['oem'], d, env['TARGET_PLATFORM'])
        #print 'extra_dir = ', extra_dir
        extras = env.Glob(os.path.join(extra_dir, '*'))
        targets += __InstallGameExras(env, game, destdir, extras, extra_dir)

    ### generate the updateService.xml for auto-update
    if env['auto_update']:
        basename = 'updateclient.ini.in'
        source = os.path.join(root, 'oem', env['oem'],
                              env['TARGET_PLATFORM'], basename)
        if not os.path.exists (source):
            source = os.path.join(root, 'oem', basename)
        target = os.path.basename(source[:len(source) - 3])
        kvs = {}
        kvs['server_addr'] = env['update_server']
        kvs['gamename'] = game.lower()
        kvs['company'] = env['oem']
        kvs['remoter'] = 'default'
        kvs['language'] = env['language'].lower()
        kvs['use_auth'] = ['noauth', 'auth'][env['auth']]
        result = env.ScanReplace(target = target,
                                 source = [source, env.Value(kvs)])
        targets += env.Install(destdir, result)
        targets += env.Command(os.path.join(destdir, 'updateService.xml'),
                                [result, env.Value(env.Dir(destdir).abspath),
                                 targets],
                               GenerateUpdateServiceXml)

    ### generate version file
    srcdir = GetCurrentSrcDir(env)
    framework_rev = GetGitVersion(root)
    app_rev = GetGitVersion(srcdir)

    targets += env.Command(os.path.join(destdir, 'version'),
                           [env.Value(framework_rev),
                            env.Value(app_rev)],
                           GenerateVersion)
    return targets

def PackageGame(env, package_name, rootdir, targets = [], fmt = None):
    ### setup default package format
    if not fmt:
        if env.has_key('TARGET_OS') and env['TARGET_OS'] == 'win32':
            fmt = 'zip'
        else:
            fmt = ['gztar', 'bztar']

    ### always use lower package name
    package_dir = os.path.dirname(package_name)
    package_base = os.path.basename(package_name)
    package_name = os.path.join(package_dir, package_base.lower())

    destdir = os.path.join(env['distdir'], env['language'], 'packages')
    tarball = env.Archive(name = package_name,
                          rootdir = os.path.dirname(rootdir),
                          basedir = os.path.basename(rootdir),
                          format = fmt)
    if targets:
        env.Depends(tarball, targets)
    md5sum = env.MD5SUM(tarball)
    return env.Install(destdir, tarball + md5sum)

def InstallGame(env, name, prog, destdir, files, targets = []):
    targets += InstallObject(env, destdir, prog)
    targets += env.Install(destdir, files)

    env['GAME_EXE'] = FilterInstallableObject(env, prog)[0].name
    targets += InstallGameExtras(env, name, destdir, targets)

    ### package all the bits
    targets += PackageGame(env, name, destdir, targets)

    env.Alias('install', targets)

    return targets
