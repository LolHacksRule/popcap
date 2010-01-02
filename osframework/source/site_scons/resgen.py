#!/bin/env python

import os
from xml.dom import minidom

class ImageRes(object):
    def __init__(self, resid = '', prefix = ''):
        self.resid = resid
        self.prefix = prefix

class FontRes(object):
    def __init__(self, resid = '', prefix = ''):
        self.resid = resid
        self.prefix = prefix

class SoundRes(object):
    def __init__(self, resid = '', prefix = ''):
        self.resid = resid
        self.prefix = prefix

class ResGroup(object):
    def __init__(self, resid = ''):
        self.resid = resid
        self.images = []
        self.fonts = []
        self.sounds = []

class ResGen(object):
    def __init__(self, fpath = 'resource.xml'):
        self.fpath = fpath
        self.groups = []
        self.idprefix = ''

    def parse(self, fpath = None):
        if fpath is not None:
            self.fpath = fpath
        dom = minidom.parse(self.fpath)
        root = dom.getElementsByTagName('ResourceManifest')
        nodes = root[0].getElementsByTagName('Resources')
        for node in nodes:
            group = self.parseResource(node)
            self.groups.append(group)

    def parseResource(self, node):
        idprefix = ''
        group = ResGroup(node.getAttribute('id'))
        for subnode in node.childNodes:
            if subnode.nodeType != minidom.Node.ELEMENT_NODE:
                continue
            if subnode.tagName == 'SetDefaults':
                if subnode.hasAttribute('idprefix'):
                    idprefix = subnode.getAttribute('idprefix')
            elif subnode.tagName == 'Font':
                resid = subnode.getAttribute('id')
                res = FontRes(resid, idprefix)
                group.fonts.append(res)
            elif subnode.tagName == 'Image':
                resid = subnode.getAttribute('id')
                res = ImageRes(resid, idprefix)
                group.images.append(res)
            elif subnode.tagName == 'Sound':
                resid = subnode.getAttribute('id')
                res = SoundRes(resid, idprefix)
                group.sounds.append(res)
        return group

    header = """#ifndef __%s__ \n#define __%s__\n\n"""
    def writeHeader(self, name = 'Res', namespace = 'Sexy'):
        fp = file(name + '.h', 'wb')
        fp.write(ResGen.header % (name.upper(), name.upper()))
        fp.write('namespace %s {\n' % namespace)
        fp.write('\tclass ResourceManager;\n')
        fp.write('\tclass Image;\n')
        fp.write('\tclass Font;\n')
        fp.write('\n')
	fp.write('\tImage* LoadImageById(ResourceManager *theManager, int theId);\n')
	fp.write('\tbool ExtractResourcesByName(ResourceManager *theManager,'
                 'const char *theName);\n\n');

        for group in self.groups:
            self.writeGroupHeader(fp, group);

        self.writeGroupId(fp)

        fp.write('}\n\n')
        fp.write('#endif\n')
        fp.close()

    def writeGroupHeader(self, fp, group):
        fp.write('\t// %s Resources\n' % group.resid)
        fp.write('\tbool Extract%sResources(ResourceManager *theMgr);\n' % group.resid)
        for font in group.fonts:
            fp.write('\textern Font* %s%s;\n' % (font.prefix, font.resid))
        if group.fonts:
            fp.write('\n')
        for image in group.images:
            fp.write('\textern Image* %s%s;\n' % (image.prefix, image.resid))
        if group.images:
            fp.write('\n');
        for sound in group.sounds:
            fp.write('\textern int %s%s;\n' % (sound.prefix, sound.resid))
        if group.sounds:
            fp.write('\n')

    def writeGroupId(self, fp):
        fp.write('\tenum ResourceId\n')
        fp.write('\t{\n')
        for group in self.groups:
            for font in group.fonts:
                fp.write('\t\t%s%s_ID,\n' % (font.prefix, font.resid))
            for image in group.images:
                fp.write('\t\t%s%s_ID,\n' % (image.prefix, image.resid))
            for sound in group.sounds:
                fp.write('\t\t%s%s_ID,\n' % (sound.prefix, sound.resid))
        fp.write('\t\tRESOURCE_ID_MAX\n')
        fp.write('\t};\n')
	fp.write("""
        Image* GetImageById(int theId);
	Font* GetFontById(int theId);
	int GetSoundById(int theId);

	ResourceId GetIdByImage(Image *theImage);
	ResourceId GetIdByFont(Font *theFont);
	ResourceId GetIdBySound(int theSound);
	const char* GetStringIdById(int theId);
	ResourceId GetIdByStringId(const char *theStringId);\n""")

    def writeCPP(self, name = 'Res', namespace = 'Sexy'):
        fp = file(name + '.cpp', 'wb')
        fp.write('#include "%s.h"\n' % name)
        fp.write('#include "ResourceManager.h"\n')
        fp.write('\n')
        fp.write('using namespace Sexy;\n')
        if namespace != 'Sexy':
            fp.write('using namespace %s\n' % namespace)
        fp.write('\n')

        fp.write('static bool gNeedRecalcVariableToIdMap = false;\n\n');

        self.writeCPPERBN(fp)
        self.writeCPPGIBSI(fp)

        for group in self.groups:
            self.writeCPPGroup(fp, group, namespace)

        self.writeCPPResourceID(fp)
        self.writeCPPGetResources(fp)

        fp.close()

    def writeCPPERBN(self, fp):
        fp.write("""bool Sexy::ExtractResourcesByName(ResourceManager *theManager, const char *theName)
{\n""")
        for group in self.groups:
            fp.write("""\tif (strcmp(theName, "%s") == 0)\n""" % group.resid)
            fp.write("""\t\treturn Extract%sResources(theManager);\n""" % group.resid)

        fp.write("""
	return false;
}\n\n""")

    def writeCPPGIBSI(self, fp):
        fp.write(
"""Sexy::ResourceId Sexy::GetIdByStringId(const char *theStringId)
{
	typedef std::map<std::string,int> MyMap;
	static MyMap aMap;
	if (aMap.empty())
	{
		for(int i = 0; i < RESOURCE_ID_MAX; i++)
			aMap[GetStringIdById(i)] = i;
	}

	MyMap::iterator anItr = aMap.find(theStringId);
	if (anItr == aMap.end())
		return RESOURCE_ID_MAX;
	else
		return (ResourceId) anItr->second;
}\n\n""")

    def writeCPPGroup(self, fp, group, namespace):
        fp.write('// %s Resources\n' % group.resid)
        for font in group.fonts:
            fp.write('Font* %s::%s%s;\n' % (namespace, font.prefix, font.resid))
        if group.fonts:
            fp.write('\n')
        for image in group.images:
            fp.write('Image* %s::%s%s;\n' % (namespace, image.prefix, image.resid))
        if group.images:
            fp.write('\n');
        for sound in group.sounds:
            fp.write('int %s::%s%s;\n' % (namespace, sound.prefix, sound.resid))
        if group.sounds:
            fp.write('\n')


        fp.write("""bool Sexy::Extract%sResources(ResourceManager *theManager)\n""" % group.resid)
        fp.write('{\n')
        fp.write('\tgNeedRecalcVariableToIdMap = true;\n\n')
	fp.write('\tResourceManager &aMgr = *theManager;\n\n')
        fp.write('\ttry\n')
        fp.write('\t{\n')

        for font in group.fonts:
            fp.write('\t\t%s%s = aMgr.GetFontThrow("%s%s");\n' % \
                     (font.prefix, font.resid,
                      font.prefix, font.resid))
        for image in group.images:
            fp.write('\t\t%s%s = aMgr.GetImageThrow("%s%s");\n' % \
                     (image.prefix, image.resid,
                      image.prefix, image.resid))
        for sound in group.sounds:
            fp.write('\t\t%s%s = aMgr.GetSoundThrow("%s%s");\n' % \
                     (sound.prefix, sound.resid,
                      sound.prefix, sound.resid))
        fp.write('\t}\n')
	fp.write('\tcatch(ResourceManagerException&)\n')
	fp.write('\t{\n')
        fp.write('\t\treturn false;\n')
        fp.write('\t}\n')
        fp.write('}\n\n')

    def writeCPPResourceID(self, fp):
        fp.write('static void* gResources[] =\n')
        fp.write('{\n')

        for group in self.groups:
            for res in group.fonts + group.images + group.sounds:
                fp.write('\t&%s%s,\n' % (res.prefix, res.resid))

        fp.write('\tNULL\n')
        fp.write('};\n\n')

    def writeCPPGetResources(self, fp):
        fp.write(
"""Image* Sexy::LoadImageById(ResourceManager *theManager, int theId)
{
	return (*((Image**)gResources[theId]) = theManager->LoadImage(GetStringIdById(theId)));
}

Image* Sexy::GetImageById(int theId)
{
	return *(Image**)gResources[theId];
}

Font* Sexy::GetFontById(int theId)
{
	return *(Font**)gResources[theId];
}

int Sexy::GetSoundById(int theId)
{
	return *(int*)gResources[theId];
}

static Sexy::ResourceId GetIdByVariable(const void *theVariable)
{
	typedef std::map<int,int> MyMap;
	static MyMap aMap;
	if (gNeedRecalcVariableToIdMap)
	{
		gNeedRecalcVariableToIdMap = false;
		aMap.clear();
		for(int i = 0; i < RESOURCE_ID_MAX; i++)
			aMap[*(int*)gResources[i]] = i;
	}

	MyMap::iterator anItr = aMap.find(*(int*)theVariable);
	if (anItr == aMap.end())
		return RESOURCE_ID_MAX;
	else
		return (ResourceId) anItr->second;
}

Sexy::ResourceId Sexy::GetIdByImage(Image *theImage)
{
	return GetIdByVariable(theImage);
}

Sexy::ResourceId Sexy::GetIdByFont(Font *theFont)
{
	return GetIdByVariable(theFont);
}

Sexy::ResourceId Sexy::GetIdBySound(int theSound)
{
	return GetIdByVariable((void*)theSound);
}\n\n""")

        fp.write("""const char* Sexy::GetStringIdById(int theId)\n""")
        fp.write("{\n")
	fp.write("\tswitch (theId)\n")
	fp.write("\t{\n")


        for group in self.groups:
            for res in group.fonts + group.images + group.sounds:
                fp.write('\t\tcase %s%s_ID:\n' % (res.prefix, res.resid))
                fp.write('\t\t\treturn "%s%s";\n' % (res.prefix, res.resid))
        fp.write('\t\tdefault:\n\t\t\tbreak;\n')

        fp.write("\t}\n\n")
        fp.write('\treturn "";\n')
        fp.write("}\n\n")

    def write(self, name = 'Res'):
        self.writeHeader(name)
        self.writeCPP(name)

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print 'USAGE: resource.xml'
        sys.exit(-1)

    resgen = ResGen()
    resgen.parse(sys.argv[1])
    resgen.write('Res')
