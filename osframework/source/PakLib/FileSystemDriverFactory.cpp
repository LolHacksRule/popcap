#include "FileSystemDriverFactory.h"

using namespace PakLib;

FileSystemDriver::FileSystemDriver (const std::string theName,
				    int		      thePriority)
	: Driver(theName, thePriority)
{
}

FileSystemDriver::~FileSystemDriver ()
{
}

FileSystemDriverFactory::FileSystemDriverFactory ()
	: DriverFactory ()
{
}

FileSystemDriverFactory::~FileSystemDriverFactory ()
{
}
