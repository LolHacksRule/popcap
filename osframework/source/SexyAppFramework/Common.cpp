#include "Common.h"
#include "MTRand.h"
#include "Debug.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#ifdef WIN32
#include <aclapi.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "PerfTimer.h"

#ifdef WIN32
HINSTANCE Sexy::gHInstance;
#endif
bool Sexy::gDebug = false;
static Sexy::MTRand gMTRand;
namespace Sexy
{
	std::string gAppDataFolder = "";
}

int Sexy::Rand()
{
	return gMTRand.Next();
}

int Sexy::Rand(int range)
{
	return gMTRand.Next((unsigned long)range);
}

float Sexy::Rand(float range)
{
	return gMTRand.Next(range);
}

void Sexy::SRand(ulong theSeed)
{
	gMTRand.SRand(theSeed);
}

bool Sexy::CheckFor98Mill()
{
	static bool needOsCheck = true;
	static bool is98Mill = false;

#ifdef WIN32
	if (needOsCheck)
	{
		bool invalid = false;
		OSVERSIONINFOEXA osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
			if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
				return false;
		}

		needOsCheck = false;
		is98Mill = osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS; // let's check Win95, 98, *AND* ME.
	}
#endif
	return is98Mill;
}

bool Sexy::CheckForVista()
{
	static bool needOsCheck = true;
	static bool isVista = false;

#ifdef WIN32
	if (needOsCheck)
	{
		bool invalid = false;
		OSVERSIONINFOEXA osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
			if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
				return false;
		}

		needOsCheck = false;
		isVista = osvi.dwMajorVersion >= 6;
	}
#endif
	return isVista;
}

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#ifndef SEXY_IOS
#include <CoreServices/CoreServices.h>
#endif
#endif

static std::string gResourceFolder = "";

std::string Sexy::GetResourcesFolder()
{
#ifdef __APPLE__
	if (gResourceFolder.length () == 0)
	{
		CFBundleRef mainBoundle = CFBundleGetMainBundle();
		if (mainBoundle)
		{
			CFURLRef appURL = CFBundleCopyBundleURL(mainBoundle);
			CFStringRef appPath = CFURLCopyFileSystemPath(appURL,
								      kCFURLPOSIXPathStyle);
			const char * cAppPath = CFStringGetCStringPtr(appPath,
								      CFStringGetFastestEncoding(appPath));

			CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(mainBoundle);
			CFStringRef resourcePath = CFURLCopyFileSystemPath(resourceURL,
									   kCFURLPOSIXPathStyle);
			const char * cResourcePath = CFStringGetCStringPtr(resourcePath,
									   CFStringGetFastestEncoding(resourcePath));
			if (cResourcePath)
			{
				std::string path = std::string(cAppPath) + "/" + std::string(cResourcePath) + "/";;
				gResourceFolder = path;
			}


			CFRelease(resourcePath);
			CFRelease(resourceURL);

			CFRelease(appPath);
			CFRelease(appURL);
		}
	}
#endif

	return gResourceFolder;
}

void Sexy::SetResourcesFolder(const std::string & theFolder)
{
	gResourceFolder = theFolder;
}

std::string Sexy::GetAppDataFolder()
{
#ifdef __APPLE__
#ifdef SEXY_IOS
	if (!Sexy::gAppDataFolder.length())
	{
		CFBundleRef mainBundle = CFBundleGetMainBundle();
		if (mainBundle)
		{
			CFURLRef appURL = CFBundleCopyBundleURL(mainBundle);
			CFStringRef appPath = CFURLCopyFileSystemPath(appURL,
								      kCFURLPOSIXPathStyle);
			const char * cAppPath = CFStringGetCStringPtr(appPath,
								      CFStringGetFastestEncoding(appPath));

			std::string path = std::string(cAppPath) + "/../Library/";

			CFStringRef identifier = CFBundleGetIdentifier(mainBundle);
			if (identifier)
			{
				const char * appId = CFStringGetCStringPtr(identifier,
									   CFStringGetFastestEncoding(identifier));
				path += std::string(appId)  + "/";

				CFRelease(identifier);
			}

			Sexy::gAppDataFolder = path;

			CFRelease(appPath);
			CFRelease(appURL);
		}
	}
#else
	if (!Sexy::gAppDataFolder.length())
	{
		OSErr   error;
		FSRef   fsRef;
		char    path[1024];

		error = FSFindFolder(kOnAppropriateDisk, kPreferencesFolderType, kDontCreateFolder, &fsRef);
		error = FSRefMakePath(&fsRef, (UInt8*)path, 1024);

		CFBundleRef mainBundle = CFBundleGetMainBundle();
		CFStringRef identifier = CFBundleGetIdentifier(mainBundle);
		if (identifier)
		{
			const char * appId = CFStringGetCStringPtr(identifier,
								   CFStringGetFastestEncoding(identifier));
			if (appId)
				Sexy::gAppDataFolder += std::string(path) + "/" + std::string(appId);
		}
	}
#endif
#endif
	return Sexy::gAppDataFolder;
}

