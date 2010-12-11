#include "AndroidFileSystem.h"
#include "Find.h"
#include "AutoCrit.h"

#include <GameFile.h>

using namespace PakLib;

class AndroidFile: public File
{
public:
	AndroidFile(awFile *fp, size_t size) : mFp(fp), mSize(size)
	{
	}

	~AndroidFile()
	{
		if (mFp)
			awFileClose(mFp);
	}

	int seek(long theOffset, int theOrigin)
	{
		return awFileSeek(mFp, theOffset, theOrigin);
	}

	int tell()
	{
		return awFileTell(mFp);
	}

	size_t read(void* thePtr, int theElemSize, int theCount)
	{
		return awFileRead(mFp, thePtr, theElemSize, theCount);
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
	awFile *mFp;
	size_t  mSize;
};

AndroidFileSystem::AndroidFileSystem(FileSystemDriver          * driver,
				     const std::string         & location,
				     int                         priority)
	: FileSystem(driver, location, priority)
{
}

AndroidFileSystem::~AndroidFileSystem()
{
	AndroidFileSystemDriver* driver = (AndroidFileSystemDriver*)mDriver;
	AutoCrit autoCrit(driver->mCritSect);
}

File* AndroidFileSystem::open(const char* theFileName,
			      const char* theAccess)
{
	if (!strcmp(theAccess, "w") || !strcmp(theAccess, "a") ||
	    !strcmp(theAccess, "a+"))
		return 0;

	if (theFileName[0] == '/' || theFileName[1] == '\\')
		return 0;

	awFile *fp = awFileOpen(theFileName);
	if (fp)
	{
		size_t size;

		awFileSeek(fp, 0, SEEK_END);
		size = awFileTell(fp);

		if (!strcmp(theAccess, "a") || !strcmp(theAccess, "a+"))
			awFileSeek(fp, 0, SEEK_END);
		else
			awFileSeek(fp, 0, SEEK_SET);

		return new AndroidFile(fp, size);
	}

	return 0;
}

File* AndroidFileSystem::open(const wchar_t* theFileName,
			      const wchar_t* theAccess)
{
	return 0;
}

Dir* AndroidFileSystem::openDir(const char *thePath)
{
	return 0;
}

AndroidFileSystemDriver::AndroidFileSystemDriver(int priority)
	: FileSystemDriver("android", priority), mAdded(false)
{
}

FileSystem* AndroidFileSystemDriver::Create(const std::string &location,
					    const std::string &type,
					    int                priority)
{
	if (type != "android" || mAdded)
		return false;

	AutoCrit autoCrit(mCritSect);
	mAdded = true;
	return new AndroidFileSystem(this, location, priority);
}
