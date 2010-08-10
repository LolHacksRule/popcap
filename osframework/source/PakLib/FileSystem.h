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
#include <time.h>

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

	enum FileType
	{
		FT_REGULAR    = 1 << 8,
		FT_DIRECTORY  = 1 << 9 ,
		FT_PIPE       = 1 << 10,
		FT_CHAR       = 1 << 11,
		FT_BLOCK      = 1 << 12,
		FT_SOCKET     = 1 << 13,
		FT_SYMLINK    = 1 << 14
	};

	enum FilePerm
	{
		FP_READ  = 1 << 0,
		FP_WRITE = 1 << 1,
		FP_EXEC  = 1 << 2,
		FP_SGID  = 1 << 3,
		FP_SUID  = 1 << 4
	};

	struct FileInfo
	{
		std::string   name;
		int           mode;
		time_t        ctime;
		time_t        mtime;
		time_t        atime;
		unsigned long size;
	};

	struct DirEntry
	{
		std::string   name;
		int           mode;
	};

	class Dir
	{
	public:
		virtual ~Dir() {};

	public:
		virtual bool                   read(DirEntry *entry) = 0;
		virtual void                   close() = 0;
	};

	class FileSystemDriver;
	class FileSystem
	{
	public:
		FileSystem(FileSystemDriver *driver,
			   const std::string &location,
			   int priority);
		virtual ~FileSystem() {}

		int getPriority() const;
		FileSystemDriver * getDriver() const;
		std::string getLocation() const;

	public:
		virtual File*			open(const char* theFileName,
						     const char* theAccess) = 0;
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess) = 0;

		virtual Dir*                    openDir(const char *theDir) = 0;

	protected:
		FileSystemDriver*               mDriver;
		int                             mPriority;
		std::string                     mLocation;
	};
}

#endif
