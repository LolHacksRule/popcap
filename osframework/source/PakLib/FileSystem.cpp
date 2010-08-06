#include "FileSystem.h"
#include "NativeFileSystem.h"
#include "ZipFileSystem.h"

using namespace PakLib;

FileSystemManager::FileSystemManager()
{
	mInitialized = true;

	// native/posix file system
	mFileSystems.push_back(new NativeFileSystem());

	// zip file system
	mFileSystems.push_back(new ZipFileSystem());
	mFileSystems.back()->addResource("main.pak", "zip");
}

FileSystemManager::~FileSystemManager()
{
	for (size_t i = 0; i < mFileSystems.size(); i++)
		delete mFileSystems[i];
	mFileSystems.clear();
	mInitialized = false;
}

FileSystemManager* FileSystemManager::getManager()
{
	static FileSystemManager manager;
	return &manager;
}

bool FileSystemManager::addResource(const std::string &location,
				    const std::string &type,
				    int priority)
{
	if (!mInitialized)
		return false;
	for (size_t i = 0; i < mFileSystems.size(); i++)
		if (mFileSystems[i]->mName == type)
			return mFileSystems[i]->addResource(location,
							    type, priority);
	return false;
}

File* FileSystemManager::open(const char* theFileName,
			      const char* theAccess)
{
	if (!mInitialized)
		return false;

	if (!theFileName || !theAccess)
		return 0;

	File *fp;
	for (size_t i = 0; i < mFileSystems.size(); i++)
	{
		fp = mFileSystems[i]->open(theFileName, theAccess);
		if (fp)
			return fp;
	}
	return 0;
}

File* FileSystemManager::open(const wchar_t* theFileName,
			      const wchar_t* theAccess)
{
	if (!mInitialized)
		return false;

	if (!theFileName || !theAccess)
		return 0;

	File *fp;
	for (size_t i = 0; i < mFileSystems.size(); i++)
	{
		fp = mFileSystems[i]->open(theFileName, theAccess);
		if (fp)
			return fp;
	}
	return 0;
}

PakHandle FileSystemManager::findFirst(PakFileNamePtr lpFileName,
				       PakFindDataPtr lpFindFileData)
{

	if (!mInitialized)
		return false;
	return 0;
}

bool FileSystemManager::findNext(PakHandle hFindFile,
				 PakFindDataPtr lpFindFileData)
{
	return false;
}

bool FileSystemManager::findClose(PakHandle hFindFile)
{
	return true;
}
