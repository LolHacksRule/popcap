#include "DescParser.h"
#include "../PakLib/PakInterface.h"

using namespace Sexy;

//////////////////////////////////////////////////////////////////////////

DataElement::DataElement() :
mIsList(false)
{
}

DataElement::~DataElement()
{
}

SingleDataElement::SingleDataElement()
{
	mIsList = false;
	mValue = NULL;
}

SingleDataElement::SingleDataElement(const std::wstring& theString) :
	mString(theString)
{
	mIsList = false;
	mValue = NULL;
}

SingleDataElement::~SingleDataElement()
{
	if (mValue != NULL)
		delete mValue;
}

DataElement* SingleDataElement::Duplicate()
{
	SingleDataElement* aSingleDataElement = new SingleDataElement(*this);
	if (mValue != NULL)
		aSingleDataElement->mValue = mValue->Duplicate();
	return aSingleDataElement;
}

ListDataElement::ListDataElement()
{
	mIsList = true;
}

ListDataElement::~ListDataElement()
{
	for (ulong i = 0; i < mElementVector.size(); i++)
		delete mElementVector[i];
}

ListDataElement::ListDataElement(const ListDataElement& theListDataElement)
{
	mIsList = true;
	for (ulong i = 0; i < theListDataElement.mElementVector.size(); i++)
		mElementVector.push_back(theListDataElement.mElementVector[i]->Duplicate());
}

ListDataElement& ListDataElement::operator=(const ListDataElement& theListDataElement)
{
	ulong i;

	for (i = 0; i < mElementVector.size(); i++)
		delete mElementVector[i];
	mElementVector.clear();

	for (i = 0; i < theListDataElement.mElementVector.size(); i++)
		mElementVector.push_back(theListDataElement.mElementVector[i]->Duplicate());

	return *this;
}

DataElement* ListDataElement::Duplicate()
{
	ListDataElement* aListDataElement = new ListDataElement(*this);
	return aListDataElement;
}

//////////////////////////////////////////////////////////////////////////

DescParser::DescParser()
{
	mCmdSep = CMDSEP_SEMICOLON;
}

DescParser::~DescParser()
{
}

bool DescParser::Error(const std::wstring& theError)
{
	if (mError.length() == 0)
		mError = theError;
	return false;
}

DataElement* DescParser::Dereference(const std::wstring& theString)
{
	std::wstring aDefineName = StringToUpper(theString);

	DataElementMap::iterator anItr = mDefineMap.find(aDefineName);
	if (anItr != mDefineMap.end())
		return anItr->second;
	else
		return NULL;
}

bool DescParser::IsImmediate(const std::wstring& theString)
{
	return (((theString[0] >= L'0') && (theString[0] <= L'9')) || (theString[0] == L'-') ||
		(theString[0] == L'+') || (theString[0] == L'\'') || (theString[0] == L'"'));
}

std::wstring DescParser::Unquote(const std::wstring& theQuotedString)
{
	if ((theQuotedString[0] == L'\'') || (theQuotedString[0] == L'"'))
	{
		SexyChar aQuoteChar = theQuotedString[0];
		std::wstring aLiteralString;
		bool lastWasQuote = true;
		bool lastWasSlash = false;

		for (ulong i = 1; i < theQuotedString.length()-1; i++)
		{
			if (lastWasSlash)
			{
				aLiteralString += theQuotedString[i];
				lastWasSlash = false;
			}
			else
			{
				if (theQuotedString[i] == aQuoteChar)
				{
					if (lastWasQuote)
						aLiteralString += aQuoteChar;

					lastWasQuote = true;
				}
				else if (theQuotedString[i] == L'\\')
				{
					lastWasSlash = true;
					lastWasQuote = false;
				}
				else
				{
					aLiteralString += theQuotedString[i];
					lastWasQuote = false;
				}
			}
		}

		return aLiteralString;
	}
	else
		return theQuotedString;
}

