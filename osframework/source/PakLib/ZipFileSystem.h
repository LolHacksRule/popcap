#ifndef __ZIP_FILE_SYSTEM__
#define __ZIP_FILE_SYSTEM__

#include "FileSystem.h"
#include "CritSect.h"
#include "FileSystemDriverFactory.h"

typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;

namespace PakLib {

	class ZipFileSystem: public FileSystem
	{
	public:
		ZipFileSystem(FileSystemDriver  *driver,
			      const std::string &location,
			      const std::string &prefix,
			      int                priority,
			      ZZIP_DIR*          dir);
		virtual ~ZipFileSystem();

	public:
		virtual File*			open(const char* theFileName,
						     const char* theAccess);
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess);
		virtual Dir*                    openDir(const char* thePath);

	 public:
		ZZIP_DIR*                       mZZipDir;
		std::string                     mPrefix;
	};

	class ZipFileSystemDriver: public FileSystemDriver
	{
		friend class ZipFileSystem;
	public:
	        ZipFileSystemDriver(int priority = 0);

		virtual FileSystem* Create (const std::string &location,
					    const std::string &type,
					    int                priority);

	private:
		CritSect            mCritSect;
	};
}

#endif