void Sexy::SetAppDataFolder(const std::string& thePath)
{
	if (CheckForVista())
	{
		std::string aPath = thePath;
		if (!aPath.empty())
		{
			if (aPath[aPath.length()-1] != '\\' && aPath[aPath.length()-1] != '/')
				aPath += '\\';
		}

		Sexy::gAppDataFolder = aPath;
	}
}


std::string Sexy::URLEncode(const std::string& theString)
{
	char* aHexChars = "0123456789ABCDEF";

	std::string aString;

	for (unsigned i = 0; i < theString.length(); i++)
	{
		switch (theString[i])
		{
		case ' ':
			aString.insert(aString.end(), '+');
			break;
		case '?':
		case '&':
		case '%':
		case '+':
		case '\r':
		case '\n':
		case '\t':
			aString.insert(aString.end(), '%');
			aString.insert(aString.end(), aHexChars[(theString[i] >> 4) & 0xF]);
			aString.insert(aString.end(), aHexChars[(theString[i]     ) & 0xF]);
			break;
		default:
			aString.insert(aString.end(), theString[i]);
		}
	}

	return aString;
}

std::string Sexy::StringToUpper(const std::string& theString)
{
	std::string aString;

	for (size_t i = 0; i < theString.length(); i++)
		aString += toupper(theString[i]);

	return aString;
}

Sexy::WString Sexy::StringToUpper(const Sexy::WString& theString)
{
	Sexy::WString aString;

	for (size_t i = 0; i < theString.length(); i++)
		aString += towupper(theString[i]);

	return aString;
}

std::string Sexy::StringToLower(const std::string& theString)
{
	std::string aString;

	for (size_t i = 0; i < theString.length(); i++)
		aString += tolower(theString[i]);

	return aString;
}

Sexy::WString Sexy::StringToLower(const Sexy::WString& theString)
{
	Sexy::WString aString;

	for (size_t i = 0; i < theString.length(); ++i)
		aString += tolower(theString[i]);

	return aString;
}

SexyString Sexy::StringToSexyString(const std::string& theString)
{
#ifdef _USE_WIDE_STRING
	return StringToWString(theString);
#else
	return SexyString(theString);
#endif
}

std::string Sexy::SexyStringToString(const SexyString& theString)
{
#ifdef _USE_WIDE_STRING
	return WStringToString(theString);
#else
	return std::string(theString);
#endif
}

std::string Sexy::Trim(const std::string& theString)
{
	int aStartPos = 0;
	while ( aStartPos < (int) theString.length() && isspace(theString[aStartPos]) )
		aStartPos++;

	int anEndPos = theString.length() - 1;
	while ( anEndPos >= 0 && isspace(theString[anEndPos]) )
		anEndPos--;

	return theString.substr(aStartPos, anEndPos - aStartPos + 1);
}

Sexy::WString Sexy::Trim(const Sexy::WString& theString)
{
	int aStartPos = 0;
	while ( aStartPos < (int) theString.length() && isspace(theString[aStartPos]) )
		aStartPos++;

	int anEndPos = theString.length() - 1;
	while ( anEndPos >= 0 && isspace(theString[anEndPos]) )
		anEndPos--;

	return theString.substr(aStartPos, anEndPos - aStartPos + 1);
}