bool DescParser::GetValues(ListDataElement* theSource, ListDataElement* theValues)
{
	theValues->mElementVector.clear();

	for (ulong aSourceNum = 0; aSourceNum < theSource->mElementVector.size(); aSourceNum++)
	{
		if (theSource->mElementVector[aSourceNum]->mIsList)
		{
			ListDataElement* aChildList = new ListDataElement();
			theValues->mElementVector.push_back(aChildList);

			if (!GetValues((ListDataElement*) theSource->mElementVector[aSourceNum], aChildList))
				return false;
		}
		else
		{
			std::wstring aString = ((SingleDataElement*) theSource->mElementVector[aSourceNum])->mString;

			if (aString.length() > 0)
			{
				if ((aString[0] == '\'') || (aString[0] == '"'))
				{
					SingleDataElement* aChildData = new SingleDataElement(Unquote(aString));
					theValues->mElementVector.push_back(aChildData);
				}
				else if (IsImmediate(aString))
				{
					theValues->mElementVector.push_back(new SingleDataElement(aString));
				}
				else
				{
					std::wstring aDefineName = StringToUpper(aString);

					DataElementMap::iterator anItr = mDefineMap.find(aDefineName);

					if (anItr == mDefineMap.end())
					{
						Error(L"Unable to Dereference \"" + aString + L"\"");
						return false;
					}

					theValues->mElementVector.push_back(anItr->second->Duplicate());
				}
			}


		}
	}

	return true;
}

std::wstring DescParser::DataElementToString(const DataElement* theDataElement, bool enclose)
{
	if (theDataElement->mIsList)
	{
		ListDataElement* aListDataElement = (ListDataElement*) theDataElement;

		std::wstring aString = enclose ? L"(" : L"";

		for (ulong i = 0; i < aListDataElement->mElementVector.size(); i++)
		{
			if (i != 0)
				aString += enclose ? L", " : L" ";

			aString += DataElementToString(aListDataElement->mElementVector[i]);
		}

		aString += enclose ? L")" : L"";

		return aString;
	}
	else
	{
		SingleDataElement* aSingleDataElement = (SingleDataElement*) theDataElement;
		if (aSingleDataElement->mValue != NULL)
			return aSingleDataElement->mString + L"=" + DataElementToString(aSingleDataElement->mValue);
		else
			return aSingleDataElement->mString;
	}
}

bool DescParser::DataToString(DataElement* theSource, std::wstring* theString)
{
	*theString = L"";

	if (theSource->mIsList)
		return false;
	if (((SingleDataElement*) theSource)->mValue != NULL)
		return false;

	std::wstring aDefName = ((SingleDataElement*) theSource)->mString;

	DataElement* aDataElement = Dereference(aDefName);

	if (aDataElement != NULL)
	{
		if (aDataElement->mIsList)
			return false;

		*theString = Unquote(((SingleDataElement*) aDataElement)->mString);
	}
	else
		*theString = Unquote(aDefName);

	return true;
}

bool DescParser::DataToKeyAndValue(DataElement* theSource, std::wstring* theKey, DataElement** theValue)
{
	*theKey = L"";

	if (theSource->mIsList)
		return false;
	if (((SingleDataElement*) theSource)->mValue == NULL)
		return false;
	*theValue = ((SingleDataElement*) theSource)->mValue;

	std::wstring aDefName = ((SingleDataElement*) theSource)->mString;

	DataElement* aDataElement = Dereference(aDefName);

	if (aDataElement != NULL)
	{
		if (aDataElement->mIsList)
			return false;

		*theKey = Unquote(((SingleDataElement*) aDataElement)->mString);
	}
	else
		*theKey = Unquote(aDefName);

	return true;
}

bool DescParser::DataToInt(DataElement* theSource, int* theInt)
{
	*theInt = 0;

	std::wstring aTempString;
	if (!DataToString(theSource, &aTempString))
		return false;

	if (!StringToInt(aTempString, theInt))
		return false;

	return true;
}

