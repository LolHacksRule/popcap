#ifndef __NATIVE_FILE_SYSTEM__
#define __NATIVE_FILE_SYSTEM__

#include "FileSystem.h"
#include "FileSystemDriverFactory.h"

namespace PakLib {

	class NativeFileSystem: public FileSystem
	{
	public:
		NativeFileSystem(FileSystemDriver   *driver,
				 const std::string  &location,
				 int                 priority);
		virtual ~NativeFileSystem();

	 private:
		NativeFileSystem(const NativeFileSystem &other);

	public:
		virtual File*			open(const char* theFileName,
						     const char* theAccess);
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess);
		virtual Dir*                    openDir(const char *theDir);

	 public:
		std::string                     mLocation;
	};

	class NativeFileSystemDriver: public FileSystemDriver
	{
	public:
	        NativeFileSystemDriver(int priority = 0);

		virtual FileSystem* Create (const std::string &location,
					    const std::string &type,
					    int                priority);
	};
}

#endif
