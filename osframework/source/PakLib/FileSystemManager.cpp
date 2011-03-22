#include "FileSystemManager.h"
#include "NativeFileSystem.h"
#include "ZipFileSystem.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include "AndroidFileSystem.h"

#include <android/log.h>

#define  LOG_TAG    "PakLib"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

using namespace PakLib;

FileSystemManager::FileSystemManager() :
	FileSystem(0, "", 0)
{
	mInitialized = true;
	mLoaded = false;

	// Native/posix file system
	mFactory.AddDriver(new NativeFileSystemDriver());

	// zip file system
	mFactory.AddDriver(new ZipFileSystemDriver());
}

void FileSystemManager::addDefaultLocations()
{
	if (mLoaded)
		return;

	mLoaded = true;
	// native/posix file system
	addResource(".", "native", 0);

	// zip file system
	addResource("main.pak", "zip", 10);

#if defined(ANDROID) || defined(__ANDROID__)
	const char* pkg = getenv("ANDROID_SOURCE_DIR");
	if (pkg)
	    addResource(std::string(pkg) + std::string("::assets/files"),
			"zip", 20);
#endif
}

FileSystemManager::~FileSystemManager()
{
	FileSystemList::iterator it = mFileSystems.begin();
	for (; it != mFileSystems.end(); ++it)
		delete *it;
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

	addDefaultLocations();

	FileSystemDriver* driver = (FileSystemDriver*)mFactory.Find(type);
	if (!driver)
		return false;

	FileSystem* fileSystem = driver->Create(location, type, priority);
	if (!fileSystem)
		return false;

	mFileSystems.insert(fileSystem);
	return true;
}

File* FileSystemManager::open(const char* theFileName,
			      const char* theAccess)
{
	if (!mInitialized)
		return false;

	addDefaultLocations();

	if (!theFileName || !theAccess)
		return 0;

	File *fp;
	FileSystemList::iterator it = mFileSystems.begin();
	for (; it != mFileSystems.end(); ++it)
	{
		fp = (*it)->open(theFileName, theAccess);
		if (fp)
		{
#if 0
			printf("PakLib: file %s successfully opened(driver: %s device: %s).\n",
			       theFileName,
			       (*it)->getDriver()->GetName().c_str(),
			       (*it)->getLocation().c_str());
#endif
			return fp;
		}
	}
	return 0;
}

File* FileSystemManager::open(const wchar_t* theFileName,
			      const wchar_t* theAccess)
{
	if (!mInitialized)
		return false;

	addDefaultLocations();

	if (!theFileName || !theAccess)
		return 0;

	File *fp;
	FileSystemList::iterator it = mFileSystems.begin();
	for (; it != mFileSystems.end(); ++it)
	{
		fp = (*it)->open(theFileName, theAccess);
		if (fp)
			return fp;
	}
	return 0;
}

Dir* FileSystemManager::openDir(const char *theDir)
{
	if (!mInitialized)
		return false;

	addDefaultLocations();

	return 0;
}