bool DescParser::DataToDouble(DataElement* theSource, double* theDouble)
{
	*theDouble = 0;

	std::wstring aTempString;
	if (!DataToString(theSource, &aTempString))
		return false;

	if (!StringToDouble(aTempString, theDouble))
		return false;

	return true;
}

bool DescParser::DataToBoolean(DataElement* theSource, bool* theBool)
{
	*theBool = false;

	std::wstring aTempString;
	if (!DataToString(theSource, &aTempString))
		return false;

	if ((wcscmp(aTempString.c_str(), L"false") == 0) ||
	    (wcscmp(aTempString.c_str(), L"no") == 0) ||
	    (wcscmp(aTempString.c_str(), L"0") == 0))
	{
		*theBool = false;
		return true;
	}

	if ((wcscmp(aTempString.c_str(), L"true") == 0) ||
	    (wcscmp(aTempString.c_str(), L"yes") == 0) ||
	    (wcscmp(aTempString.c_str(), L"1") == 0))
	{
		*theBool = true;
		return true;
	}

	return false;
}

bool DescParser::DataToStringVector(DataElement* theSource, WStringVector* theStringVector)
{
	theStringVector->clear();

	ListDataElement aStaticValues;
	ListDataElement* aValues;

	if (theSource->mIsList)
	{
		if (!GetValues((ListDataElement*) theSource, &aStaticValues))
			return false;

		aValues = &aStaticValues;
	}
	else
	{
		std::wstring aDefName = ((SingleDataElement*) theSource)->mString;

		DataElement* aDataElement = Dereference(aDefName);

		if (aDataElement == NULL)
		{
			Error(L"Unable to Dereference \"" + aDefName + L"\"");
			return false;
		}

		if (!aDataElement->mIsList)
			return false;

		aValues = (ListDataElement*) aDataElement;
	}

	for (ulong i = 0; i < aValues->mElementVector.size(); i++)
	{
		if (aValues->mElementVector[i]->mIsList)
		{
			theStringVector->clear();
			return false;
		}

		SingleDataElement* aSingleDataElement = (SingleDataElement*) aValues->mElementVector[i];

		theStringVector->push_back(aSingleDataElement->mString);
	}

	return true;
}

bool DescParser::DataToList(DataElement* theSource, ListDataElement* theValues)
{
	if (theSource->mIsList)
	{
		return GetValues((ListDataElement*) theSource, theValues);
	}

	DataElement* aDataElement = Dereference(((SingleDataElement*) theSource)->mString);

	if ((aDataElement == NULL) || (!aDataElement->mIsList))
		return false;

	ListDataElement* aListElement = (ListDataElement*) aDataElement;

	*theValues = *aListElement;

	return true;
}

bool DescParser::DataToIntVector(DataElement* theSource, IntVector* theIntVector)
{
	theIntVector->clear();

	WStringVector aStringVector;
	if (!DataToStringVector(theSource, &aStringVector))
		return false;

	for (ulong i = 0; i < aStringVector.size(); i++)
	{
		int aIntVal;
		if (!StringToInt(aStringVector[i], &aIntVal))
			return false;

		theIntVector->push_back(aIntVal);
	}

	return true;
}

bool DescParser::DataToDoubleVector(DataElement* theSource, DoubleVector* theDoubleVector)
{
	theDoubleVector->clear();

	WStringVector aStringVector;
	if (!DataToStringVector(theSource, &aStringVector))
		return false;

	for (ulong i = 0; i < aStringVector.size(); i++)
	{
		double aDoubleVal;
		if (!StringToDouble(aStringVector[i], &aDoubleVal))
			return false;

		theDoubleVector->push_back(aDoubleVal);
	}

	return true;
}

