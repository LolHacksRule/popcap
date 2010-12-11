#include "ZipFileSystem.h"
#include "Find.h"
#include "AutoCrit.h"

#include <zzip/zzip.h>

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

using namespace PakLib;

class ZipFile: public File
{
public:
	ZipFile(ZZIP_FILE *fp, size_t size) : mFp(fp), mSize(size)
	{
	}

	~ZipFile()
	{
		if (mFp)
			zzip_close(mFp);
	}

	int seek(long theOffset, int theOrigin)
	{
		return zzip_seek(mFp, theOffset, theOrigin);
	}

	int tell()
	{
		return zzip_tell(mFp);
	}

	size_t read(void* thePtr, int theElemSize, int theCount)
	{
		return zzip_fread(thePtr, theElemSize, theCount, mFp);
	}

	size_t write(const void* thePtr, int theElemSize, int theCount)
	{
		return 0;
	}

	int getc()
	{
		char c;
		if (read(&c, sizeof(c), 1) != 1)
			return -1;
		return c;
	}

	int ungetc(int theChar)
	{
		return seek(-1, SEEK_CUR);
	}

	char* gets(char* thePtr, int theSize)
	{
		for (int i = 0; i < theSize; i++)
		{
			int c = getc();
			if (c < 0)
				return 0;
			thePtr[i] = (char)c;
			if (c == '\0')
				return thePtr;
		}

		return thePtr;
	}

	wchar_t* gets(wchar_t* thePtr, int theSize)
	{
		for (int i = 0; i < theSize; i++)
		{
			wchar_t c;

			if (read(&c, sizeof(c), 1) != 1)
				return 0;
			thePtr[i] = c;
			if (c == '\0')
				return thePtr;
		}

		return thePtr;
	}

	int eof()
	{
		return (size_t)tell() == mSize;
	}

	void close()
	{
		delete this;
	}

private:
	ZZIP_FILE *mFp;
	size_t mSize;
};

ZipFileSystem::ZipFileSystem(FileSystemDriver  * driver,
			     const std::string & location,
			     const std::string & prefix,
			     int                 priority,
			     ZZIP_DIR*           dir)
	: FileSystem(driver, location, priority), mZZipDir(dir),
	  mPrefix(prefix)
{
}

ZipFileSystem::~ZipFileSystem()
{
	ZipFileSystemDriver* driver = (ZipFileSystemDriver*)mDriver;
	AutoCrit autoCrit(driver->mCritSect);

	zzip_dir_close(mZZipDir);
}

static char * p_wcstombs(const wchar_t * theString)
{
        char * aString;
        size_t length;

#if defined(ANDROID)
	length = wcslen(theString);
        aString = new char[length + 1];
	for (size_t i = 0; i < length + 1; i++)
	    aString[i] = (char)theString[i];
#else
	length = wcstombs(NULL, theString, 0);
        aString = new char[length + 1];
        wcstombs(aString, theString, length + 1);
#endif
        return aString;
}

#if 0
static wchar_t * p_mbstowcs(const char * theString)
{
        wchar_t * aString;
        size_t length = mbstowcs(NULL, theString, 0);

        aString = new wchar_t[length + 1];
        mbstowcs(aString, theString, length + 1);

        return aString;
}
#endif

File* ZipFileSystem::open(const char* theFileName,
			  const char* theAccess)
{
	if (!strcmp(theAccess, "w") || !strcmp(theAccess, "a") ||
	    !strcmp(theAccess, "a+"))
		return 0;

	if (theFileName[0] == '/' || theFileName[1] == '\\')
		return 0;

#ifdef WIN32
	if (strlen(theFileName) >= 2 && isalpha(theFileName[0]) &&
	    theFileName[1] == ':')
		return 0;
#endif

	std::string filename;

	filename = mPrefix + std::string(PATH_SEP) + std::string(theFileName);
	ZZIP_FILE* zzipFile =
		zzip_file_open(mZZipDir,
			       filename.c_str(),
			       ZZIP_ONLYZIP | ZZIP_CASELESS);
	if (zzipFile)
	{
		// Get uncompressed size too
		ZZIP_STAT zstat;

		zzip_dir_stat(mZZipDir, filename.c_str(),
			      &zstat, ZZIP_CASEINSENSITIVE);
		return new ZipFile(zzipFile, zstat.st_size);
	}

	return 0;
}

File* ZipFileSystem::open(const wchar_t* theFileName,
			  const wchar_t* theAccess)
{
	File *result;
	char *aFileName = p_wcstombs(theFileName);
        char *aAccess = p_wcstombs(theAccess);

        result = open(aFileName, aAccess);

        delete [] aFileName;
        delete [] aAccess;
	return result;
}

Dir* ZipFileSystem::openDir(const char *thePath)
{
	return 0;
}

ZipFileSystemDriver::ZipFileSystemDriver(int priority)
	: FileSystemDriver("zip", priority)
{
}

FileSystem* ZipFileSystemDriver::Create(const std::string &location,
				 const std::string &type,
				 int                priority)
{
	if (type != "zip" || location.empty())
		return false;

	AutoCrit autoCrit(mCritSect);
	std::string path;
	std::string prefix;
	size_t pos = location.find("::");

	if (pos > 0)
	{
	    path = location.substr(0, pos);
	    prefix = location.substr(pos + 2, location.length() - pos - 2);
	}
	else
	{
	    path = location;
	}
	zzip_error_t zzipError;
	ZZIP_DIR *dir = zzip_dir_open(path.c_str(), &zzipError);
	if (!dir)
		return false;

	return new ZipFileSystem(this, path, prefix, priority, dir);
}