bool Sexy::StringToInt(const std::string theString, int* theIntVal)
{
	*theIntVal = 0;

	if (theString.length() == 0)
		return false;

	int theRadix = 10;
	bool isNeg = false;

	unsigned i = 0;
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (; i < theString.length(); i++)
	{
		char aChar = theString[i];

		if ((theRadix == 10) && (aChar >= '0') && (aChar <= '9'))
			*theIntVal = (*theIntVal * 10) + (aChar - '0');
		else if ((theRadix == 0x10) &&
			(((aChar >= '0') && (aChar <= '9')) ||
			 ((aChar >= 'A') && (aChar <= 'F')) ||
			 ((aChar >= 'a') && (aChar <= 'f'))))
		{
			if (aChar <= '9')
				*theIntVal = (*theIntVal * 0x10) + (aChar - '0');
			else if (aChar <= 'F')
				*theIntVal = (*theIntVal * 0x10) + (aChar - 'A') + 0x0A;
			else
				*theIntVal = (*theIntVal * 0x10) + (aChar - 'a') + 0x0A;
		}
		else if (((aChar == 'x') || (aChar == 'X')) && (i == 1) && (*theIntVal == 0))
		{
			theRadix = 0x10;
		}
		else
		{
			*theIntVal = 0;
			return false;
		}
	}

	if (isNeg)
		*theIntVal = -*theIntVal;

	return true;
}

bool Sexy::StringToInt(const Sexy::WString theString, int* theIntVal)
{
	*theIntVal = 0;

	if (theString.length() == 0)
		return false;

	int theRadix = 10;
	bool isNeg = false;

	unsigned i = 0;
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (; i < theString.length(); i++)
	{
		int aChar = theString[i];

		if ((theRadix == 10) && (aChar >= '0') && (aChar <= '9'))
			*theIntVal = (*theIntVal * 10) + (aChar - L'0');
		else if ((theRadix == 0x10) &&
			(((aChar >= '0') && (aChar <= '9')) ||
			 ((aChar >= 'A') && (aChar <= 'F')) ||
			 ((aChar >= 'a') && (aChar <= 'f'))))
		{
			if (aChar <= '9')
				*theIntVal = (*theIntVal * 0x10) + (aChar - '0');
			else if (aChar <= 'F')
				*theIntVal = (*theIntVal * 0x10) + (aChar - 'A') + 0x0A;
			else
				*theIntVal = (*theIntVal * 0x10) + (aChar - 'a') + 0x0A;
		}
		else if (((aChar == 'x') || (aChar == 'X')) && (i == 1) && (*theIntVal == 0))
		{
			theRadix = 0x10;
		}
		else
		{
			*theIntVal = 0;
			return false;
		}
	}

	if (isNeg)
		*theIntVal = -*theIntVal;

	return true;
}

bool Sexy::StringToDouble(const std::string theString, double* theDoubleVal)
{
	*theDoubleVal = 0.0;

	if (theString.length() == 0)
		return false;

	bool isNeg = false;

	unsigned i = 0;
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (; i < theString.length(); i++)
	{
		char aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
			*theDoubleVal = (*theDoubleVal * 10) + (aChar - '0');
		else if (aChar == '.')
		{
			i++;
			break;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}
	}

	double aMult = 0.1;
	for (; i < theString.length(); i++)
	{
		char aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
		{
			*theDoubleVal += (aChar - '0') * aMult;
			aMult /= 10.0;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}
	}

	if (isNeg)
		*theDoubleVal = -*theDoubleVal;

	return true;
}

bool Sexy::StringToDouble(const Sexy::WString theString, double* theDoubleVal)
{
	*theDoubleVal = 0.0;

	if (theString.length() == 0)
		return false;

	bool isNeg = false;

	unsigned i = 0;
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (; i < theString.length(); i++)
	{
		int aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
			*theDoubleVal = (*theDoubleVal * 10) + (aChar - '0');
		else if (aChar == L'.')
		{
			i++;
			break;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}
	}

	double aMult = 0.1;
	for (; i < theString.length(); i++)
	{
		int aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
		{
			*theDoubleVal += (aChar - '0') * aMult;
			aMult /= 10.0;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}
	}

	if (isNeg)
		*theDoubleVal = -*theDoubleVal;

	return true;
}

// TODO: Use <locale> for localization of number output?
SexyString Sexy::CommaSeperate(int theValue)
{
	if (theValue == 0)
		return "0";

	SexyString aCurString;

	int aPlace = 0;
	int aCurValue = theValue;

	while (aCurValue > 0)
	{
		if ((aPlace != 0) && (aPlace % 3 == 0))
			aCurString = ',' + aCurString;
		aCurString = (char)('0' + (aCurValue % 10)) + aCurString;
		aCurValue /= 10;
		aPlace++;
	}

	return aCurString;
}