bool DescParser::ParseToList(const std::wstring& theString, ListDataElement* theList, bool expectListEnd, int* theStringPos)
{
	bool inSingleQuotes = false;
	bool inDoubleQuotes = false;
	bool escaped = false;
	bool wantTerminateSingleDataElement = false;

	SingleDataElement* aKeySingleDataElement = NULL;
	SingleDataElement* aCurSingleDataElement = NULL;

	int aStringPos = 0;

	if (theStringPos == NULL)
		theStringPos = &aStringPos;

	while (*theStringPos < (int) theString.length())
	{
		bool addSingleChar = false;
		wchar_t aChar = theString[(*theStringPos)++];

		bool isSeperator = (aChar == L' ') || (aChar == L'\t') ||
			(aChar == L'\n') || (aChar == L',');

		if (escaped)
		{
			addSingleChar = true;
		}
		else
		{
			if ((aChar == L'\'') && (!inDoubleQuotes))
				inSingleQuotes = !inSingleQuotes;
			else if ((aChar == L'"') && (!inSingleQuotes))
				inDoubleQuotes = !inDoubleQuotes;

			if (aChar == L'\\')
			{
				escaped = true;
			}
			else if ((!inSingleQuotes) && (!inDoubleQuotes))
			{
				if (aChar == L')')
				{
					if (expectListEnd)
						return true;
					else
					{
						Error(L"Unexpected List End");
						return false;
					}
				}
				else if (aChar == L'(')
				{
					if (wantTerminateSingleDataElement)
					{
						aCurSingleDataElement = NULL;
						wantTerminateSingleDataElement = false;
					}

					if (aCurSingleDataElement != NULL)
					{
						Error(L"Unexpected List Start");
						return false;
					}

					ListDataElement* aChildList = new ListDataElement();

					if (!ParseToList(theString, aChildList, true, theStringPos))
						return false;

					if (aKeySingleDataElement != NULL)
					{
						aKeySingleDataElement->mValue = aChildList;
						aKeySingleDataElement = NULL;
					}
					else
						theList->mElementVector.push_back(aChildList);
				}
				else if (aChar == L'=')
				{
					// Make it a value
					aKeySingleDataElement = aCurSingleDataElement;
					/*SingleDataElement* aNewSingleDataElement = new SingleDataElement();
					aCurSingleDataElement->mValue = aNewSingleDataElement;
					aCurSingleDataElement = aNewSingleDataElement;
					wantTerminateSingleDataElement = false;*/

					wantTerminateSingleDataElement = true;
				}
				else if (isSeperator)
				{
					if ((aCurSingleDataElement != NULL) && (aCurSingleDataElement->mString.length() > 0))
						wantTerminateSingleDataElement = true;
				}
				else
				{
					if (wantTerminateSingleDataElement)
					{
						aCurSingleDataElement = NULL;
						wantTerminateSingleDataElement = false;
					}
					addSingleChar = true;
				}
			}
			else
			{
				if (wantTerminateSingleDataElement)
				{
					aCurSingleDataElement = NULL;
					wantTerminateSingleDataElement = false;
				}
				addSingleChar = true;
			}
		}

		if (addSingleChar)
		{
			if (aCurSingleDataElement == NULL)
			{
				aCurSingleDataElement = new SingleDataElement();
				if (aKeySingleDataElement != NULL)
				{
					aKeySingleDataElement->mValue = aCurSingleDataElement;
					aKeySingleDataElement = NULL;
				}
				else
					theList->mElementVector.push_back(aCurSingleDataElement);
			}

			if (escaped)
			{
				aCurSingleDataElement->mString += L"\\";
				escaped = false;
			}

			aCurSingleDataElement->mString += aChar;
		}
	}

	if (inSingleQuotes)
	{
		Error(L"Unterminated Single Quotes");
		return false;
	}

	if (inDoubleQuotes)
	{
		Error(L"Unterminated Double Quotes");
		return false;
	}

	if (expectListEnd)
	{
		Error(L"Unterminated List");
		return false;
	}

	return true;
}

