#ifndef __PAKINTERFACE_H__
#define __PAKINTERFACE_H__

#include <map>
#include <list>
#include <string>
#include <cstdlib>
#include <cstdio>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdint.h>
#endif

#ifdef WIN32
typedef FILETIME PakFileTime;
typedef HANDLE PakHandle;
typedef LPCTSTR PakFileNamePtr;
typedef LPWIN32_FIND_DATA PakFindDataPtr;
#else
typedef uint64_t PakFileTime;
typedef void * PakHandle;
typedef char * PakFileNamePtr;
#define FILE_ATTRIBUTE_DIRECTORY 0x01
typedef struct {
    unsigned dwFileAttributes;
    unsigned long nFileSizeLow;
    PakFileTime ftCreationTime;
    PakFileTime ftLastWriteTime;
    PakFileTime ftLastAccessTime;
    char cFileName[1024];
} PakFindData, * PakFindDataPtr;
#endif

struct PFILE;

bool                    p_addResource(const char *location,
				      const char *type,
				      int         priority = 0);

PFILE*                  p_fopen(const char* theFileName, const char* theAccess);
PFILE*                  p_fopen(const wchar_t* theFileName, const wchar_t* theAccess);
int                     p_fclose(PFILE* theFile);
int                     p_fseek(PFILE* theFile, long theOffset, int theOrigin);
int                     p_ftell(PFILE* theFile);
size_t                  p_fread(void* thePtr, int theSize, int theCount, PFILE* theFile);
size_t                  p_fwrite(const void* thePtr, int theSize, int theCount, PFILE* theFile);
int                     p_fgetc(PFILE* theFile);
int                     p_ungetc(int theChar, PFILE* theFile);
char*                   p_fgets(char* thePtr, int theSize, PFILE* theFile);
wchar_t*                p_fgets(wchar_t* thePtr, int theSize, PFILE* theFile);
int                     p_feof(PFILE* theFile);
PakHandle               p_FindFirstFile(PakFileNamePtr lpFileName, PakFindDataPtr lpFindFileData);
bool                    p_FindNextFile(PakHandle hFindFile, PakFindDataPtr lpFindFileData);
bool                    p_FindClose(PakHandle hFindFile);

#endif //__PAKINTERFACE_H__
