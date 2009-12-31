from SCons.Builder import Builder
from SCons.Util    import WhereIs, is_List

def generate(env):
    try:
        env["MSGFMT"]
    except KeyError:
        env["XGETTEXT"] = WhereIs("xgettext")
        env["MSGFMT"] = WhereIs("msgfmt")
        msgfmt = Builder(action = "$MSGFMT -c --statistics -o $TARGET $SOURCE",
                         src_suffix = ".po",
                         suffix = ".mo",
                         single_source = True)
        env["BUILDERS"]["Msgfmt"] = msgfmt

        env["MSGMERGE"] = WhereIs("msgmerge")
        msgmerge = Builder(action = "$MSGMERGE $TARGET $SOURCE -o $TARGET",
                           src_suffix = ".pot",
                           suffix = ".po",
                           single_source = True)
        env["BUILDERS"]["MsgMerge"] = msgmerge

        env["MSGINIT"] = WhereIs("msginit")
        msginit = Builder(action = "$MSGINIT -i $SOURCE -o $TARGET --no-translator",
                          src_suffix = ".pot",
                          suffix = ".po",
                          single_source = True)
        env["BUILDERS"]["MsgInit"] = msginit

        def MsgInitMerge(env, target, source):
            #print target[0].path, os.path.exists(target[0].abspath)
            if os.path.exists(target[0].abspath):
                return env.MsgMerge(target, source)
            else:
                return env.MsgInit(target, source)
        env["BUILDERS"]["MsgInitMerge"] = MsgInitMerge

def exists():
    return True