bool DescParser::ParseDescriptorLine(const std::wstring& theDescriptorLine)
{
	ListDataElement aParams;
	if (!ParseToList(theDescriptorLine, &aParams, false, NULL))
		return false;

	if (aParams.mElementVector.size() > 0)
	{
		if (aParams.mElementVector[0]->mIsList)
		{
			Error(L"Missing Command");
			return false;
		}

		if (!HandleCommand(aParams))
			return false;
	}

	return true;
}

bool DescParser::LoadDescriptor(const std::string& theFileName)
{
	mCurrentLineNum = 0;
	int aLineCount = 0;
	bool hasErrors = false;

	//Apparently VC6 doesn't have a clear() function for basic_strings
	//mError.clear();
	mError.erase();
	mCurrentLine = L"";

	if (!EncodingParser::OpenFile(theFileName))
		return Error(L"Unable to open file: " + StringToWString(theFileName));

	while (!EndOfFile())
	{
		wchar_t aChar;

		bool skipLine = false;
		bool atLineStart = true;
		bool inSingleQuotes = false;
		bool inDoubleQuotes = false;
		bool escaped = false;
		bool isIndented = false;

		for (;;)
		{
			EncodingParser::GetCharReturnType aResult = GetChar(&aChar);
			if (aResult == END_OF_FILE)
				break;

			if (aResult == INVALID_CHARACTER)
				return Error(L"Invalid Character");
			if (aResult != SUCCESSFUL)
				return Error(L"Internal Error");

			if (aChar != L'\r')
			{
				if (aChar == L'\n')
					aLineCount++;

				if (((aChar == L' ') || (aChar == L'\t')) && (atLineStart))
					isIndented = true;

				if ((!atLineStart) || ((aChar != L' ') && (aChar != L'\t') && (aChar != L'\n')))
				{
					if (atLineStart)
					{
						if ((mCmdSep & CMDSEP_NO_INDENT) && (!isIndented) && (mCurrentLine.size() > 0))
						{
							// Start a new non-indented line
							PutChar(aChar);
							break;
						}

						if (aChar == L'#')
							skipLine = true;

						atLineStart = false;
					}

					if (aChar == L'\n')
					{
						isIndented = false;
						atLineStart = true;
					}

					if ((aChar == L'\n') && (skipLine))
					{
						skipLine = false;
					}
					else if (!skipLine)
					{
						if (aChar == L'\\' && (inSingleQuotes || inDoubleQuotes) && !escaped)
							escaped = true;
						else
						{
							if ((aChar == L'\'') && (!inDoubleQuotes) && (!escaped))
								inSingleQuotes = !inSingleQuotes;

							if ((aChar == L'"') && (!inSingleQuotes) && (!escaped))
								inDoubleQuotes = !inDoubleQuotes;

							if ((aChar == L';') && (mCmdSep & CMDSEP_SEMICOLON) && (!inSingleQuotes) && (!inDoubleQuotes))
								break;

							if(escaped) // stay escaped for when this is actually parsed
							{
								mCurrentLine += L'\\';
								escaped = false;
							}

							if (mCurrentLine.size() == 0)
								mCurrentLineNum = aLineCount + 1;

							mCurrentLine += aChar;
						}
					}
				}
			}
		}

		if (mCurrentLine.length() > 0)
		{
			if (!ParseDescriptorLine(mCurrentLine))
			{
				hasErrors = true;
				break;
			}

			//Apparently VC6 doesn't have a clear() function for basic_strings
			//mCurrentLine.clear();
			mCurrentLine.erase();
		}
	}

	//Apparently VC6 doesn't have a clear() function for basic_strings
	//mCurrentLine.clear();
	mCurrentLine.erase();
	mCurrentLineNum = 0;

	CloseFile();
	return !hasErrors;
}
