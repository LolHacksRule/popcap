#include "Common.h"
#include "RegistryInterface.h"

using namespace Sexy;

RegistryInterface::RegistryInterface(SexyAppBase * theApp)
{
	mApp = theApp;
}

bool RegistryInterface::Write(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength)
{
	return false;
}

bool RegistryInterface::Read(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength)
{
	return ReadKey(theValueName, theType, theValue, theLength, HKEY_CURRENT_USER);
}

bool RegistryInterface::ReadKey(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength, HKEY theKey)
{
	return false;
}

bool RegistryInterface::GetSubKeys(const std::string& theKeyName, StringVector* theSubKeys)
{
	return false;
}

bool RegistryInterface::ReadString(const std::string& theKey, std::string* theString)
{
	char aStr[1024];

	ulong aType = REG_SZ;
	ulong aLen = sizeof(aStr) - 1;

	if (!Read(theKey, &aType, (uchar*) aStr, &aLen))
		return false;

	aStr[aLen] = 0;
	*theString = aStr;
	return true;
}

bool RegistryInterface::ReadInteger(const std::string& theKey, int* theValue)
{
	ulong aType = REG_DWORD;
	uint aLong;
	ulong aLen = 4;

	if (!Read(theKey, &aType, (uchar*) &aLong, &aLen))
		return false;

	*theValue = aLong;
	return true;
}

bool RegistryInterface::ReadBoolean(const std::string& theKey, bool* theValue)
{
	int aValue;

	if (!ReadInteger(theKey, &aValue))
		return false;

	*theValue = aValue != 0;
	return true;
}

bool RegistryInterface::ReadData(const std::string& theKey, uchar* theValue, ulong* theLength)
{
	ulong aType = REG_BINARY;
	ulong aLen = *theLength;

	if (!Read(theKey, &aType, (uchar*) theValue, theLength))
		return false;

	return true;
}

bool RegistryInterface::WriteString(const std::string& theValueName, const std::string& theString)
{
	return Write(theValueName, REG_SZ, (uchar*) theString.c_str(), theString.length());
}

bool RegistryInterface::WriteInteger(const std::string& theValueName, int theValue)
{
	return Write(theValueName, REG_DWORD, (uchar*) &theValue, sizeof(int));
}

bool RegistryInterface::WriteBoolean(const std::string& theValueName, bool theValue)
{
	int aValue = theValue ? 1 : 0;

	return Write(theValueName, REG_DWORD, (uchar*) &aValue, sizeof(int));
}

bool RegistryInterface::WriteData(const std::string& theValueName, const uchar* theValue, ulong theLength)
{
	return Write(theValueName, REG_BINARY, (uchar*) theValue, theLength);
}

bool RegistryInterface::EraseKey(const SexyString& theKeyName)
{
	return false;
}

void RegistryInterface::EraseValue(const SexyString& theValueName)
{
}

void RegistryInterface::Flush()
{
}
