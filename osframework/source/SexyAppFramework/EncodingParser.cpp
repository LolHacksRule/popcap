#include "EncodingParser.h"
#include "SexyUtf8.h"
#include "Debug.h"
#include "../PakLib/PakInterface.h"

using namespace Sexy;

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
EncodingParser::EncodingParser()
{
	mFile = NULL;
	mGetCharFunc = &EncodingParser::GetUTF8Char;
	mForcedEncodingType = false;
	mFirstChar = false;
	mByteSwap = false;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
EncodingParser::~EncodingParser()
{
	if (mFile != NULL)
		p_fclose(mFile);
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void EncodingParser::SetEncodingType(EncodingType theEncoding)
{
	switch (theEncoding)
	{
		case ASCII:	mGetCharFunc = &EncodingParser::GetAsciiChar;	mForcedEncodingType = true; break;
		case UTF_8:	mGetCharFunc = &EncodingParser::GetUTF8Char;	mForcedEncodingType = true; break;
		case UTF_16:	mGetCharFunc = &EncodingParser::GetUTF16Char;	mForcedEncodingType = true; break;
		case UTF_16_LE:	mGetCharFunc = &EncodingParser::GetUTF16LEChar;	mForcedEncodingType = true; break;
		case UTF_16_BE:	mGetCharFunc = &EncodingParser::GetUTF16BEChar;	mForcedEncodingType = true; break;
	}
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::GetAsciiChar(unichar_t* theChar, bool* error)
{
	unsigned char aChar = 0;
	if (p_fread(&aChar, 1, 1, mFile) != 1)
		return false;

	*theChar = static_cast<unichar_t>(aChar);
	if (aChar > 127)
		;
	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::GetUTF8Char(unichar_t* theChar, bool* error)
{
	static const unsigned short aMaskData[] = {
		0xC0,		// 1 extra byte
		0xE0,		// 2 extra bytes
		0xF0,		// 3 extra bytes
		0xF8,		// 4 extra bytes
		0xFC		// 5 extra bytes
	};
	*error = true;

	int aTempChar = 0;
	unsigned char aTempCharX = 0;
	if (p_fread(&aTempCharX, 1, 1, mFile) == 1)
	{
		aTempChar = aTempCharX; // this prevents endian-ness issues.
		if ((aTempChar & 0x80) != 0)
		{
			if ((aTempChar & 0xC0) != 0xC0) return false; // sanity check: high bit should not be set without the next highest bit being set too.

			int aBytesRead[6];
			int* aBytesReadPtr = &aBytesRead[0];

			*aBytesReadPtr++ = aTempChar;

			int aLen;
			for (aLen = 0; aLen < (int)(sizeof(aMaskData)/sizeof(*aMaskData)); ++aLen)
			{
				if ( (aTempChar & aMaskData[aLen]) == ((aMaskData[aLen] << 1) & aMaskData[aLen]) ) break;
			}
			if (aLen >= (int)(sizeof(aMaskData)/sizeof(*aMaskData))) return false;

			aTempChar &= ~aMaskData[aLen];
			int aTotalLen = aLen+1;

			DBG_ASSERTE(aTotalLen >= 2 && aTotalLen <= 6);

			int anExtraChar = 0;
			while (aLen > 0)
			{
				if (p_fread(&aTempCharX, 1, 1, mFile) != 1) return false;
				anExtraChar = aTempCharX;
				if ((anExtraChar & 0xC0) != 0x80) return false; // sanity check: high bit set, and next highest bit NOT set.

				*aBytesReadPtr++ = anExtraChar;

				aTempChar = (aTempChar << 6) | (anExtraChar & 0x3F);
				--aLen;
			}

			// validate substrings
			bool valid = true;
			switch (aTotalLen)
			{
				case 2:
					valid = !((aBytesRead[0] & 0x3E) == 0);
					break;
				case 3:
					valid = !((aBytesRead[0] & 0x1F) == 0 && (aBytesRead[1] & 0x20) == 0);
					break;
				case 4:
					valid = !((aBytesRead[0] & 0x0F) == 0 && (aBytesRead[1] & 0x30) == 0);
					break;
				case 5:
					valid = !((aBytesRead[0] & 0x07) == 0 && (aBytesRead[1] & 0x38) == 0);
					break;
				case 6:
					valid = !((aBytesRead[0] & 0x03) == 0 && (aBytesRead[1] & 0x3C) == 0);
					break;
			}
			if (!valid) return false;
		}

		if ( (aTempChar >= 0xD800 && aTempChar <= 0xDFFF) || (aTempChar >= 0xFFFE && aTempChar <= 0xFFFF) ) return false;

		if (aTempChar == 0xFEFF && mFirstChar) // zero-width non breaking space as the first char is a byte order marker.
		{
			mFirstChar = false;
			return GetUTF8Char(theChar, error);
		}

		*theChar = (unichar_t)aTempChar;
		*error = false;
		return true;
	}

	*error = false;
	return false;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::GetUTF16Char(unichar_t* theChar, bool* error)
{
	unichar_t aTempChar = 0;
	if (p_fread(&aTempChar, 2, 1, mFile) != 1)
		return false;

	if (mFirstChar)
	{
		mFirstChar = false;
		if (aTempChar == 0xFEFF)
		{
			mByteSwap = false;
			return GetUTF16Char(theChar, error);
		}
		else if (aTempChar == 0xFFFE)
		{
			mByteSwap = true;
			return GetUTF16Char(theChar, error);
		}
	}
	if (mByteSwap) aTempChar = (unichar_t)(((aTempChar << 8) & 0xFF00) | ((aTempChar >> 8) & 0xFF)); // ands remove assumption that unichar_t = 16 bits.

	if ((aTempChar & 0xFC00) == 0xD800)
	{
		*error = true;

		unichar_t aNextChar = 0;
		if (p_fread(&aNextChar, 2, 1, mFile) != 1)
			return false;

		if (mByteSwap) aNextChar = (unichar_t)(((aNextChar << 8) & 0xFF00) | ((aNextChar >> 8) & 0xFF));
		if ((aNextChar & 0xFC00) == 0xDC00)
		{
			*theChar = (unichar_t)((((aTempChar & 0x3FF) << 10) | (aNextChar & 0x3FF)) + 0x10000);
		}
		else return false;
	}
	else *theChar = aTempChar;

	*error = false;
	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::GetUTF16LEChar(unichar_t* theChar, bool* error)
{
	unichar_t aTempChar = 0;
	if (p_fread(&aTempChar, 2, 1, mFile) != 1)
		return false;

	aTempChar = WORD_LITTLEE_TO_NATIVE(aTempChar);

	if ((aTempChar & 0xFC00) == 0xD800)
	{
		*error = true;

		unichar_t aNextChar = 0;
		if (p_fread(&aNextChar, 2, 1, mFile) != 1)
			return false;

		aNextChar = WORD_LITTLEE_TO_NATIVE(aTempChar);
		if ((aNextChar & 0xFC00) == 0xDC00)
		{
			*theChar = (unichar_t)((((aTempChar & 0x3FF) << 10) | (aNextChar & 0x3FF)) + 0x10000);
		}
		else return false;
	}

	*error = false;
	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::GetUTF16BEChar(unichar_t* theChar, bool* error)
{
	unichar_t aTempChar = 0;
	if (p_fread(&aTempChar, 2, 1, mFile) != 1)
		return false;

	aTempChar = WORD_BIGE_TO_NATIVE(aTempChar);

	if ((aTempChar & 0xFC00) == 0xD800)
	{
		*error = true;

		unichar_t aNextChar = 0;
		if (p_fread(&aNextChar, 2, 1, mFile) != 1)
			return false;

		aNextChar = WORD_BIGE_TO_NATIVE(aTempChar);
		if ((aNextChar & 0xFC00) == 0xDC00)
		{
			*theChar = (unichar_t)((((aTempChar & 0x3FF) << 10) | (aNextChar & 0x3FF)) + 0x10000);
		}
		else return false;
	}

	*error = false;
	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::OpenFile(const std::string& theFileName)
{
	mFile = p_fopen(theFileName.c_str(), "r");

	if (mFile == NULL)
		return false;

	if (!mForcedEncodingType) // Let's try to detect the encoding type using BOMs
	{
		char buffer[8192];
		long len;

		p_fseek(mFile, 0, SEEK_END);
		long aFileLen = p_ftell(mFile);
		p_fseek(mFile, 0, SEEK_SET);

		len = std::min(8192L, aFileLen);
		len = p_fread(buffer, 1, len, mFile);
		p_fseek(mFile, 0, SEEK_SET);

		mGetCharFunc = &EncodingParser::GetAsciiChar;
		if (aFileLen >= 2) // UTF-16?
		{
			int aChar1 = buffer[0];
			int aChar2 = buffer[1];

			if ( (aChar1 == 0xFF && aChar2 == 0xFE) || (aChar1 == 0xFE && aChar2 == 0xFF) )
				mGetCharFunc = &EncodingParser::GetUTF16Char;
		}
		if (mGetCharFunc == &EncodingParser::GetAsciiChar)
		{
                       if (aFileLen >= 3) // UTF-8?
                       {

				int aChar1 = buffer[0];
				int aChar2 = buffer[1];
				int aChar3 = buffer[2];

				// check BOM
				if (aChar1 == 0xEF && aChar2 == 0xBB && aChar3 == 0xBF)
				{
					mGetCharFunc = &EncodingParser::GetUTF8Char;
				}
				else
				{
					for (size_t i = 0; i < 6 && len - i >= 0; i++)
					{
						if (SexyUtf8Validate(buffer, len - i))
						{
							mGetCharFunc = &EncodingParser::GetUTF8Char;
							break;
						}
					}
				}
			}
		}
	}

	mFirstChar = true;

	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::CloseFile()
{
	if (mFile != NULL)
	{
		p_fclose(mFile);
		mFile = NULL;
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::EndOfFile()
{
	if (mBufferedText.size() > 0)
		return false;

	if (mFile != NULL)
		return p_feof(mFile) != 0;

	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void EncodingParser::SetStringSource(const Sexy::WString& theString)
{
	int aSize = theString.size();

	mBufferedText.resize(aSize);
	for (int i = 0; i < aSize; i++)
		mBufferedText[i] = theString[aSize - i - 1];
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void EncodingParser::SetStringSource(const std::string& theString)
{
	SetStringSource(WSTR(theString.c_str()));
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
EncodingParser::GetCharReturnType EncodingParser::GetChar(unichar_t* theChar)
{
	if (theChar == NULL)
		return FAILURE;

	if (mBufferedText.size() != 0)
	{
		*theChar = mBufferedText.back();
		mBufferedText.pop_back();
		return SUCCESSFUL;
	}
	else
	{
		if (mFile == NULL || p_feof(mFile))
			return END_OF_FILE;

		bool error = false;
		if ((this->*mGetCharFunc)(theChar, &error))
			return SUCCESSFUL;

		if (error)
			return INVALID_CHARACTER;
		else
			return END_OF_FILE;
	}

	DBG_ASSERTE(0); // not reached
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::PutChar(const unichar_t& theChar)
{
	mBufferedText.push_back(theChar);
	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
bool EncodingParser::PutString(const Sexy::WString& theString)
{
	WcharBuffer::size_type aCurSize = mBufferedText.size();
	WcharBuffer::size_type anAddSize = WcharBuffer::size_type(theString.length());

	mBufferedText.resize(aCurSize + anAddSize);
	std::copy(theString.rbegin(), theString.rend(), mBufferedText.begin() + aCurSize);
	return true;
}
