#include "Font.h"
#include "Image.h"

using namespace Sexy;

Font::Font()
{
	mAscent = 0;
	mHeight = 0;
	mAscentPadding = 0;
	mLineSpacingOffset = 0;
	mSupportUnicode = false;
	mDrawShadow = false;
	mSimulateBold = false;
	mOutLine = false;
}

Font::Font(const Font& theFont) :
	mAscent(theFont.mAscent),
	mAscentPadding(theFont.mAscentPadding),
	mHeight(theFont.mHeight),
	mLineSpacingOffset(theFont.mLineSpacingOffset),
	mSupportUnicode(theFont.mSupportUnicode),
	mDrawShadow(theFont.mDrawShadow),
	mSimulateBold(theFont.mSimulateBold),
	mOutLine(theFont.mOutLine)
{
}

Font::~Font()
{
}

bool    Font::IsSupportUnicode()
{
	return mSupportUnicode;
}

int	Font::GetAscent()
{
	return mAscent;
}

int	Font::GetAscentPadding()
{
	return mAscentPadding;
}

int	Font::GetDescent()
{
	return mHeight - mAscent;
}

int	Font::GetHeight()
{
	return mHeight;
}

int Font::GetLineSpacingOffset()
{
	return mLineSpacingOffset;
}

int Font::GetLineSpacing()
{
	return mHeight + mLineSpacingOffset;
}

int Font::StringWidth(const std::string& theString)
{
	return 0;
}

int Font::StringWidth(const std::wstring& theString)
{
	return 0;
}

int Font::CharWidth(int theChar)
{
	SexyString aString(1, theChar);
	return StringWidth(aString);
}

int Font::CharWidthKern(int theChar, int thePrevChar)
{
	return CharWidth(theChar);
}

void Font::DrawString(Graphics* g, int theX, int theY, const std::string& theString,
		      const Color& theColor, const Rect& theClipRect, bool unicode)
{
}

void Font::DrawString(Graphics* g, int theX, int theY, const std::wstring& theString,
		      const Color& theColor, const Rect& theClipRect)
{
}
