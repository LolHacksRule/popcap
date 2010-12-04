#ifndef _ENCODINGPARSER_H_
#define _ENCODINGPARSER_H_

#include "Common.h"

struct PFILE;

namespace Sexy
{

typedef std::vector<unichar_t> WcharBuffer;

class EncodingParser
{
protected:
	PFILE*					mFile;

private:
	WcharBuffer				mBufferedText;
	bool					(EncodingParser::*mGetCharFunc)(unichar_t* theChar, bool* error);
	bool					mForcedEncodingType;
	bool					mFirstChar;
	bool					mByteSwap;

private:
	bool					GetAsciiChar(unichar_t* theChar, bool* error);
	bool					GetUTF8Char(unichar_t* theChar, bool* error);
	bool					GetUTF16Char(unichar_t* theChar, bool* error);
	bool					GetUTF16LEChar(unichar_t* theChar, bool* error);
	bool					GetUTF16BEChar(unichar_t* theChar, bool* error);

public:
	enum EncodingType
	{
		ASCII,
		UTF_8,
		UTF_16,
		UTF_16_LE,
		UTF_16_BE
	};

	enum GetCharReturnType
	{
		SUCCESSFUL,
		INVALID_CHARACTER,
		END_OF_FILE,
		FAILURE				// general case failures
	};

public:
	EncodingParser();
	virtual ~EncodingParser();

	virtual void			SetEncodingType(EncodingType theEncoding);
	virtual bool			OpenFile(const std::string& theFilename);
	virtual bool			CloseFile();
	virtual bool			EndOfFile();
	virtual void			SetStringSource(const Sexy::WString& theString);
	void					SetStringSource(const std::string& theString);

	virtual GetCharReturnType GetChar(unichar_t* theChar);
	virtual bool			PutChar(const unichar_t& theChar);
	virtual bool			PutString(const Sexy::WString& theString);
};

};

#endif // #ifndef _ENCODINGPARSER_H_
