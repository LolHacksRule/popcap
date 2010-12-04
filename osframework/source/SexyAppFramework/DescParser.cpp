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

SingleDataElement::SingleDataElement(const Sexy::WString& theString) :
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

bool DescParser::Error(const Sexy::WString& theError)
{
	if (mError.length() == 0)
		mError = theError;
	return false;
}

DataElement* DescParser::Dereference(const Sexy::WString& theString)
{
	Sexy::WString aDefineName = StringToUpper(theString);

	DataElementMap::iterator anItr = mDefineMap.find(aDefineName);
	if (anItr != mDefineMap.end())
		return anItr->second;
	else
		return NULL;
}

bool DescParser::IsImmediate(const Sexy::WString& theString)
{
	return (((theString[0] >= L'0') && (theString[0] <= L'9')) || (theString[0] == L'-') ||
		(theString[0] == L'+') || (theString[0] == L'\'') || (theString[0] == L'"'));
}

Sexy::WString DescParser::Unquote(const Sexy::WString& theQuotedString)
{
	if ((theQuotedString[0] == L'\'') || (theQuotedString[0] == L'"'))
	{
		SexyChar aQuoteChar = theQuotedString[0];
		Sexy::WString aLiteralString;
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
			Sexy::WString aString = ((SingleDataElement*) theSource->mElementVector[aSourceNum])->mString;

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
					Sexy::WString aDefineName = StringToUpper(aString);

					DataElementMap::iterator anItr = mDefineMap.find(aDefineName);

					if (anItr == mDefineMap.end())
					{
						Error(WSTR("Unable to Dereference \"") + aString + WSTR("\""));
						return false;
					}

					theValues->mElementVector.push_back(anItr->second->Duplicate());
				}
			}


		}
	}

	return true;
}

Sexy::WString DescParser::DataElementToString(const DataElement* theDataElement, bool enclose)
{
	if (theDataElement->mIsList)
	{
		ListDataElement* aListDataElement = (ListDataElement*) theDataElement;

		Sexy::WString aString = enclose ? WSTR("(") : WSTR("");

		for (ulong i = 0; i < aListDataElement->mElementVector.size(); i++)
		{
			if (i != 0)
				aString += enclose ? WSTR(L", ") : WSTR(L" ");

			aString += DataElementToString(aListDataElement->mElementVector[i]);
		}

		aString += enclose ? WSTR(L")") : WSTR(L"");

		return aString;
	}
	else
	{
		SingleDataElement* aSingleDataElement = (SingleDataElement*) theDataElement;
		if (aSingleDataElement->mValue != NULL)
			return aSingleDataElement->mString + WSTR(L"=") + DataElementToString(aSingleDataElement->mValue);
		else
			return aSingleDataElement->mString;
	}
}

bool DescParser::DataToString(DataElement* theSource, Sexy::WString* theString)
{
	theString->clear();

	if (theSource->mIsList)
		return false;
	if (((SingleDataElement*) theSource)->mValue != NULL)
		return false;

	Sexy::WString aDefName = ((SingleDataElement*) theSource)->mString;

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

bool DescParser::DataToKeyAndValue(DataElement* theSource, Sexy::WString* theKey, DataElement** theValue)
{
	theKey->clear();

	if (theSource->mIsList)
		return false;
	if (((SingleDataElement*) theSource)->mValue == NULL)
		return false;
	*theValue = ((SingleDataElement*) theSource)->mValue;

	Sexy::WString aDefName = ((SingleDataElement*) theSource)->mString;

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

	Sexy::WString aTempString;
	if (!DataToString(theSource, &aTempString))
		return false;

	if (!StringToInt(aTempString, theInt))
		return false;

	return true;
}

bool DescParser::DataToDouble(DataElement* theSource, double* theDouble)
{
	*theDouble = 0;

	Sexy::WString aTempString;
	if (!DataToString(theSource, &aTempString))
		return false;

	if (!StringToDouble(aTempString, theDouble))
		return false;

	return true;
}

bool DescParser::DataToBoolean(DataElement* theSource, bool* theBool)
{
	*theBool = false;

	Sexy::WString aTempString;
	if (!DataToString(theSource, &aTempString))
		return false;

	inlineLower(aTempString);
	if (aTempString == WSTR("false") ||
	    aTempString == WSTR("no") ||
	    aTempString == WSTR("0"))
	{
		*theBool = false;
		return true;
	}

	if (aTempString == WSTR("true") ||
	    aTempString == WSTR("yes") ||
	    aTempString == WSTR("1"))
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
		Sexy::WString aDefName = ((SingleDataElement*) theSource)->mString;

		DataElement* aDataElement = Dereference(aDefName);

		if (aDataElement == NULL)
		{
			Error(WSTR("Unable to Dereference \"") + aDefName + WSTR("\""));
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

bool DescParser::ParseToList(const Sexy::WString& theString, ListDataElement* theList, bool expectListEnd, int* theStringPos)
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
		unichar_t aChar = theString[(*theStringPos)++];

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
						Error(WSTR("Unexpected List End"));
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
						Error(WSTR("Unexpected List Start"));
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
				aCurSingleDataElement->mString += WSTR("\\");
				escaped = false;
			}

			aCurSingleDataElement->mString += aChar;
		}
	}

	if (inSingleQuotes)
	{
		Error(WSTR("Unterminated Single Quotes"));
		return false;
	}

	if (inDoubleQuotes)
	{
		Error(WSTR("Unterminated Double Quotes"));
		return false;
	}

	if (expectListEnd)
	{
		Error(WSTR("Unterminated List"));
		return false;
	}

	return true;
}

bool DescParser::ParseDescriptorLine(const Sexy::WString& theDescriptorLine)
{
	ListDataElement aParams;
	if (!ParseToList(theDescriptorLine, &aParams, false, NULL))
		return false;

	if (aParams.mElementVector.size() > 0)
	{
		if (aParams.mElementVector[0]->mIsList)
		{
			Error(WSTR("Missing Command"));
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
	mCurrentLine.clear();

	if (!EncodingParser::OpenFile(theFileName))
		return Error(WSTR("Unable to open file: ") + Sexy::WString(theFileName.begin(), theFileName.end()));

	while (!EndOfFile())
	{
		unichar_t aChar;

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
				return Error(WSTR("Invalid Character"));
			if (aResult != SUCCESSFUL)
				return Error(WSTR("Internal Error"));

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
