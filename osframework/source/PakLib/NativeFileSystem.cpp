#include "NativeFileSystem.h"
#include "Find.h"

#ifdef WIN32
#include <io.h>

#define access _access
#ifndef R_OK
#define R_OK 4
#endif
#endif

using namespace PakLib;

class NativeFile: public File
{
public:
	NativeFile(FILE *fp) : mFp(fp)
	{
		size_t pos = ftell(fp);
		fseek(mFp, 0, SEEK_END);
		mSize = ftell(fp);
		fseek(mFp, pos, SEEK_SET);
	}

	~NativeFile()
	{
		if (mFp)
			fclose(mFp);
	}

	int seek(long theOffset, int theOrigin)
	{
		return fseek(mFp, theOffset, theOrigin);
	}

	long tell()
	{
		return ftell(mFp);
	}

	size_t read(void* thePtr, int theElemSize, int theCount)
	{
		return fread(thePtr, theElemSize, theCount, mFp);
	}

	size_t write(const void* thePtr, int theElemSize, int theCount)
	{
		return fwrite(thePtr, theElemSize, theCount, mFp);
	}

	int getc()
	{
		return fgetc(mFp);
	}

	int ungetc(int theChar)
	{
		return ::ungetc(theChar, mFp);
	}

	char* gets(char* thePtr, int theSize)
	{
		return fgets(thePtr, theSize, mFp);
	}

	wchar_t* gets(wchar_t* thePtr, int theSize)
	{
		return fgetws(thePtr, theSize, mFp);
	}

	int eof()
	{
		return feof(mFp);
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
	FILE *mFp;
	size_t mSize;
};

NativeFileSystem::NativeFileSystem(FileSystemDriver  *driver,
				   const std::string &location,
				   int                priority)
	: FileSystem(driver, location, priority)
{
}

NativeFileSystem::~NativeFileSystem()
{
}

static FILE* _fopencase(const char *path, const char * mode)
{
	if (!path || !mode)
		return 0;

	FILE* fp = fopen(path, mode);
	if (fp)
		return fp;

#ifndef WIN32
	struct _finddata_t finddata;
	intptr_t handle = _findfirst(path, &finddata);
	if (handle == (intptr_t)-1)
		return 0;

	std::string aPath(path);
	if (aPath.rfind('/') >= 0)
		aPath = aPath.substr(0, aPath.rfind('/')) + '/' + finddata.name;
	if (strcmp(aPath.c_str(), path) && getenv("PAKLIB_DEBUG_REDIR"))
		printf ("PakLib: fopencase: '%s' to '%s'\n", path, finddata.name);
	fp = fopen(aPath.c_str(), mode);
	_findclose(handle);
#endif
	return fp;
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

#ifdef WIN32
static wchar_t * p_mbstowcs(const char * theString)
{
        wchar_t * aString;
        size_t length = mbstowcs(NULL, theString, 0);

        aString = new wchar_t[length + 1];
        mbstowcs(aString, theString, length + 1);

        return aString;
}
#endif

File* NativeFileSystem::open(const char* theFileName,
			     const char* theAccess)
{
	FILE *fp = _fopencase(theFileName, theAccess);
	if (fp)
		return new NativeFile(fp);

	if (theFileName[0] == '/' || theFileName[1] == '\\')
		return false;

#ifdef WIN32
	if (strlen(theFileName) >= 2 && isalpha(theFileName[0]) &&
	    theFileName[1] == ':')
		return false;
#endif

	std::string path = mLocation + "/" + std::string(theFileName);
	fp = _fopencase(path.c_str(), theAccess);
	if (fp)
		return new NativeFile(fp);

	return 0;
}

File* NativeFileSystem::open(const wchar_t* theFileName,
			     const wchar_t* theAccess)
{
	File *result = 0;

#ifndef WIN32
        char * aFileName = p_wcstombs(theFileName);
        char * aAccess = p_wcstombs(theAccess);

        result = open(aFileName, aAccess);

        delete [] aFileName;
        delete [] aAccess;
	return result;
#else
	FILE *fp = _wfopen(theFileName, theAccess);
	if (fp)
		return new NativeFile(fp);

	if (theFileName[0] == '/' || theFileName[1] == '\\')
		return 0;

#ifdef WIN32
	if (wcslen(theFileName) >= 2 && isalpha(theFileName[0]) &&
	    theFileName[1] == ':')
		return 0;
#endif

	wchar_t *location = p_mbstowcs(mLocation.c_str());
	if (!location)
		return 0;

	std::wstring path = std::wstring(location) + L"/" + std::wstring(theFileName);
	delete [] location;

	fp = _wfopen(path.c_str(), theAccess);
	if (fp)
		return new NativeFile(fp);
#endif

	return result;
}

Dir* NativeFileSystem::openDir(const char *thePath)
{
	return 0;
}

NativeFileSystemDriver::NativeFileSystemDriver(int priority)
	: FileSystemDriver("native", priority)
{
}

FileSystem* NativeFileSystemDriver::Create(const std::string &location,
					   const std::string &type,
					   int                priority)
{
	if (type != "native" || location.empty())
		return 0;

	if (access(location.c_str(), R_OK) != 0)
		return 0;

	return new NativeFileSystem(this, location, priority);
}
