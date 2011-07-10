#include "PropertiesParser.h"
#include "XMLParser.h"
#include <stdlib.h>

using namespace Sexy;

PropertiesParser::PropertiesParser(SexyAppBase* theApp)
{
	mApp = theApp;
	mHasFailed = false;
	mXMLParser = NULL;
}

void PropertiesParser::Fail(const SexyString& theErrorText)
{
	if (!mHasFailed)
	{
		mHasFailed = true;
		int aLineNum = mXMLParser->GetCurrentLineNum();

		mError = theErrorText;
		if (aLineNum > 0) mError += StrFormat(" on Line %d", aLineNum);
		if (!mXMLParser->GetFileName().empty()) mError += StrFormat(" in File '%s'", mXMLParser->GetFileName().c_str());
	}
}


PropertiesParser::~PropertiesParser()
{
}


bool PropertiesParser::ParseSingleElement(SexyString* aString)
{
	*aString = "";

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			Fail("Unexpected Section: '" + aXMLElement.mValue + "'");
			return false;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			*aString = aXMLElement.mValue;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

bool PropertiesParser::ParseStringArray(StringVector* theStringVector)
{
	theStringVector->clear();

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "String")
			{
				SexyString aString;

				if (!ParseSingleElement(&aString))
					return false;

				theStringVector->push_back(SexyStringToStringFast(aString));
			}
			else
			{
				Fail("Invalid Section '" + aXMLElement.mValue + "'");
				return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			Fail("Element Not Expected '" + aXMLElement.mValue + "'");
			return false;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}


bool PropertiesParser::ParseProperties()
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "String")
			{
				SexyString aDef;
				if (!ParseSingleElement(&aDef))
					return false;

				std::string anId = SexyStringToStringFast(aXMLElement.mAttributes["id"]);
				mApp->SetString(anId, Sexy::WString(aDef.begin(), aDef.end()));
			}
			else if (aXMLElement.mValue == "StringArray")
			{
				StringVector aDef;

				if (!ParseStringArray(&aDef))
					return false;

				std::string anId = SexyStringToStringFast(aXMLElement.mAttributes["id"]);

				mApp->mStringVectorProperties.insert(StringStringVectorMap::value_type(anId, aDef));
			}
			else if (aXMLElement.mValue == "Boolean")
			{
				SexyString aVal;

				if (!ParseSingleElement(&aVal))
					return false;

				aVal = Upper(aVal);

				bool boolVal;
				if ((aVal == "1") || (aVal == "YES") || (aVal == "ON") || (aVal == "TRUE"))
					boolVal = true;
				else if ((aVal == "0") || (aVal == "NO") || (aVal == "OFF") || (aVal == "FALSE"))
					boolVal = false;
				else
				{
					Fail("Invalid Boolean Value: '" + aVal + "'");
					return false;
				}

				std::string anId = SexyStringToStringFast(aXMLElement.mAttributes["id"]);

				mApp->SetBoolean(anId, boolVal);
			}
			else if (aXMLElement.mValue == "Integer")
			{
				SexyString aVal;

				if (!ParseSingleElement(&aVal))
					return false;

				int anInt;
				if (!StringToInt(aVal, &anInt))
				{
					Fail("Invalid Integer Value: '" + aVal + "'");
					return false;
				}

				std::string anId = SexyStringToStringFast(aXMLElement.mAttributes["id"]);

				mApp->SetInteger(anId, anInt);
			}
			else if (aXMLElement.mValue == "Double")
			{
				SexyString aVal;

				if (!ParseSingleElement(&aVal))
					return false;

				double aDouble;
				if (!StringToDouble(aVal, &aDouble))
				{
					Fail("Invalid Double Value: '" + aVal + "'");
					return false;
				}

				std::string anId = SexyStringToStringFast(aXMLElement.mAttributes["id"]);

				mApp->SetDouble(anId, aDouble);
			}
			else
			{
				Fail("Invalid Section '" + aXMLElement.mValue + "'");
				return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			Fail("Element Not Expected '" + aXMLElement.mValue + "'");
			return false;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

bool PropertiesParser::DoParseProperties()
{
	if (!mXMLParser->HasFailed())
	{
		for (;;)
		{
			XMLElement aXMLElement;
			if (!mXMLParser->NextElement(&aXMLElement))
				break;

			if (aXMLElement.mType == XMLElement::TYPE_START)
			{
				if (aXMLElement.mValue == "Properties")
				{
					if (!ParseProperties())
						break;
				}
				else
				{
					Fail("Invalid Section '" + aXMLElement.mValue + "'");
					break;
				}
			}
			else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
			{
				Fail("Element Not Expected '" + aXMLElement.mValue + "'");
				break;
			}
		}
	}

	if (mXMLParser->HasFailed())
		Fail(mXMLParser->GetErrorText());

	delete mXMLParser;
	mXMLParser = NULL;

	return !mHasFailed;
}

bool PropertiesParser::ParsePropertiesBuffer(const Buffer& theBuffer)
{
	mXMLParser = new XMLParser();

	mXMLParser->SetStringSource(std::string((const char *)theBuffer.GetDataPtr(),
						theBuffer.GetDataLen()));
	return DoParseProperties();
}

bool PropertiesParser::ParsePropertiesFile(const std::string& theFilename)
{
	mXMLParser = new XMLParser();
	mXMLParser->OpenFile(theFilename);
	return DoParseProperties();
}

SexyString PropertiesParser::GetErrorText()
{
	return mError;
}
