#include "Common.h"
#include "XMLParser.h"
#include "XMLWriter.h"
#include "XMLRegistryInterface.h"

#include <sstream>

using namespace Sexy;

XMLRegistryInterface::XMLRegistryInterface(SexyAppBase * theApp)
	: RegistryInterface(theApp)
{
	mLoaded = false;
}

XMLRegistryInterface::~XMLRegistryInterface()
{
	Flush();
}

bool XMLRegistryInterface::Write(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength)
{
	Load();

	if (!mRegKey.length())
		return false;

	std::string aValueName;
	std::string::size_type aSlashPos = theValueName.rfind('\\');
	if (aSlashPos != std::string::npos)
	{
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

        std::ostringstream stream;
        if (theType == REG_DWORD) {
		int i;
		memcpy(&i, theValue, sizeof(int));;
		stream << i;
        }
        else if (theType == REG_SZ)
	{
		stream << theValue;
        }
        else if (theType == REG_BINARY)
	{
		uchar* buf = new uchar[theLength + 1];
		memcpy(buf, theValue, theLength);
		buf[theLength] = '\0';
		stream << buf;
		delete [] buf;
        }
        mRegistry[StringToSexyString(aValueName)] = StringToSexyString(stream.str());
	return true;
}

static std::string FileNameFromRegKey(std::string theRegKey)
{
	std::string &copy = theRegKey;

	std::string::size_type pos = copy.find_first_of("\\/");

	while (pos != std::string::npos) {
		copy.replace(pos, 1, ".");
		pos = copy.find_first_of("\\/");
	}

	return copy + ".xml";
}


void XMLRegistryInterface::Load()
{
	if (mLoaded)
		return;

    	mRegKey = mApp->mRegKey;
	if (!mRegKey.length())
		return;

	std::string FileName = FileNameFromRegKey(mRegKey);

	XMLParser parser;

	std::string path = GetAppDataFolder() + FileName;

	if (parser.OpenFile(path)) {
		XMLElement e;

		std::string key;
		std::string value;

		mRegistry.clear();

		while (parser.NextElement(&e))
		{
			if (e.mType == XMLElement::TYPE_START)
			{
				if (e.mSection == "Registry")
				{
					if (e.mValue == "Key")
					{
						key = e.mAttributes["ID"];
					}
				}
				else if (e.mSection == "Registry/Key")
				{
					if (e.mValue == "Value")
					{
						value= e.mAttributes["value"];
					}
				}
			}
			else if (e.mType == XMLElement::TYPE_END)
			{
				if (e.mSection == "Registry/Key")
				{
					printf ("%s -> %s\n", key.c_str(), value.c_str());
					mRegistry[key] = value;
				}
			}
		}
	}

	mLoaded = true;
}

bool XMLRegistryInterface::ReadKey(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength, HKEY theKey)
{
	Load();

	std::string aValueName;

	int aSlashPos = (int) theValueName.rfind('\\');
	if (aSlashPos != -1)
	{
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	SexyString s = mRegistry[aValueName];
	if (s == "")
		return false;

	uchar* c_str = (uchar*)(SexyStringToString(s)).c_str();
	if (*theType == REG_SZ) {
		memcpy(theValue, c_str, 1023);
	}
	else if (*theType == REG_DWORD)
	{
		int i = atoi((const char*)c_str);
		int *ptr = (int*)theValue;
		*ptr = i;
	}
	else if (*theType == REG_BINARY)
	{
		memcpy(theValue, c_str, *theLength);
	}
	return true;
}

bool XMLRegistryInterface::GetSubKeys(const std::string& theKeyName, StringVector* theSubKeys)
{
	Load();
	return false;
}

bool XMLRegistryInterface::EraseKey(const SexyString& theKeyName)
{
	Load();

	if (mRegKey.length() == 0)
		return false;

        mRegistry.erase(theKeyName);
	return true;
}

void XMLRegistryInterface::EraseValue(const SexyString& theValueName)
{
	Load();

	if (mRegKey.length() == 0)
		return;

	std::map<SexyString, SexyString>::iterator it = mRegistry.begin();
	while (it != mRegistry.end())
	{
		if (it->second == theValueName)
			it->second = "";
		++it;
	}
}

void XMLRegistryInterface::Flush()
{
	if (!mRegKey.length())
		return;

        XMLWriter writer;

        std::string path = GetAppDataFolder();
	MkDir(path);

	std::string FileName = FileNameFromRegKey(mRegKey);
        path += FileName;

        if (!writer.OpenFile(path))
		return;

        writer.StartElement("Registry");

        std::map<SexyString, SexyString>::const_iterator it = mRegistry.begin();
        while (it != mRegistry.end())
	{
		writer.StartElement("Key");
		writer.WriteAttribute("ID", it->first);
		writer.StartElement("Value");
		writer.WriteAttribute("value", it->second);
		writer.StopElement();
		writer.StopElement();
		++it;
        }
        writer.StopElement();
}
