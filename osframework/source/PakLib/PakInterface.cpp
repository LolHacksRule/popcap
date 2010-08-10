#include "PakInterface.h"
#include "FileSystemManager.h"

#include <stdio.h>

using namespace PakLib;

PFILE* p_fopen(const char* theFileName, const char* theAccess)
{
	FileSystemManager *manager = FileSystemManager::getManager();
	return (PFILE*)manager->open(theFileName, theAccess);
}

PFILE* p_fopen(const wchar_t* theFileName, const wchar_t* theAccess)
{
	FileSystemManager *manager = FileSystemManager::getManager();
	return (PFILE*)manager->open(theFileName, theAccess);
}

int p_fclose(PFILE* theFile)
{
	File* f = (File*)theFile;

	f->close();
	return 0;
}

int p_fseek(PFILE* theFile, long theOffset, int theOrigin)
{
	File* f = (File*)theFile;

	if (!f)
		return -1;
	return f->seek(theOffset, theOrigin);
}

int p_ftell(PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return -1;
	return f->tell();
}

size_t p_fread(void* thePtr, int theSize, int theCount, PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return -1;
	return f->read(thePtr, theSize, theCount);
}

size_t p_fwrite(const void* thePtr, int theSize, int theCount, PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return -1;
	return f->write(thePtr, theSize, theCount);
}

int p_fgetc(PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return -1;
	return f->getc();
}

int p_ungetc(int theChar, PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return -1;
	return f->ungetc(theChar);
}

char* p_fgets(char* thePtr, int theSize, PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return 0;
	return f->gets(thePtr, theSize);
}

wchar_t* p_fgets(wchar_t* thePtr, int theSize, PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return 0;

	return f->gets(thePtr, theSize);
}

int p_feof(PFILE* theFile)
{
	File* f = (File*)theFile;

	if (!f)
		return -1;
	return f->eof();
}

PakHandle p_FindFirstFile(PakFileNamePtr lpFileName, PakFindDataPtr lpFindFileData)
{
	return 0;
}

bool p_FindNextFile(PakHandle hFindFile, PakFindDataPtr lpFindFileData)
{
	return false;
}

bool p_FindClose(PakHandle hFindFile)
{
	return true;
}

bool p_addResource(const char *location,
		   const char *type,
		   int         priority)
{
	FileSystemManager *manager = FileSystemManager::getManager();

	if (!location || !type)
		return false;

	return manager->addResource(std::string(location),
				    std::string(type),
				    priority);
}
