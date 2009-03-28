#include "Common.h"
#include "Win32RegistryInterface.h"

#include <sstream>

using namespace Sexy;

Win32RegistryInterface::Win32RegistryInterface(SexyAppBase * theApp)
	: RegistryInterface(theApp)
{
	mLoaded = false;
}

Win32RegistryInterface::~Win32RegistryInterface()
{
	Flush();
}

bool Win32RegistryInterface::Write(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength)
{
	Load();

	HKEY aGameKey;

	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = (int) theValueName.rfind('\\');
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	int aResult = RegOpenKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, KEY_WRITE, &aGameKey);
	if (aResult != ERROR_SUCCESS)
	{
		ulong aDisp;
		aResult = RegCreateKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, "Key", REG_OPTION_NON_VOLATILE,
					  KEY_ALL_ACCESS, NULL, &aGameKey, &aDisp);
	}

	if (aResult != ERROR_SUCCESS)
		return false;

	RegSetValueExA(aGameKey, aValueName.c_str(), 0, theType, theValue, theLength);
	RegCloseKey(aGameKey);

	return true;
}

void Win32RegistryInterface::Load()
{
	if (mLoaded)
		return;

	mRegKey = mApp->mRegKey;
	mLoaded = true;
}

bool Win32RegistryInterface::ReadKey(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength, HKEY theKey)
{
	if (mRegKey.length() == 0)
		return false;

	HKEY aGameKey;

	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = (int) theValueName.rfind('\\');
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	if (RegOpenKeyExA(theKey, aKeyName.c_str(), 0, KEY_READ, &aGameKey) != ERROR_SUCCESS)
		return false;

	if (RegQueryValueExA(aGameKey, aValueName.c_str(), 0, theType, (uchar*) theValue, theLength) != ERROR_SUCCESS)
	{
		RegCloseKey(aGameKey);
		return false;
	}

	RegCloseKey(aGameKey);
	return true;
}

bool Win32RegistryInterface::GetSubKeys(const std::string& theKeyName, StringVector* theSubKeys)
{
	theSubKeys->clear();

	if (mRegKey.length() == 0)
		return false;

	HKEY aKey;

	std::string aKeyName = RemoveTrailingSlash(RemoveTrailingSlash("SOFTWARE\\" + mRegKey) + "\\" + theKeyName);
	int aResult = RegOpenKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, KEY_READ, &aKey);

	if (aResult != ERROR_SUCCESS)
		return false;

	for (int anIdx = 0; ; anIdx++)
	{
		char aStr[1024];

		aResult = RegEnumKeyA(aKey, anIdx, aStr, 1024);
		if (aResult != ERROR_SUCCESS)
			break;

		theSubKeys->push_back(aStr);
	}

	RegCloseKey(aKey);

	return true;
}

bool Win32RegistryInterface::EraseKey(const SexyString& _theKeyName)
{
	std::string theKeyName = SexyStringToStringFast(_theKeyName);

	if (mRegKey.length() == 0)
		return false;

	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey) + "\\" + theKeyName;

	int aResult = RegDeleteKeyA(HKEY_CURRENT_USER, aKeyName.c_str());
	if (aResult != ERROR_SUCCESS)
		return false;
	return true;
}

void Win32RegistryInterface::EraseValue(const SexyString& _theValueName)
{
	std::string theValueName = SexyStringToStringFast(_theValueName);
	if (mRegKey.length() == 0)
		return;

	HKEY aGameKey;
	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = (int) theValueName.rfind('\\');
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	int aResult = RegOpenKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, KEY_WRITE, &aGameKey);
	if (aResult != ERROR_SUCCESS)
		return;

	RegDeleteValueA(aGameKey, aValueName.c_str());
	RegCloseKey(aGameKey);
}

void Win32RegistryInterface::Flush()
{
}
