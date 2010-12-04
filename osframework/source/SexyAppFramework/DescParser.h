#ifndef __DESCPARSER_H__
#define __DESCPARSER_H__

#include "Common.h"
#include "EncodingParser.h"

namespace Sexy
{

class DataElement
{
public:
	bool				mIsList;

public:
	DataElement();
	virtual ~DataElement();

	virtual DataElement*	Duplicate() = 0;
};

class SingleDataElement : public DataElement
{
public:
	Sexy::WString			mString;
	DataElement*			mValue;

public:
	SingleDataElement();
	SingleDataElement(const Sexy::WString& theString);
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

typedef std::map<Sexy::WString, DataElement*> DataElementMap;
typedef std::vector<Sexy::WString> WStringVector;
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
	int					mCmdSep;

	Sexy::WString				mError;
	int					mCurrentLineNum;
	Sexy::WString			        mCurrentLine;
	DataElementMap			        mDefineMap;

public:
	virtual bool			        Error(const Sexy::WString& theError);
	virtual DataElement*	                Dereference(const Sexy::WString& theString);
	bool					IsImmediate(const Sexy::WString& theString);
	Sexy::WString				Unquote(const Sexy::WString& theQuotedString);
	bool					GetValues(ListDataElement* theSource, ListDataElement* theValues);
	Sexy::WString				DataElementToString(const DataElement* theDataElement, bool enclose = true);
	bool					DataToString(DataElement* theSource, Sexy::WString* theString);
	bool					DataToKeyAndValue(DataElement* theSource, Sexy::WString* theKey,
								  DataElement** theValue);
	bool					DataToInt(DataElement* theSource, int* theInt);
	bool					DataToDouble(DataElement* theSource, double* theDouble);
	bool					DataToBoolean(DataElement* theSource, bool* theBool);
	bool					DataToStringVector(DataElement* theSource, WStringVector* theStringVector);
	bool					DataToList(DataElement* theSource, ListDataElement* theValues);
	bool					DataToIntVector(DataElement* theSource, IntVector* theIntVector);
	bool					DataToDoubleVector(DataElement* theSource, DoubleVector* theDoubleVector);
	bool					ParseToList(const Sexy::WString& theString, ListDataElement* theList,
							    bool expectListEnd, int* theStringPos);
	bool					ParseDescriptorLine(const Sexy::WString& theDescriptorLine);

	// You must implement this one
	virtual bool			        HandleCommand(const ListDataElement& theParams) = 0;

public:
	DescParser();
	virtual ~DescParser();

	virtual bool			        LoadDescriptor(const std::string& theFileName);
};

}

#endif //__DESCPARSER_H__
