#include "ZipFileSystem.h"
#include "FileSystemManager.h"
#include "Find.h"
#include "AutoCrit.h"

#include <zzip/zzip.h>
#include <zzip/plugin.h>

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

using namespace PakLib;

class ZipFile: public File
{
public:
	ZipFile(ZipFileSystem *filesystem, ZZIP_FILE *fp,
			zzip_off_t size) : mFileSystem(filesystem), mFp(fp), mSize(size)
	{
	}

	~ZipFile()
	{
		AutoCrit autoCrit(mFileSystem->mCritSect);
		if (mFp)
			zzip_close(mFp);
	}

	int seek(long theOffset, int theOrigin)
	{
		AutoCrit autoCrit(mFileSystem->mCritSect);
		if (zzip_seek(mFp, theOffset, theOrigin) < 0)
			return -1;
		return 0;
	}

	long tell()
	{
		AutoCrit autoCrit(mFileSystem->mCritSect);
		return zzip_tell(mFp);
	}

	size_t read(void* thePtr, int theElemSize, int theCount)
	{
		AutoCrit autoCrit(mFileSystem->mCritSect);
		zzip_ssize_t ret = zzip_fread(thePtr, theElemSize, theCount, mFp);
		if (ret < 0)
			return 0;
		return ret;
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
		AutoCrit autoCrit(mFileSystem->mCritSect);
		return zzip_tell(mFp) == mSize;
	}

	void close()
	{
		delete this;
	}

	long getSize()
	{
		return mSize;
	}

private:
	ZipFileSystem *mFileSystem;
	ZZIP_FILE *mFp;
	zzip_off_t mSize;
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
       if (theFileName[0] == '.' && theFileName[1] == '/')
               theFileName += 2;

	std::string theFileNameS = std::string(theFileName);
	if(theFileNameS.size() >= 3 &&theFileNameS[0] == '.' && theFileNameS[1] == '/')
		theFileNameS = theFileNameS.substr(2, theFileNameS.size() -2);
	std::string filename;

	if (mPrefix.empty())
		filename = std::string(theFileName);
	else
		filename = mPrefix + std::string(PATH_SEP) +
			std::string(theFileName);

	AutoCrit autoCrit(mCritSect);
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
		return new ZipFile(this, zzipFile, zstat.st_size);
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

static int pak_open(zzip_char_t* name, int flags, ...)
{
    File* file = FileSystemManager::getManager()->open(name, "rb");
    if (!file)
	    return -1;
    return file->getId();
}

static int pak_close(int fd)
{
    File* file = FileSystemManager::getManager()->getFile(fd);
    if (file)
	file->close();
    return 0;
}

static zzip_ssize_t pak_read(int fd, void* buf, zzip_size_t len)
{
    File* file = FileSystemManager::getManager()->getFile(fd);
    if (!file)
	return -1;
    return file->read(buf, 1, len);
}

static zzip_off_t pak_seeks(int fd, zzip_off_t offset, int whence)
{
    File* file = FileSystemManager::getManager()->getFile(fd);
    if (!file)
	return -1;

    int ret = file->seek(offset, whence);
    if (ret == 0)
	return file->tell();
    return ret;
}

static zzip_off_t pak_filesize(int fd)
{
    File* file = FileSystemManager::getManager()->getFile(fd);
    if (!file)
	return -1;
    return file->getSize();
}

static zzip_plugin_io_handlers pak_plugin_io = {
    { pak_open, pak_close, pak_read, pak_seeks, pak_filesize }
};

FileSystem* ZipFileSystemDriver::Create(const std::string &location,
				 const std::string &type,
				 int                priority)
{
	if (type != "zip" || location.empty())
		return false;

	std::string path;
	std::string prefix;
	size_t pos = location.find("::");

	if (pos != std::string::npos)
	{
	    path = location.substr(0, pos);
	    prefix = location.substr(pos + 2, location.length() - pos - 2);
	}
	else
	{
	    path = location;
	}

	AutoCrit autoCrit(mCritSect);
	zzip_error_t zzipError;
	ZZIP_DIR *dir = zzip_dir_open(path.c_str(), &zzipError);
	if (!dir)
	{
		mCritSect.Leave();
		dir = zzip_opendir_ext_io(path.c_str(), ZZIP_ONLYZIP | ZZIP_CASELESS,
					  0, &pak_plugin_io);
		mCritSect.Enter();
		if (!dir)
			return false;
	}

	return new ZipFileSystem(this, path, prefix, priority, dir);
}