std::string Sexy::GetCurDir()
{
	char aDir[1024];
#ifdef WIN32
	return _getcwd(aDir, sizeof(aDir));
#else
	return getcwd(aDir, sizeof(aDir));
#endif
}

std::string Sexy::GetFullPath(const std::string& theRelPath)
{
	return GetPathFrom(theRelPath, GetCurDir());
}

std::string Sexy::GetPathFrom(const std::string& theRelPath, const std::string& theDir)
{
	std::string aDriveString;
	std::string aNewPath = theDir;

	if ((theRelPath.length() >= 2) && (theRelPath[1] == ':'))
		return theRelPath;

	char aSlashChar = '/';

	if ((theRelPath.find('\\') != -1) || (theDir.find('\\') != -1))
		aSlashChar = '\\';

	if ((aNewPath.length() >= 2) && (aNewPath[1] == ':'))
	{
		aDriveString = aNewPath.substr(0, 2);
		aNewPath.erase(aNewPath.begin(), aNewPath.begin()+2);
	}

	// Append a trailing slash if necessary
	if ((aNewPath.length() > 0) && (aNewPath[aNewPath.length()-1] != '\\') && (aNewPath[aNewPath.length()-1] != '/'))
		aNewPath += aSlashChar;

	std::string aTempRelPath = theRelPath;

	for (;;)
	{
		if (aNewPath.length() == 0)
			break;

		int aFirstSlash = aTempRelPath.find('\\');
		int aFirstForwardSlash = aTempRelPath.find('/');

		if ((aFirstSlash == -1) || ((aFirstForwardSlash != -1) && (aFirstForwardSlash < aFirstSlash)))
			aFirstSlash = aFirstForwardSlash;

		if (aFirstSlash == -1)
			break;

		std::string aChDir = aTempRelPath.substr(0, aFirstSlash);

		aTempRelPath.erase(aTempRelPath.begin(), aTempRelPath.begin() + aFirstSlash + 1);

		if (aChDir.compare("..") == 0)
		{
			int aLastDirStart = aNewPath.length() - 1;
			while ((aLastDirStart > 0) && (aNewPath[aLastDirStart-1] != '\\') && (aNewPath[aLastDirStart-1] != '/'))
				aLastDirStart--;

			std::string aLastDir = aNewPath.substr(aLastDirStart, aNewPath.length() - aLastDirStart - 1);
			if (aLastDir.compare("..") == 0)
			{
				aNewPath += "..";
				aNewPath += aSlashChar;
			}
			else
			{
				aNewPath.erase(aNewPath.begin() + aLastDirStart, aNewPath.end());
			}
		}
		else if (aChDir.compare("") == 0)
		{
			aNewPath = aSlashChar;
			break;
		}
		else if (aChDir.compare(".") != 0)
		{
			aNewPath += aChDir + aSlashChar;
			break;
		}
	}

	aNewPath = aDriveString + aNewPath + aTempRelPath;

	if (aSlashChar == '/')
	{
		for (;;)
		{
			int aSlashPos = aNewPath.find('\\');
			if (aSlashPos == -1)
				break;
			aNewPath[aSlashPos] = '/';
		}
	}
	else
	{
		for (;;)
		{
			int aSlashPos = aNewPath.find('/');
			if (aSlashPos == -1)
				break;
			aNewPath[aSlashPos] = '\\';
		}
	}

	return aNewPath;
}

