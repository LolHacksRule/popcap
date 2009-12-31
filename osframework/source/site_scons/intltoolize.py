#!/bin/env python
import os
import shutil

### generate/update domain.pot
def generatePOTFILES(env, target, source):
    s = '\n'.join([s.path for s in source])
    file(target[0].path, 'w').write(s + '\n')

def genPotFiles(env, podir, domain, source):
    return env.Command (os.path.join(podir, domain + '.potfiles'),
                        source, generatePOTFILES)

def remove_pot_cdate(path):
    return "".join(filter(lambda line: "POT-Creation-Date: " not in line, open(path).readlines()))

def update_pot(target, source, env):
    pot = target[0].path
    new_pot = source[0].path

    if not os.path.exists(new_pot):
        return
    if os.path.exists(pot):
        if remove_pot_cdate(new_pot) != remove_pot_cdate(pot):
            shutil.copy2(new_pot, pot)
    else:
        shutil.copy2(new_pot, pot)

def getsrcdir(env, srcdir):
    if type(srcdir) is str:
        srcdir = env.Dir(srcdir)
    return srcdir.srcnode().abspath

def getlinguas(env, srcdir, podirname):
    origsrcdir = getsrcdir(env, srcdir)
    podir = os.path.join(origsrcdir, podirname)
    if os.path.exists(os.path.join(podir, 'LINGUAS')):
        lines = env.Split(open(os.path.join(podir, "LINGUAS")).read())
        linguas = []
        for line in lines:
            linguas.append(env.File(os.path.join(podir, line + '.po')))
    else:
        linguas = env.Glob(os.path.join(podir, '*.po'))

    result = []
    for lingua in linguas:
        basename = os.path.basename(lingua.path)
        result.append(os.path.join(podir, basename))
    return result

def intltoolize(env, srcdir, podirname, domain,
                package = None, package_version = '1.0.0'):
    origsrcdir = getsrcdir(env, srcdir)
    podir = os.path.join(origsrcdir, podirname)
    buildpodir = os.path.join(srcdir, podirname)
    if not os.path.exists(os.path.join(podir, 'POTFILES')):
        return []

    targets = []
    patterns = env.Split(file(os.path.join(podir, 'POTFILES')).read())
    sources = [ env.Glob(os.path.join(str(srcdir), f)) for f in patterns ]

    potfiles = genPotFiles(env, buildpodir, domain, sources)

    potbuild_name = domain + '.pot' + '.build'
    pot_name = domain + '.pot'
    if package is None:
        package = domain
    command = '$XGETTEXT --package-name=%s --package-version=%s ' \
              '-o $TARGET --keyword=tr -f $SOURCE' % (package, package_version)
    potbuild = env.Command (os.path.join(podir, potbuild_name), potfiles,
                            command)
    env.Depends(potbuild, sources)

    pot = env.Command (os.path.join(podir, pot_name), potbuild, update_pot)
    env.Alias('update-pot', pot)
    env.Alias('update-po', pot)
    targets.append(pot)

    ### update *.po
    linguas = getlinguas(env, srcdir, podirname)
    for lingua in linguas:
        update_po = env.MsgInitMerge(env.File(lingua), pot)

        env.Precious(update_po)
        env.NoClean(update_po)
        env.Alias('update-po', update_po)

        targets.append(update_po)
    return targets

def potoxml(target, source, env):
    import po2xml
    po2xml.po2xml(source[0].path, target[0].path)

def installlocale(env, srcdir, podirname, domain, langs, localedir):
    origsrcdir = getsrcdir(env, srcdir)
    podir = os.path.join(origsrcdir, podirname)
    buildpodir = os.path.join(srcdir, podirname)
    linguas = getlinguas(env, srcdir, podirname)
    if type(langs) is not list:
        langs = [langs]
    pos = [ lang + '.po' for lang in langs ]
    targets = []
    for lingua in linguas:
        if os.path.basename(lingua) in pos:
            xml = os.path.join(buildpodir, lang + '.xml')
            xml = env.Command(xml, lingua, potoxml)
            targets += xml
            target = os.path.join(localedir, lang, domain + '.xml')
            targets += env.InstallAs(target, xml)
    return targets
