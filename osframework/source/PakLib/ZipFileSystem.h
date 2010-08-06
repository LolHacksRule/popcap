#ifndef __ZIP_FILE_SYSTEM__
#define __ZIP_FILE_SYSTEM__

#include "FileSystem.h"
#include "CritSect.h"

typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;

namespace PakLib {

	class ZipFileSystem: public FileSystem
	{
	public:
		ZipFileSystem();
		virtual ~ZipFileSystem();

	public:
		virtual bool                    addResource(const std::string &location,
							    const std::string &type,
							    int priority = 0);

		virtual File*			open(const char* theFileName,
						     const char* theAccess);
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess);

		virtual PakHandle		findFirst(PakFileNamePtr lpFileName,
							  PakFindDataPtr lpFindFileData);
		virtual bool			findNext(PakHandle hFindFile,
							 PakFindDataPtr lpFindFileData);
		virtual bool			findClose(PakHandle hFindFile);

	 public:
		CritSect                        mCritSect;
		std::vector<ZZIP_DIR*>          mLocations;
	};
}

#endif
