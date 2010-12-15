#!/usr/bin/python
import os
import sys
import stat

from SCons.Defaults import *
from SCons.Util import *

if not hasattr(os.path, 'relpath'):
    if sys.platform == 'win32':
        def RelPath(path, start=os.path.curdir):
            """Return a relative version of a path"""

            from os.path import abspath, join, sep, pardir, splitunc

            if not path:
                raise ValueError("no path specified")
            start_list = abspath(start).split(sep)
            path_list = abspath(path).split(sep)
            if start_list[0].lower() != path_list[0].lower():
                unc_path, rest = splitunc(path)
                unc_start, rest = splitunc(start)
                if bool(unc_path) ^ bool(unc_start):
                    raise ValueError("Cannot mix UNC and non-UNC paths (%s and %s)"
                                                                        % (path, start))
                else:
                    raise ValueError("path is on drive %s, start on drive %s"
                                                        % (path_list[0], start_list[0]))
            # Work out how much of the filepath is shared by start and path.
            for i in range(min(len(start_list), len(path_list))):
                if start_list[i].lower() != path_list[i].lower():
                    break
            else:
                i += 1

            rel_list = [pardir] * (len(start_list)-i) + path_list[i:]
            if not rel_list:
                return curdir
            return join(*rel_list)
    else:
        def RelPath(path, start=os.path.curdir):
            """Return a relative version of a path"""

            from os.path import abspath, join, sep, pardir, commonprefix

            if not path:
                raise ValueError("no path specified")

            start_list = abspath(start).split(sep)
            path_list = abspath(path).split(sep)

            # Work out how much of the filepath is shared by start and path.
            i = len(commonprefix([start_list, path_list]))

            rel_list = [pardir] * (len(start_list)-i) + path_list[i:]
            if not rel_list:
                return curdir
            return join(*rel_list)
else:
    RelPath = os.path.relpath

def GetCurrentSrcDir(env):
    srcdir = env.Dir('.').srcnode().abspath
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

def CopyDir(env, target, source):
    if not is_List(source):
        source = [source]
    nodes = env.arg2nodes(source, env.fs.Entry)
    targets = []
    for node in nodes:
        if env.fs.isfile (node.path):
            targets += env.Commond(os.path.join(str(target), node.name), node,
                                   SCons.Defaults.Copy ("$TARGET", "$SOURCE"))
        else:
            files = ListDirFiles(node.abspath)
            for f in files:
                relpath = os.path.relpath(f, node.abspath)
                targets += env.InstallAs(os.path.join(str(target),
                                                      os.path.basename(node.path),
                                                      relpath),
                                            SCons.Defaults.Copy ("$TARGET", "$SOURCE"))
    return targets

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
        gitcmd += ' 2>nul'
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

def ListDirFiles(path, empty_dir = False):
    result = []
    for root, dirs, files in os.walk(path):
        if not files and not dirs and empty_dir:
            result.append(root)
        for f in files:
            result.append(os.path.join(root, f))
    return result

def InstallDir(env, target, source):
    if not is_List(source):
        source = [source]
    nodes = env.arg2nodes(source, env.fs.Entry)
    targets = []
    for node in nodes:
        if env.fs.isfile (node.abspath):
            targets += env.Install(target, node)
        else:
            files = ListDirFiles(node.abspath)
            for f in files:
                relpath = RelPath(f, node.abspath)
                targets += env.InstallAs(os.path.join(str(target),
                                                      os.path.basename(node.path),
                                                      relpath), f)
    return targets

def InstallGameExtras(env, game, destdir, targets = []):
    ### install extra objects
    targets += InstallObject(env, destdir, env['extras_objs'])

    ### install objects
    objects = []
    for package in env['ENABLED_PACKAGES']:
        info = env['PACKAGES_INFO'][package]
        for dep in info['DEPENDS']:
            objects.append(env['PACKAGES_INFO'][dep]['OBJECTS'])
        objects.append(info['OBJECTS'])
    for obj in objects:
        targets += InstallObject(env, destdir, obj)

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
    targets += InstallDir(env, destdir, files)

    env['GAME_EXE'] = FilterInstallableObject(env, prog)[0].name
    targets += InstallGameExtras(env, name, destdir, targets)

    ### package all the bits
    if env['release']:
        targets += PackageGame(env, name, destdir, targets)

    env.Alias('install', targets)

    return targets