bool Sexy::AllowAllAccess(const std::string& theFileName)
{
#ifdef WIN32
	HMODULE aLib = LoadLibraryA("advapi32.dll");
	if (aLib == NULL)
		return false;

	BOOL (WINAPI *fnSetFileSecurity)(LPCTSTR lpFileName, SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor);
	BOOL (WINAPI *fnSetSecurityDescriptorDacl)(PSECURITY_DESCRIPTOR pSecurityDescriptor, BOOL bDaclPresent, PACL pDacl, BOOL bDaclDefaulted);
	BOOL (WINAPI *fnInitializeSecurityDescriptor)(PSECURITY_DESCRIPTOR pSecurityDescriptor, DWORD dwRevision);
	BOOL (WINAPI *fnAllocateAndInitializeSid)(
	  PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
	  BYTE nSubAuthorityCount,
	  DWORD dwSubAuthority0,
	  DWORD dwSubAuthority1,
	  DWORD dwSubAuthority2,
	  DWORD dwSubAuthority3,
	  DWORD dwSubAuthority4,
	  DWORD dwSubAuthority5,
	  DWORD dwSubAuthority6,
	  DWORD dwSubAuthority7,
	  PSID* pSid
	);
	DWORD (WINAPI *fnSetEntriesInAcl)(ULONG cCountOfExplicitEntries, PEXPLICIT_ACCESS pListOfExplicitEntries, PACL OldAcl, PACL* NewAcl);
	PVOID (WINAPI *fnFreeSid)(PSID pSid);

	*(void**)&fnSetFileSecurity = (void*)GetProcAddress(aLib, "SetFileSecurityA");
	*(void**)&fnSetSecurityDescriptorDacl = (void*)GetProcAddress(aLib, "SetSecurityDescriptorDacl");
	*(void**)&fnInitializeSecurityDescriptor = (void*)GetProcAddress(aLib, "InitializeSecurityDescriptor");
	*(void**)&fnAllocateAndInitializeSid = (void*)GetProcAddress(aLib, "AllocateAndInitializeSid");
	*(void**)&fnSetEntriesInAcl = (void*)GetProcAddress(aLib, "SetEntriesInAclA");
	*(void**)&fnFreeSid = (void*) GetProcAddress(aLib, "FreeSid");

	if (!(fnSetFileSecurity && fnSetSecurityDescriptorDacl && fnInitializeSecurityDescriptor && fnAllocateAndInitializeSid && fnSetEntriesInAcl && fnFreeSid))
	{
		FreeLibrary(aLib);
		return false;
	}


	PSID pEveryoneSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	bool result = false;

    // Create a well-known SID for the Everyone group.
    if (fnAllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID))
    {
		EXPLICIT_ACCESS ea;

		// Initialize an EXPLICIT_ACCESS structure for an ACE.
		// The ACE will allow Everyone read access to the key.
		ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
		ea.grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea.Trustee.ptstrName = (LPTSTR) pEveryoneSID;

		// Create a new ACL that contains the new ACEs.
		PACL pACL = NULL;
		if (fnSetEntriesInAcl(1, &ea, NULL, &pACL) == ERROR_SUCCESS)
		{
			// Initialize a security descriptor.
			PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) new char[SECURITY_DESCRIPTOR_MIN_LENGTH];

			if (fnInitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
			{
				// Add the ACL to the security descriptor.
				if (fnSetSecurityDescriptorDacl(pSD,
						TRUE,     // bDaclPresent flag
						pACL,
						FALSE))   // not a default DACL
				{
					if (fnSetFileSecurity(theFileName.c_str(), DACL_SECURITY_INFORMATION, pSD))
						result = true;
				}
			}
		}
	}

	FreeLibrary(aLib);
	return result;
#else
	return true;
#endif
}

