#ifndef __PAKLIB_FILESYSTEM_H__
#define __PAKLIB_FILESYSTEM_H__

#include <vector>
#include <string>

#define NOMINMAX
#include "PakInterface.h"
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#define stricmp(x, y) strcasecmp(x, y)
#define strnicmp(x, y, l) strncasecmp(x, y, l)
#endif
#include <stdio.h>

namespace PakLib {
	class File
	{
	public:
		virtual ~File() {}

	public:
		virtual int			seek(long theOffset, int theOrigin) = 0;
		virtual int			tell() = 0;
		virtual size_t			read(void* thePtr, int theElemSize, int theCount) = 0;
		virtual size_t			write(const void* thePtr, int theElemSize, int theCount) = 0;
		virtual int			getc() = 0;
		virtual int			ungetc(int theChar) = 0;
		virtual char*			gets(char* thePtr, int theSize) = 0;
		virtual wchar_t*		gets(wchar_t* thePtr, int theSize) = 0;
		virtual int			eof() = 0;
	};

	class FileSystem
	{
	public:
		virtual ~FileSystem() {}
	public:
		virtual bool                    addResource(const std::string &location,
							    const std::string &type,
							    int priority = 0) = 0;

		virtual File*			open(const char* theFileName,
						     const char* theAccess) = 0;
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess) = 0;

		virtual PakHandle		findFirst(PakFileNamePtr lpFileName,
							  PakFindDataPtr lpFindFileData) = 0;
		virtual bool			findNext(PakHandle hFindFile,
							 PakFindDataPtr lpFindFileData) = 0;
		virtual bool			findClose(PakHandle hFindFile) = 0;

	public:
		std::string                     mName;
	};

	class FileSystemManager: FileSystem
	{
	private:
		FileSystemManager();
		~FileSystemManager();

	public:
		static FileSystemManager*      getManager();

	public:
		bool                           addResource(const std::string &location,
							   const std::string &type,
							   int priority = 0);

	public:
		virtual File*			open(const char* theFileName,
						     const char* theAccess);
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess);

		virtual PakHandle		findFirst(PakFileNamePtr lpFileName,
							  PakFindDataPtr lpFindFileData);
		virtual bool			findNext(PakHandle hFindFile,
							 PakFindDataPtr lpFindFileData);
		virtual bool			findClose(PakHandle hFindFile);

	private:
		bool mInitialized;

		typedef std::vector<FileSystem*> FileSystemVector;
		FileSystemVector mFileSystems;
	};

}

#endif
