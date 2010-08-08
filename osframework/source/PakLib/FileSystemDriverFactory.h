#ifndef __FILE_SYSTEM_DRIVERFACTORY_H__
#define __FILE_SYSTEM_DRIVERFACTORY_H__

#include "DriverFactory.h"
#include "FileSystem.h"

namespace PakLib {

class FileSystemDriver: public Driver
{
 public:
	virtual FileSystem* Create (const std::string &location,
				    const std::string &type,
				    int                priority) = 0;


 public:
	FileSystemDriver (const std::string theName,
			  int	            thePriority = 0);
	~FileSystemDriver ();
};


class FileSystemDriverFactory: public DriverFactory
{
 public:
	FileSystemDriverFactory ();
	~FileSystemDriverFactory ();
};

}

#endif
