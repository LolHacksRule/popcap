#!/usr/bin/python
import os

def GetGameDistDir(env, game):
    language = env['language']
    if language:
        destdir = os.path.join(env['distdir'], language, game)
    else:
        destdir = os.path.join(env['distdir'], game)
    return destdir

def FilterInstallableObject(env, sources, additional_suffixes = []):
    suffixes = [env['PROGSUFFIX'], env['SHLIBSUFFIX']]
    suffixes += additional_suffixes
    result = []
    for source in sources:
        for suffix in suffixes:
            if suffix in str(source):
                result += [source]
                break
    return result

def InstallObject(env, target, sources, additional_suffixes = [],
                  *args, **kwargs):
    return env.Install(target, FilterInstallableObject(env, sources),
                       *args, **kwargs)
