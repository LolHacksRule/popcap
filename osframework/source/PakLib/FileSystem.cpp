#include "FileSystem.h"
#include "FileSystemManager.h"

using namespace PakLib;

File::~File()
{
	FileSystemManager *manager = FileSystemManager::getManager();
	if (manager)
		manager->removeFile(this);
}

FileSystem::FileSystem(FileSystemDriver * driver,
		       const std::string &location,
		       int                priority)
	: mDriver(driver), mPriority(priority),
	  mLocation(location)
{
}

int FileSystem::getPriority() const
{
	return mPriority;
}

FileSystemDriver* FileSystem::getDriver() const
{
	return mDriver;
}

std::string FileSystem::getLocation() const
{
	return mLocation;
}
