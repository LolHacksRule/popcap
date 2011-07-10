#ifndef __SEXYAPPFRAMEWORK_COMMON_H__
#define __SEXYAPPFRAMEWORK_COMMON_H__

#ifdef _MSC_VER
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#undef _WIN32_WINNT
#undef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef _UNICODE
#undef UNICODE

#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <wctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#include <mmsystem.h>
#define snprintf _snprintf
#define wcsncmp wcsicmp
#define wcsncasecmp wcsnicmp
#else
#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef unsigned char  BYTE;
typedef unsigned int DWORD;
typedef unsigned int UINT;
typedef void * HWND;
#ifdef __APPLE__
typedef signed char BOOL;
#else
typedef int BOOL;
#endif
#define TRUE 1
#define FALSE 0
#define _stricmp(x, y) strcasecmp((x), (y))
#define stricmp(x, y) strcasecmp((x), (y))
#define ZeroMemory(p, s) memset((p), 0, (s))
#define Sleep(s) usleep(s)
#endif
#include "ModVal.h"
#include "SexyString.h"

typedef std::string			SexyString;
#define _S(x)				x


#define sexystrncmp			strncmp
#define sexystrcmp			strcmp
#define sexystricmp			stricmp
#define sexysscanf			sscanf
#define sexyatoi			atoi
#define sexystrcpy			strcpy

#define sexystrlen			strlen
#define sexyatof			atof
#define sexysatol			atol
#define sexystrtoul			strtoul
#define sexysscanf			sscanf
#define sexyisspace			isspace
#define sexyisdigit			isdigit
#define sexystrtok			strtok
#define sexystrchr			strchr
#define sexysprintf			sprintf
#define sexytolower			tolower
#define sexyfstream			std::fstream
#define sexygetcwd			getcwd
#define sexychdir			chdir
#define sexyfgets			fgets
#define sexystrtok			strtok
#define sexyitoa			itoa
#define sexytoupper			toupper

#define SexyStringToStringFast(x)	(x)
#define SexyStringToWStringFast(x)	StringToWString(x)
#define StringToSexyStringFast(x)	(x)
#define WStringToSexyStringFast(x)	WStringToString(x)

#define LONG_BIGE_TO_NATIVE(l) (((l >> 24) & 0xFF) | ((l >> 8) & 0xFF00) | ((l << 8) & 0xFF0000) | ((l << 24) & 0xFF000000))
#define WORD_BIGE_TO_NATIVE(w) (((w >> 8) & 0xFF) | ((w << 8) & 0xFF00))
#define LONG_LITTLEE_TO_NATIVE(l) (l)
#define WORD_LITTLEE_TO_NATIVE(w) (w)

#define LENGTH(anyarray) (sizeof(anyarray) / sizeof(anyarray[0]))

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned int uint32;
#ifdef _MSC_VER
typedef __int64 int64;
#else
typedef int64_t int64;
#endif

typedef std::map<std::string, std::string>		DefinesMap;
typedef std::map<Sexy::WString, Sexy::WString>	WStringWStringMap;
typedef SexyString::value_type					SexyChar;
#define HAS_SEXYCHAR

#if defined(WIN32) && !defined(BUILDING_STATIC_SEXYFRAMEWORK)
#ifdef BUILDING_SEXYFRAMEWORK
#define __declspec (dllexport)
#else
#define __declspec (dllimport)
#endif
#endif

#ifndef SEXY_EXPORT
#define SEXY_EXPORT
#endif

namespace Sexy
{

	const ulong SEXY_RAND_MAX = 0x7FFFFFFF;

	extern bool			gDebug;
#ifdef WIN32
	extern HINSTANCE		gHInstance;
#endif

