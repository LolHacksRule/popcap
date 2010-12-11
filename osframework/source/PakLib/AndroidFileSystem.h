#ifndef __ANDROID_FILE_SYSTEM__
#define __ANDROID_FILE_SYSTEM__

#include "FileSystem.h"
#include "CritSect.h"
#include "FileSystemDriverFactory.h"

namespace PakLib {

	class AndroidFileSystem: public FileSystem
	{
	public:
		AndroidFileSystem(FileSystemDriver  *driver,
				  const std::string &location,
				  int                priority);
		virtual ~AndroidFileSystem();

	public:
		virtual File*			open(const char* theFileName,
						     const char* theAccess);
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess);
		virtual Dir*                    openDir(const char* thePath);

	 public:
	};

	class AndroidFileSystemDriver: public FileSystemDriver
	{
		friend class AndroidFileSystem;
	public:
	        AndroidFileSystemDriver(int priority = 0);

		virtual FileSystem* Create (const std::string &location,
					    const std::string &type,
					    int                priority);

	private:
		CritSect            mCritSect;
		bool                mAdded;
	};
}

#endif
