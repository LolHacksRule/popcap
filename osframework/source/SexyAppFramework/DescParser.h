#ifndef __DESCPARSER_H__
#define __DESCPARSER_H__

#include "Common.h"
#include "EncodingParser.h"

namespace Sexy
{

class DataElement
{
public:
	bool					mIsList;

public:
	DataElement();
	virtual ~DataElement();

	virtual DataElement*	Duplicate() = 0;
};

class SingleDataElement : public DataElement
{
public:
	std::wstring				mString;
	DataElement*			mValue;

public:
	SingleDataElement();
	SingleDataElement(const std::wstring& theString);
	virtual ~SingleDataElement();

	virtual DataElement*	Duplicate();
};

typedef std::vector<DataElement*> ElementVector;

class ListDataElement : public DataElement
{
public:
	ElementVector			mElementVector;

public:
	ListDataElement();
	ListDataElement(const ListDataElement& theListDataElement);
	virtual ~ListDataElement();

	ListDataElement&		operator=(const ListDataElement& theListDataElement);

	virtual DataElement*	Duplicate();
};

typedef std::map<std::wstring, DataElement*> DataElementMap;
typedef std::vector<std::wstring> WStringVector;
typedef std::vector<int> IntVector;
typedef std::vector<double> DoubleVector;

class DescParser : public EncodingParser
{
public:
	enum
	{
		CMDSEP_SEMICOLON = 1,
		CMDSEP_NO_INDENT = 2
	};

public:
	int						mCmdSep;

	std::wstring				mError;
	int						mCurrentLineNum;
	std::wstring			mCurrentLine;
	DataElementMap			mDefineMap;

public:
	virtual bool			Error(const std::wstring& theError);
	virtual DataElement*	Dereference(const std::wstring& theString);
	bool					IsImmediate(const std::wstring& theString);
	std::wstring				Unquote(const std::wstring& theQuotedString);
	bool					GetValues(ListDataElement* theSource, ListDataElement* theValues);
	std::wstring				DataElementToString(const DataElement* theDataElement, bool enclose = true);
	bool					DataToString(DataElement* theSource, std::wstring* theString);
	bool					DataToKeyAndValue(DataElement* theSource, std::wstring* theKey, DataElement** theValue);
	bool					DataToInt(DataElement* theSource, int* theInt);
	bool					DataToDouble(DataElement* theSource, double* theDouble);
	bool					DataToBoolean(DataElement* theSource, bool* theBool);
	bool					DataToStringVector(DataElement* theSource, WStringVector* theStringVector);
	bool					DataToList(DataElement* theSource, ListDataElement* theValues);
	bool					DataToIntVector(DataElement* theSource, IntVector* theIntVector);
	bool					DataToDoubleVector(DataElement* theSource, DoubleVector* theDoubleVector);
	bool					ParseToList(const std::wstring& theString, ListDataElement* theList, bool expectListEnd, int* theStringPos);
	bool					ParseDescriptorLine(const std::wstring& theDescriptorLine);

	// You must implement this one
	virtual bool			HandleCommand(const ListDataElement& theParams) = 0;

public:
	DescParser();
	virtual ~DescParser();

	virtual bool			LoadDescriptor(const std::string& theFileName);
};

}

#endif //__DESCPARSER_H__
