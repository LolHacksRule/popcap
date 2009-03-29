#include "Common.h"
#include "DarwinRegistryInterface.h"

#include <CoreFoundation/CoreFoundation.h>

using namespace Sexy;

DarwinRegistryInterface::DarwinRegistryInterface(SexyAppBase * theApp)
	: RegistryInterface (theApp)
{
	CFPreferencesAppSynchronize (kCFPreferencesCurrentApplication);
}

DarwinRegistryInterface::~DarwinRegistryInterface()
{
	Flush ();
}

bool DarwinRegistryInterface::Write(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength)
{
	return false;
}

bool DarwinRegistryInterface::Read(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength)
{
	return false;
}

bool DarwinRegistryInterface::ReadKey(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength, HKEY theKey)
{
	return false;
}

bool DarwinRegistryInterface::GetSubKeys(const std::string& theKeyName, StringVector* theSubKeys)
{
	return false;
}

static std::string StringFromCFString(CFStringRef theStrRef)
{
	static std::string empty;

	if (!theStrRef)
		return empty;

	CFIndex size = sizeof(UniChar) * CFStringGetLength(theStrRef) + 1;
	char * cstr;

	cstr = new char[size];
	if (!CFStringGetCString(theStrRef, cstr, size, kCFStringEncodingASCII))
	{
		if (!CFStringGetCString(theStrRef, cstr, size, kCFStringEncodingUTF8))
		{
			delete [] cstr;
			return empty;
		}
	}

	std::string result = std::string (cstr);
	delete [] cstr;

	return result;
}

bool DarwinRegistryInterface::ReadString(const std::string& theKey, std::string* theString)
{
	CFStringRef name = CFStringCreateWithBytes (NULL,
						    (UInt8*) theKey.data(), theKey.size(),
						    kCFStringEncodingMacRoman, false);

	CFStringRef value = (CFStringRef)
		CFPreferencesCopyAppValue (name,
					   kCFPreferencesCurrentApplication);
	if (!value)
	{
		CFRelease (name);
		return false;
	}

	*theString = StringFromCFString (value);

	CFRelease (value);
	CFRelease (name);
	return true;
}

bool DarwinRegistryInterface::ReadInteger(const std::string& theKey, int* theValue)
{
	CFStringRef name = CFStringCreateWithBytes (NULL,
						    (UInt8*) theKey.data(), theKey.size(),
						    kCFStringEncodingMacRoman, false);
	Boolean valid = false;
	CFIndex result = CFPreferencesGetAppIntegerValue (name, kCFPreferencesCurrentApplication,
							  &valid);
	if (valid)
		*theValue = result;

	CFRelease (name);
	return valid;
}

bool DarwinRegistryInterface::ReadBoolean(const std::string& theKey, bool* theValue)
{
	CFStringRef name = CFStringCreateWithBytes (NULL,
						    (UInt8*) theKey.data(), theKey.size(),
						    kCFStringEncodingMacRoman, false);
	Boolean valid = false;
	CFIndex result = CFPreferencesGetAppIntegerValue (name, kCFPreferencesCurrentApplication,
							  &valid);
	if (valid)
		*theValue = !!result;

	CFRelease (name);
	return valid;

}

bool DarwinRegistryInterface::ReadData(const std::string& theKey, uchar* theValue, ulong* theLength)
{
	return false;
}

bool DarwinRegistryInterface::WriteString(const std::string& theKey, const std::string& theString)
{
	CFStringRef name = CFStringCreateWithBytes (NULL,
						    (UInt8*) theKey.data(), theKey.size(),
						    kCFStringEncodingMacRoman, false);
	CFStringRef value = CFStringCreateWithBytes (NULL,
						     (UInt8*) theString.data(), theString.size(),
						     kCFStringEncodingMacRoman, false);

	CFPreferencesSetAppValue (name, value, kCFPreferencesCurrentApplication);
	
	CFRelease (name);
	CFRelease (value);
	return true;
}

bool DarwinRegistryInterface::WriteInteger(const std::string& theKey, int theValue)
{
	CFStringRef name = CFStringCreateWithBytes (NULL,
						    (UInt8*) theKey.data(), theKey.size(),
						    kCFStringEncodingMacRoman, false);
	CFNumberRef value = CFNumberCreate (kCFAllocatorDefault, kCFNumberIntType, &theValue);

	CFPreferencesSetAppValue (name, value, kCFPreferencesCurrentApplication);

	CFRelease (name);
	CFRelease (value);
	return true;
}

bool DarwinRegistryInterface::WriteBoolean(const std::string& theKey, bool theValue)
{
	CFStringRef name = CFStringCreateWithBytes (NULL,
						    (UInt8*) theKey.data(), theKey.size(),
						    kCFStringEncodingMacRoman, false);

	CFPreferencesSetAppValue (name, theValue ? kCFBooleanTrue : kCFBooleanFalse,
				  kCFPreferencesCurrentApplication);

	CFRelease (name);
	return true;
}

bool DarwinRegistryInterface::WriteData(const std::string& theValueName, const uchar* theValue, ulong theLength)
{
	return false;
}

bool DarwinRegistryInterface::EraseKey(const SexyString& theKeyName)
{
	return false;
}

void DarwinRegistryInterface::EraseValue(const SexyString& theValueName)
{
}

void DarwinRegistryInterface::Flush()
{
	CFPreferencesAppSynchronize (kCFPreferencesCurrentApplication);
}