def APK(env, srcdir, origsrcdir, destdir, targets, android_dir = 'android',
        android_build_dir = 'android-build'):
    launcher = env['ANDROID_LAUNCHER']
    android = android_dir
    androidbuild = os.path.join(srcdir, android_build_dir)

    ### combine android project
    srcs = []
    androidabs = os.path.join(origsrcdir, android)
    for root, dirs, files in os.walk(os.path.join(launcher, 'src')):
        for f in files:
            if '.java' in f:
                src = os.path.join(root, f)
                reldir = root.replace(launcher + os.path.sep, '')
                dest = os.path.join(androidbuild, reldir, f)
                #print src, '->', dest
                srcs += env.Command(dest, src,
                                    SCons.Defaults.Copy ('$TARGET', '$SOURCE'))

    files = env.Glob(os.path.join(origsrcdir, android, '*'))
    for f in files:
        if f.name in ['local.properties', 'libs', 'bin']:
            continue
        srcs += InstallDir(env, androidbuild, f)

    ### generate local.properties
    def GenerateLocalProp(env, target, source):
        f = file(target[0].abspath, 'w')
        header = \
               "# This file is automatically generated by SCons.\n" \
               "# Do not modify this file -- YOUR CHANGES WILL BE ERASED!\n"
        f.write(header)
        f.write('sdk.dir=%s\n' % source[0].value)
        f.close()
        return 0

    lp = os.path.join(androidbuild, 'local.properties')
    sdk_path = os.path.expanduser(env['android_sdk_path'])
    srcs += env.Command(lp, env.Value(sdk_path),
                        GenerateLocalProp)

    ### copy *.so to 'android-build'
    jnis = env.Glob(os.path.join(destdir, '*.so'))
    srcs += env.Install(os.path.join(android_build_dir, 'libs',
                                     env['gccabi'].replace('-', '')),
                        jnis)

    ### copy resources files to asserts/files
    destdirabs = env.Dir(destdir).abspath
    filesdir = os.path.join(android_build_dir, 'assets', 'files')
    for f in targets:
        if not env.fs.isfile(f.abspath):
            continue
        if os.path.splitext(f.name)[1] == '.so':
            continue
        rel = f.abspath.replace(destdirabs + os.path.sep, '')
        if os.path.isabs(rel):
            print 'APK: ignoring', rel
            continue
        basedir = os.path.dirname(rel)
        base = os.path.basename(rel)
        if base[0] == '_':
            name, ext = os.path.splitext(base)
            rel = os.path.join(basedir, name[1:] + '_' + ext)
        srcs += env.InstallAs(os.path.join(filesdir, rel), f.abspath)


    from xml.dom import minidom
    build_xml = os.path.join(origsrcdir, android, 'build.xml')
    dom = minidom.parse(build_xml)
    root = dom.getElementsByTagName('project')
    name = str(root[0].getAttribute('name'))

    ### build the apk
    build_xml = os.path.join(env.Dir(srcdir).abspath, androidbuild, 'build.xml')
    target = 'debug'
    build = 'debug'
    if not env['debug']:
        target = 'release'
        build = 'unsigned'
    dest = '%s-%s.apk' % (name, build)
    apk = env.Command([os.path.join(androidbuild, 'bin', dest),
                       os.path.join(androidbuild, 'bin', name + '.ap_')], [],
                      'ant -f %s %s' % (build_xml, target))
    env.Depends(apk, srcs)

    pkgdir = os.path.join(env['distdir'], env['language'], 'packages')
    result = env.Install(pkgdir, apk[0])

    if target == 'debug':
        return result

    keystore = env.File(env['apk_sign_keystore'])
    if not os.path.exists(keystore.abspath):
        return result

    orig_apk = result
    ### signed the apk with our key
    if env['debug']:
        command = 'jarsigner -verbose'
    else:
        command = 'jarsigner'
    command += ' -keystore %s' % keystore.path
    command += ' -storepass %s -keypass %s' % (env['apk_sign_keystore_pass'], env['apk_sign_key_pass'])
    command += ' -signedjar $TARGET $SOURCE %s' % env['apk_sign_key']
    signed_apk = env.Command(name + '-signed.apk', orig_apk, command)

    ### align the signed apk
    command = os.path.join(env['android_sdk_path'], 'tools', 'zipalign')
    if env['debug']:
        command += ' -v'
    command += ' 4 $SOURCE $TARGET'
    aligned_apk = env.Command(name + '-aligned.apk', signed_apk, command)
    result += env.InstallAs(os.path.join(pkgdir, name + '-signed.apk'), aligned_apk)
    return result
