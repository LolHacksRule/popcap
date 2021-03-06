#!/bin/env python
import os
import shutil

from SCons.Defaults import *
from SCons.Script import COMMAND_LINE_TARGETS, ARGUMENTS
from SCons.Action import Action

### generate/update domain.pot
def generatePOTFILES(env, target, source):
    s = '\n'.join([s.path for s in source])
    file(target[0].path, 'w').write(s + '\n')

def genPotFiles(env, podir, domain, source):
    return env.Command (os.path.join(podir, domain + '-POTFILES.gen'),
                        source, Action(generatePOTFILES, "Generating $TARGET"))

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

def get_update_pot_targets():
    target = ARGUMENTS.get('update-pot', '')
    if not target:
        return []
    targets = target.split(',')
    result = []
    for t in targets:
        t.strip()
        if t:
            result.append(t)
    return result

def get_update_po_targets():
    target = ARGUMENTS.get('update-po', '')
    if not target:
        return []
    targets = target.split(',')
    result = []
    for t in targets:
        t.strip()
        if t:
            result.append(t)
    return result

def intltoolize(env, srcdir, podirname, domain,
                package = None, package_version = '1.0.0'):
    origsrcdir = getsrcdir(env, srcdir)
    podir = os.path.join(origsrcdir, podirname)
    buildpodir = os.path.join(srcdir, podirname)
    if not os.path.exists(os.path.join(podir, 'POTFILES')):
        return []

    targets = []
    if 'update-pot' in COMMAND_LINE_TARGETS or 'update-po' in COMMAND_LINE_TARGETS or \
       domain in get_update_pot_targets() or domain in get_update_po_targets():
        patterns = env.Split(file(os.path.join(podir, 'POTFILES')).read())
        sources = [ env.Glob(os.path.join(str(srcdir), f)) for f in patterns ]

        potfiles = genPotFiles(env, buildpodir, domain, sources)

        potbuild_name = domain + '.pot' + '.build'
        pot_name = domain + '.pot'
        if package is None:
            package = domain
        command = '$XGETTEXT --package-name=%s --package-version=%s ' \
                  '-o $TARGET --keyword=tr_noop --keyword=tr_cnoop:1c,2 ' \
                  '--keyword=tr_dnoop:2 --keyword=tr_dcnoop:2c,3 ' \
                  '--keyword=tr --keyword=dtr:2 --keyword=tr:1c,2' \
                  '--keyword=dtr:2c,3 -f $SOURCE' % \
                  (package, package_version)
        potbuild = env.Command (os.path.join(buildpodir, potbuild_name), potfiles,
                                command)
        env.AlwaysBuild(potbuild)
        env.Depends(potbuild, sources)

        pot = env.Command (os.path.join(podir, pot_name), potbuild,
                           Action(update_pot, "Updating $TARGET from $SOURCE"))
        env.AlwaysBuild(pot)
        env.Precious(pot)
        env.NoClean(pot)
        env.Alias('update-pot', pot)
        env.Alias('update-po', pot)
        targets.append(pot)

    ### update *.po
    if 'update-po' in COMMAND_LINE_TARGETS or domain in get_update_po_targets():
        linguas = getlinguas(env, srcdir, podirname)
        for lingua in linguas:
            update_po = env.MsgInitMerge(env.File(lingua), pot)

            env.AlwaysBuild(update_po)
            env.Precious(update_po)
            env.NoClean(update_po)
            env.Alias('update-po', update_po)

        targets.append(update_po)
    return targets

def potoxml(target, source, env):
    import po2xml
    po2xml.po2xml(source[0].path, target[0].path)

def expandlangs(env, srcdir, podirname, langs):
    if type(langs) is str and langs == 'all':
        langs = []
        linguas = getlinguas(env, srcdir, podirname)
        for lin in linguas:
            langs.append(os.path.splitext(os.path.basename(str(lin)))[0])
    return langs

def installLocale(env, srcdir, podirname, domain, langs, localedir):
    origsrcdir = getsrcdir(env, srcdir)
    podir = os.path.join(origsrcdir, podirname)
    buildpodir = os.path.join(srcdir, podirname)
    linguas = getlinguas(env, srcdir, podirname)
    langs = expandlangs(env, srcdir, podirname, langs)
    if type(langs) is not list:
        langs = [langs]
    pos = [ lang + '.po' for lang in langs ]
    targets = []
    for lingua in linguas:
        baselingua = os.path.basename(lingua)
        if baselingua in pos and os.path.exists(lingua):
            xml = os.path.join(buildpodir, lingua + '.xml')
            xml = env.Command(xml, lingua,
                              Action(potoxml,
                                     "Converting $SOURCE to $TARGET"))
            targets += xml
            lang = os.path.splitext(baselingua)[0]
            target = os.path.join(localedir, lang, domain + '.xml')
            targets += env.InstallAs(target, xml)
    return targets

def accumtexts(target, source, env):
    import polib
    fp = file(target[0].abspath, 'w')
    fp.write('a')
    for lingua in source:
        if lingua.suffix == '.po':
            po = polib.pofile(lingua.abspath)
            entries = [po.metadata_as_entry()]
            entries += [e for e in po if not e.obsolete]
            for entry in entries:
                fp.write(unicode(entry).encode('utf-8'))
        else:
            lfp = file(lingua.abspath, 'r')
            for line in lfp.readlines():
                fp.write(line)
    fp.close()

def stripFonts(env, srcdir, fonts, podirname, destdir, langs):
    origsrcdir = getsrcdir(env, srcdir)
    podir = os.path.join(origsrcdir, podirname)
    buildpodir = os.path.join(srcdir, podirname)
    linguas = getlinguas(env, srcdir, podirname)
    langs = expandlangs(env, srcdir, podirname, langs)
    if type(langs) is not list:
        langs = [langs]
    if type(fonts) is not list:
        fonts = [fonts]

    targets = []
    ### accumtexts
    pos = [ lang + '.po' for lang in langs ]
    texts = os.path.join('font-extractor.txt')
    toaccums = []
    for lingua in linguas:
        if os.path.basename(lingua) in pos and os.path.exists(lingua):
            toaccums.append(lingua)
    targets += env.Command(texts, toaccums,
                           accumtexts)

    ### strip fonts
    fontforge = env.WhereIs('fontforge')
    for font in fonts:
        if fontforge:
            basename = os.path.basename(str(font))
            tmpname = os.path.splitext(basename)[0] + '-striped' + os.path.splitext(basename)[1]
            fontstrip = env.File(os.path.join('#tools', 'fontforge', 'fontstrip.pe')).path
            command = 'fontforge %s --font ${SOURCES[0]} -i ${SOURCES[1]} -o $TARGET 2>%s' % \
	              (fontstrip, os.devnull)
            sfont = env.Command(tmpname, [font, texts], command)
            targets += env.InstallAs(os.path.join(destdir, basename), sfont)
        else:
            targets += env.Install(destdir, font)

    return targets
