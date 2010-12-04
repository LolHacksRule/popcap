#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__

#include "Common.h"

#include "PerfTimer.h"

struct PFILE;

namespace Sexy
{

class XMLParam
{
public:
	std::string				mKey;
	std::string				mValue;
};

typedef std::map<SexyString, SexyString>	XMLParamMap;
typedef std::list<XMLParamMap::iterator>	XMLParamMapIteratorList;

typedef std::vector<unichar_t> XMLParserBuffer;

class XMLElement
{
public:
	enum
	{
		TYPE_NONE,
		TYPE_START,
		TYPE_END,
		TYPE_ELEMENT,
		TYPE_INSTRUCTION,
		TYPE_COMMENT,
		TYPE_CDATA
	};
public:

	int					mType;
	SexyString				mSection;
	SexyString				mValue;
	SexyString				mInstruction;
	XMLParamMap				mAttributes;
	XMLParamMapIteratorList	mAttributeIteratorList; // stores attribute iterators in their original order
};

class XMLParser
{
protected:
	std::string				mFileName;
	SexyString				mErrorText;
	int					mLineNum;
	PFILE*					mFile;
	bool					mHasFailed;
	bool					mAllowComments;
	XMLParserBuffer			        mBufferedText;
	SexyString				mSection;
	bool					(XMLParser::*mGetCharFunc)(unichar_t* theChar, bool* error);
	bool					mForcedEncodingType;
	bool					mFirstChar;
	bool					mByteSwap;

protected:
	void					Fail(const SexyString& theErrorText);
	void					Init();

	bool					AddAttribute(XMLElement* theElement,
							     const SexyString& aAttributeKey,
							     const SexyString& aAttributeValue);
	bool					AddAttribute(XMLElement* theElement,
							     const Sexy::WString& aAttributeKey,
							     const Sexy::WString& aAttributeValue);

	bool					GetAsciiChar(unichar_t* theChar, bool* error);
	bool					GetUTF8Char(unichar_t* theChar, bool* error);
	bool					GetUTF16Char(unichar_t* theChar, bool* error);
	bool					GetUTF16LEChar(unichar_t* theChar, bool* error);
	bool					GetUTF16BEChar(unichar_t* theChar, bool* error);

public:
	enum XMLEncodingType
	{
		ASCII,
		UTF_8,
		UTF_16,
		UTF_16_LE,
		UTF_16_BE
	};

public:
	XMLParser();
	virtual ~XMLParser();

	void					SetEncodingType(XMLEncodingType theEncoding);
	bool					OpenFile(const std::string& theFilename);
	void					SetStringSource(const Sexy::WString& theString);
	void					SetStringSource(const std::string& theString);
	bool					NextElement(XMLElement* theElement);
	SexyString				GetErrorText();
	int						GetCurrentLineNum();
	std::string				GetFileName();

	inline void				AllowComments(bool doAllow) { mAllowComments = doAllow; }

	bool					HasFailed();
	bool					EndOfFile();
};

};

#endif //__XMLPARSER_H__