bool Sexy::Deltree(const std::string& thePath)
{
	bool success = true;
#ifdef WIN32
	std::string aSourceDir = thePath;

	if (aSourceDir.length() < 2)
		return false;

	if ((aSourceDir[aSourceDir.length() - 1] != '\\') ||
		(aSourceDir[aSourceDir.length() - 1] != '/'))
		aSourceDir += "\\";

	WIN32_FIND_DATAA aFindData;

	HANDLE aFindHandle = FindFirstFileA((aSourceDir + "*.*").c_str(), &aFindData);
	if (aFindHandle == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if ((aFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			if ((strcmp(aFindData.cFileName, ".") != 0) &&
				(strcmp(aFindData.cFileName, "..") != 0))
			{
				// Follow the directory
				if (!Deltree(aSourceDir + aFindData.cFileName))
					success = false;
			}
		}
		else
		{
			std::string aFullName = aSourceDir + aFindData.cFileName;
			if (!DeleteFileA(aFullName.c_str()))
				success = false;
		}
	}
	while (FindNextFileA(aFindHandle, &aFindData));
	FindClose(aFindHandle);

	if (rmdir(thePath.c_str()) == 0)
		success = false;
#else
#endif
	return success;
}

bool Sexy::FileExists(const std::string& theFileName)
{
#ifdef WIN32
	WIN32_FIND_DATAA aFindData;

	HANDLE aFindHandle = FindFirstFileA(theFileName.c_str(), &aFindData);
	if (aFindHandle == INVALID_HANDLE_VALUE)
		return false;

	FindClose(aFindHandle);
#else
	return access (theFileName.c_str(), F_OK) == 0;
#endif
	return true;
}

void Sexy::MkDir(const std::string& theDir)
{
	std::string aPath = theDir;

	int aCurPos = 0;
	for (;;)
	{
		int aSlashPos = aPath.find_first_of("\\/", aCurPos);
		if (aSlashPos == -1)
		{
#ifdef WIN32
			_mkdir(aPath.c_str());
#else
			mkdir(aPath.c_str(), 0755);
#endif
			break;
		}

		aCurPos = aSlashPos+1;

		std::string aCurPath = aPath.substr(0, aSlashPos);
#ifdef WIN32
		_mkdir(aCurPath.c_str());
#else
		mkdir(aCurPath.c_str(), 0755);
#endif
	}
}

std::string Sexy::GetFileName(const std::string& thePath, bool noExtension)
{
	int aLastSlash = (std::max)((int) thePath.rfind('\\'), (int) thePath.rfind('/'));

	if (noExtension)
	{
		int aLastDot = (int)thePath.rfind('.');
		if (aLastDot > aLastSlash)
			return thePath.substr(aLastSlash + 1, aLastDot - aLastSlash - 1);
	}

	if (aLastSlash == -1)
		return thePath;
	else
		return thePath.substr(aLastSlash + 1);
}

std::string Sexy::GetFileDir(const std::string& thePath, bool withSlash)
{
	int aLastSlash = (std::max)((int) thePath.rfind('\\'), (int) thePath.rfind('/'));

	if (aLastSlash == -1)
		return "";
	else
	{
		if (withSlash)
			return thePath.substr(0, aLastSlash+1);
		else
			return thePath.substr(0, aLastSlash);
	}
}

std::string Sexy::RemoveTrailingSlash(const std::string& theDirectory)
{
	int aLen = theDirectory.length();

	if ((aLen > 0) && ((theDirectory[aLen-1] == '\\') || (theDirectory[aLen-1] == '/')))
		return theDirectory.substr(0, aLen - 1);
	else
		return theDirectory;
}

std::string	Sexy::AddTrailingSlash(const std::string& theDirectory, bool backSlash)
{
	if (!theDirectory.empty())
	{
		char aChar = theDirectory[theDirectory.length()-1];
		if (aChar!='\\' && aChar!='/')
			return theDirectory + (backSlash?'\\':'/');
		else
			return theDirectory;
	}
	else
		return "";
}


time_t Sexy::GetFileDate(const std::string& theFileName)
{
	time_t aFileDate = 0;
#if WIN32
	WIN32_FIND_DATAA aFindData;
	HANDLE aFindHandle = ::FindFirstFileA(theFileName.c_str(), &aFindData);

	if (aFindHandle != INVALID_HANDLE_VALUE)
	{
		FILETIME aFileTime = aFindData.ftLastWriteTime;

		//FileTimeToUnixTime(&aFileTime, &aFileDate, FALSE);

		LONGLONG ll = (__int64) aFileTime.dwHighDateTime << 32;
		LONGLONG offset = 116444736;
		offset *= 1000000000;
		ll = ll + aFileTime.dwLowDateTime - offset;
		aFileDate = (time_t) (ll/10000000);

		FindClose(aFindHandle);
	}
#else
#endif
	return aFileDate;
}

std::string Sexy::vformat(const char* fmt, va_list argPtr)
{
	// We draw the line at a 1MB string.
	const int maxSize = 1000000;

	// If the string is less than 1024 characters,
	// allocate it on the stack because this saves
	// the malloc/free time.
	const int bufSize = 2049;
	char stackBuffer[bufSize];

	int attemptedSize = bufSize - 1;

	int numChars = 0;
#ifdef _WIN32
	numChars = _vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#else
	numChars = vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#endif

	//cout << "NumChars: " << numChars << endl;

	if ((numChars >= 0) && (numChars <= attemptedSize))
	{
		// Needed for case of 2048-character printf thing
		stackBuffer[numChars] = '\0';

		// Got it on the first try.
		return std::string(stackBuffer);
	}

	// Now use the heap.
	char* heapBuffer = NULL;

	while (((numChars == -1) || (numChars > attemptedSize)) &&
	       (attemptedSize < maxSize))
	{
		// Try a bigger size
		attemptedSize *= 2;
		heapBuffer = (char*)realloc(heapBuffer, (attemptedSize + 1));
#ifdef _WIN32
		numChars = _vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#else
		numChars = vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#endif
	}

	heapBuffer[numChars] = 0;

	std::string result = std::string(heapBuffer);

	free(heapBuffer);
	return result;
}

//overloaded StrFormat: should only be used by the xml strings
std::string Sexy::StrFormat(const char* fmt ...)
{
    va_list argList;
    va_start(argList, fmt);
	std::string result = vformat(fmt, argList);
    va_end(argList);

    return result;
}

std::string Sexy::Evaluate(const std::string& theString, const DefinesMap& theDefinesMap)
{
	std::string anEvaluatedString = theString;

	for (;;)
	{
		int aPercentPos = anEvaluatedString.find('%');

		if (aPercentPos == std::string::npos)
			break;

		int aSecondPercentPos = anEvaluatedString.find('%', aPercentPos + 1);
		if (aSecondPercentPos == std::string::npos)
			break;

		std::string aName = anEvaluatedString.substr(aPercentPos + 1, aSecondPercentPos - aPercentPos - 1);

		std::string aValue;
		DefinesMap::const_iterator anItr = theDefinesMap.find(aName);
		if (anItr != theDefinesMap.end())
			aValue = anItr->second;
		else
			aValue = "";

		anEvaluatedString.erase(anEvaluatedString.begin() + aPercentPos, anEvaluatedString.begin() + aSecondPercentPos + 1);
		anEvaluatedString.insert(anEvaluatedString.begin() + aPercentPos, aValue.begin(), aValue.begin() + aValue.length());
	}

	return anEvaluatedString;
}

std::string Sexy::XMLDecodeString(const std::string& theString)
{
	std::string aNewString;

	int aUTF8Len = 0;
	int aUTF8CurVal = 0;

	for (ulong i = 0; i < theString.length(); i++)
	{
		char c = theString[i];

		if (c == '&')
		{
			int aSemiPos = theString.find(';', i);

			if (aSemiPos != -1)
			{
				std::string anEntName = theString.substr(i+1, aSemiPos-i-1);
				i = aSemiPos;

				if (anEntName == "lt")
					c = '<';
				else if (anEntName == "amp")
					c = '&';
				else if (anEntName == "gt")
					c = '>';
				else if (anEntName == "quot")
					c = '"';
				else if (anEntName == "apos")
					c = '\'';
				else if (anEntName == "nbsp")
					c = ' ';
				else if (anEntName == "cr")
					c = '\n';
			}
		}

		aNewString += c;
	}

	return aNewString;
}

Sexy::WString Sexy::XMLDecodeString(const Sexy::WString& theString)
{
	Sexy::WString aNewString;

	int aUTF8Len = 0;
	int aUTF8CurVal = 0;

	for (size_t i = 0; i < theString.length(); i++)
	{
		int c = theString[i];

		if (c == '&')
		{
			int aSemiPos = theString.find(';', i);

			if (aSemiPos != -1)
			{
				Sexy::WString anEntName = theString.substr(i+1, aSemiPos-i-1);
				i = aSemiPos;

				if (anEntName == WSTR("lt"))
					c = '<';
				else if (anEntName == WSTR("amp"))
					c = '&';
				else if (anEntName == WSTR("gt"))
					c = '>';
				else if (anEntName == WSTR("quot"))
					c = '"';
				else if (anEntName == WSTR("apos"))
					c = '\'';
				else if (anEntName == WSTR("nbsp"))
					c = ' ';
				else if (anEntName == WSTR("cr"))
					c = '\n';
			}
		}

		aNewString += c;
	}

	return aNewString;
}

std::string Sexy::XMLEncodeString(const std::string& theString)
{
	std::string aNewString;

	bool hasSpace = false;

	for (ulong i = 0; i < theString.length(); i++)
	{
		char c = theString[i];

		if (c == ' ')
		{
			if (hasSpace)
			{
				aNewString += "&nbsp;";
				continue;
			}

			hasSpace = true;
		}
		else
			hasSpace = false;

		/*if ((uchar) c >= 0x80)
		{
			// Convert to UTF
			aNewString += (char) (0xC0 | ((c >> 6) & 0xFF));
			aNewString += (char) (0x80 | (c & 0x3F));
		}
		else*/
		{
			switch (c)
			{
			case '<':
				aNewString += "&lt;";
				break;
			case '&':
				aNewString += "&amp;";
				break;
			case '>':
				aNewString += "&gt;";
				break;
			case '"':
				aNewString += "&quot;";
				break;
			case '\'':
				aNewString += "&apos;";
				break;
			case '\n':
				aNewString += "&cr;";
				break;
			default:
				aNewString += c;
				break;
			}
		}
	}

	return aNewString;
}

Sexy::WString Sexy::XMLEncodeString(const Sexy::WString& theString)
{
	Sexy::WString aNewString;

	bool hasSpace = false;

	for (size_t i = 0; i < theString.length(); i++)
	{
		int c = theString[i];

		if (c == ' ')
		{
			if (hasSpace)
			{
				aNewString += WSTR("&nbsp;");
				continue;
			}

			hasSpace = true;
		}
		else
			hasSpace = false;

		/*if ((uchar) c >= 0x80)
		{
			// Convert to UTF
			aNewString += (char) (0xC0 | ((c >> 6) & 0xFF));
			aNewString += (char) (0x80 | (c & 0x3F));
		}
		else*/
		{
			switch (c)
			{
			case '<':
				aNewString += WSTR("&lt;");
				break;
			case '&':
				aNewString += WSTR("&amp;");
				break;
			case '>':
				aNewString += WSTR("&gt;");
				break;
			case '"':
				aNewString += WSTR("&quot;");
				break;
			case '\'':
				aNewString += WSTR("&apos;");
				break;
			case '\n':
				aNewString += WSTR("&cr;");
				break;
			default:
				aNewString += c;
				break;
			}
		}
	}

	return aNewString;
}

std::string Sexy::Upper(const std::string& _data)
{
	std::string s = _data;
	std::transform(s.begin(), s.end(), s.begin(), toupper);
	return s;
}
Sexy::WString Sexy::Upper(const Sexy::WString& _data)
{
	Sexy::WString s = _data;
	std::transform(s.begin(), s.end(), s.begin(), towupper);
	return s;
}

std::string Sexy::Lower(const std::string& _data)
{
	std::string s = _data;
	std::transform(s.begin(), s.end(), s.begin(), tolower);
	return s;
}

Sexy::WString Sexy::Lower(const Sexy::WString& _data)
{
	Sexy::WString s = _data;
	std::transform(s.begin(), s.end(), s.begin(), tolower);
	return s;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int Sexy::StrFindNoCase(const char *theStr, const char *theFind)
{
	int p1,p2;
	int cp = 0;
	const int len1 = (int)strlen(theStr);
	const int len2 = (int)strlen(theFind);
	while(cp < len1)
	{
		p1 = cp;
		p2 = 0;
		while(p1<len1 && p2<len2)
		{
			if(tolower(theStr[p1])!=tolower(theFind[p2]))
				break;

			p1++; p2++;
		}
		if(p2==len2)
			return p1-len2;

		cp++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool Sexy::StrPrefixNoCase(const char *theStr, const char *thePrefix, int maxLength)
{
	int i;
	char c1 = 0, c2 = 0;
	for (i=0; i<maxLength; i++)
	{
		c1 = tolower(*theStr++);
		c2 = tolower(*thePrefix++);

		if (c1==0 || c2==0)
			break;

		if (c1!=c2)
			return false;
	}

	return c2==0 || i==maxLength;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char* Sexy::GetEnv(const char *option)
{
	return getenv (option);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool Sexy::GetEnvOption(const char *option,  bool value)
{
	const char *s;

	s = getenv (option);
	if (s && (!strcmp(s, "1") || !strcmp(s, "yes") ||
		  !strcmp(s, "true") || !strcmp(s, "True")))
		value = true;
	else if (s && (!strcmp(s, "0") || !strcmp(s, "no") ||
		       !strcmp(s, "false") || !strcmp(s, "False")))
		value = false;
	return value;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int Sexy::GetEnvIntOption(const char *option, int value)
{
	const char *s;

	s = getenv (option);
	if (s)
		return atoi(s);
	return value;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Sexy::Split(const std::string& s, const std::string& delim,
		 std::vector<std::string>& ret)
{
        size_t last = 0;
        size_t index = s.find_first_of(delim, last);

        while (index != std::string::npos)
        {
                ret.push_back(s.substr(last, index - last));
                last = index + 1;
                index = s.find_first_of(delim, last);
        }
        if (index - last > 0)
        {
                ret.push_back(s.substr(last, index-last));
        }
}