	int				Rand();
	int				Rand(int range);
	float				Rand(float range);
	void				SRand(ulong theSeed);
	std::string			vformat(const char* fmt, va_list argPtr);
	std::string			StrFormat(const char* fmt ...);
	bool				CheckFor98Mill();
	bool				CheckForVista();
	std::string			GetResourcesFolder();
	void				SetResourcesFolder(const std::string& thePath);
	std::string			GetAppDataFolder();
	void				SetAppDataFolder(const std::string& thePath);
	std::string			URLEncode(const std::string& theString);
	std::string			StringToUpper(const std::string& theString);
	Sexy::WString			StringToUpper(const Sexy::WString& theString);
	std::string			StringToLower(const std::string& theString);
	Sexy::WString			StringToLower(const Sexy::WString& theString);
	std::string			WStringToString(const Sexy::WString &theString);
	SexyString			StringToSexyString(const std::string& theString);
	std::string			SexyStringToString(const SexyString& theString);
	std::string			Upper(const std::string& theData);
	Sexy::WString			Upper(const Sexy::WString& theData);
	std::string			Lower(const std::string& theData);
	Sexy::WString			Lower(const Sexy::WString& theData);
	std::string			Trim(const std::string& theString);
	Sexy::WString			Trim(const Sexy::WString& theString);
	bool				StringToInt(const std::string theString, int* theIntVal);
	bool				StringToDouble(const std::string theString, double* theDoubleVal);
	bool				StringToInt(const Sexy::WString theString, int* theIntVal);
	bool				StringToDouble(const Sexy::WString theString, double* theDoubleVal);
	int				StrFindNoCase(const char *theStr, const char *theFind);
	bool				StrPrefixNoCase(const char *theStr, const char *thePrefix,
							int maxLength = 10000000);
	SexyString			CommaSeperate(int theValue);
	std::string			Evaluate(const std::string& theString,
						 const DefinesMap& theDefinesMap);
	std::string			XMLDecodeString(const std::string& theString);
	std::string			XMLEncodeString(const std::string& theString);
	Sexy::WString			XMLDecodeString(const Sexy::WString& theString);
	Sexy::WString			XMLEncodeString(const Sexy::WString& theString);

	bool				Deltree(const std::string& thePath);
	bool				FileExists(const std::string& theFileName);
	void				MkDir(const std::string& theDir);
	std::string			GetFileName(const std::string& thePath, bool noExtension = false);
	std::string			GetFileDir(const std::string& thePath, bool withSlash = false);
	std::string			RemoveTrailingSlash(const std::string& theDirectory);
	std::string			AddTrailingSlash(const std::string& theDirectory,
							 bool backSlash = false);
	time_t				GetFileDate(const std::string& theFileName);
	std::string			GetCurDir();
	std::string			GetFullPath(const std::string& theRelPath);
	std::string			GetPathFrom(const std::string& theRelPath,
						    const std::string& theDir);
	bool				AllowAllAccess(const std::string& theFileName);

	bool                            GetEnvOption(const char *option,  bool value = false);
	int                             GetEnvIntOption(const char *option, int value = 0);

	inline void			inlineUpper(std::string &theData)
	{
		//std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

		int aStrLen = (int) theData.length();
		for (int i = 0; i < aStrLen; i++)
		{
			theData[i] = toupper(theData[i]);
		}
	}

	inline void			inlineUpper(Sexy::WString &theData)
	{
		//std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

		int aStrLen = (int) theData.length();
		for (int i = 0; i < aStrLen; i++)
		{
			theData[i] = toupper(theData[i]);
		}
	}

	inline void			inlineLower(std::string &theData)
	{
		std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
	}

	inline void			inlineLower(Sexy::WString &theData)
	{
		std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
	}

	inline void			inlineLTrim(std::string &theData,
						    const std::string& theChars = " \t\r\n")
	{
		theData.erase(0, theData.find_first_not_of(theChars));
	}

	inline void			inlineLTrim(Sexy::WString &theData,
						    const Sexy::WString& theChars = WSTR(" \t\r\n"))
	{
		theData.erase(0, theData.find_first_not_of(theChars));
	}

	inline void			inlineRTrim(std::string &theData,
						    const std::string& theChars = " \t\r\n")
	{
		theData.resize(theData.find_last_not_of(theChars) + 1);
	}

	inline void			inlineRTrim(Sexy::WString &theData,
						    const Sexy::WString& theChars = WSTR(" \t\r\n"))
	{
		theData.resize(theData.find_last_not_of(theChars) + 1);
	}

	inline void			inlineTrim(std::string &theData,
						   const std::string& theChars = " \t\r\n")
	{
		inlineRTrim(theData, theChars);
		inlineLTrim(theData, theChars);
	}

	inline void			inlineTrim(Sexy::WString &theData,
						   const Sexy::WString& theChars = WSTR(" \t\r\n"))
	{
		inlineRTrim(theData, theChars);
		inlineLTrim(theData, theChars);
	}

	struct StringLessNoCase
	{
		bool operator()(const std::string &s1, const std::string &s2) const
		{
			return _stricmp(s1.c_str(),s2.c_str())<0;
		}
	};

}

static inline int RoundToPOT (int i)
{
	int v = 1;

	while (v < i)
		v <<= 1;

	return v;
}

static inline bool IsPOT (int i)
{
	return i && !(i & (i - 1));
}

#endif //__SEXYAPPFRAMEWORK_COMMON_H__
