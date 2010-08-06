#ifndef  __NATIVE_FILE_SYSTEM__
#define __NATIVE_FILE_SYSTEM__

#include "FileSystem.h"

namespace PakLib {

	class NativeFileSystem: public FileSystem
	{
	public:
		NativeFileSystem();
		virtual ~NativeFileSystem();

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
		std::vector<std::string>        mLocations;
	};
}

#endif
