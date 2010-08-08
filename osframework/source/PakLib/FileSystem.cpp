#include "FileSystem.h"

using namespace PakLib;

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
