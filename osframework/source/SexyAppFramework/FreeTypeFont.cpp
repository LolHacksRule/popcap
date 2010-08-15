#include "FreeTypeFont.h"
#include "FreeTypeFontMap.h"
#include "SexyAppBase.h"
#include "MemoryImage.h"
#include "Graphics.h"
#include "SexyUtf8.h"

#include "ImageLib.h"

#include <assert.h>
#include <math.h>

#define FLOAT_TO_26_6(d) ((FT_F26Dot6)((d) * 64.0f))
#define FLOAT_FROM_26_6(t) ((float)(t) / 64.0f)
#define FLOAT_TO_16_16(d) ((FT_Fixed)((d) * 65536.0f))
#define FLOAT_FROM_16_16(t) ((float)(t) / 65536.0f)

using namespace Sexy;

FreeTypeFont::FreeTypeFont(const std::string& theFace, int thePointSize, bool bold,
			   bool italics, bool underline)
{
	Init(gSexyAppBase, theFace, thePointSize, bold, italics, underline);
}

FreeTypeFont::FreeTypeFont(SexyAppBase* theApp, const std::string& theFace, int thePointSize,
			   bool bold, bool italics, bool underline)
{
	Init(theApp, theFace, thePointSize, bold, italics, underline);
}

void FreeTypeFont::Init(SexyAppBase* theApp, const std::string& theFace, int thePointSize,
			bool bold, bool italics, bool underline)
{
	FreeTypeFontMap* aFontMap = FreeTypeFontMap::GetFreeTypeFontMap();

	mScaledFont = aFontMap->CreateScaledFont(theApp, theFace, thePointSize,
						 bold, italics, underline);
	mSupportUnicode = true;
	mHeight = mScaledFont->mHeight;
	mAscent = mScaledFont->mAscent;
	mLineSpacingOffset = mScaledFont->mLineSpacingOffset;
}

FreeTypeFont::FreeTypeFont(const FreeTypeFont& theFont)
{
	mSupportUnicode = true;
	mApp = theFont.mApp;
	mScaledFont = theFont.mScaledFont;
	mHeight = theFont.mHeight;
	mAscent = theFont.mAscent;
	mLineSpacingOffset = theFont.mLineSpacingOffset;
	mDrawShadow = theFont.mDrawShadow;
	mSimulateBold =theFont.mSimulateBold;
	mScaledFont->Ref();
}

FreeTypeFont::~FreeTypeFont()
{
	FreeTypeFontMap* aFontMap = FreeTypeFontMap::GetFreeTypeFontMap();

	aFontMap->FreeScaledFont(mScaledFont);
}

int FreeTypeFont::StringWidth(const std::string& theString)
{
	bool unicode = false;
	if (Graphics::GetPreferedEncoding() == "UTF-8")
               unicode = true;
	return mScaledFont->StringWidth(theString, unicode);
}

void FreeTypeFont::DrawString(Graphics* g, int theX, int theY, const std::string& theString,
			      const Color& theColor, const Rect& theClipRect)
{
	bool unicode = false;
	if (Graphics::GetPreferedEncoding() == "UTF-8")
               unicode = true;
	mScaledFont->DrawString(g, theX, theY, theString, theColor, theClipRect,
				unicode, mDrawShadow);
}

int FreeTypeFont::StringWidth(const std::wstring& theString)
{
	return mScaledFont->StringWidth(theString);
}

void FreeTypeFont::DrawString(Graphics* g, int theX, int theY, const std::wstring& theString,
			      const Color& theColor, const Rect& theClipRect)
{
	mScaledFont->DrawString(g, theX, theY, theString, theColor, theClipRect, mDrawShadow);
}

bool FreeTypeFont::StringToGlyphs(const std::wstring &theString,
				  GlyphVector &theGlyphs)
{
	return mScaledFont->StringToGlyphs(theString, theGlyphs);
}

void FreeTypeFont::DrawGlyphs(Graphics *g, int theX, int theY,
			      GlyphVector& theGlyphs,
			      size_t from, size_t length,
			      const Color &theColor,
			      const Rect& theClipRect)
{
	mScaledFont->DrawGlyphs(g, theX, theY, theGlyphs, theColor, theClipRect, mDrawShadow);
}

int FreeTypeFont::CharWidth(int theChar)
{
	return mScaledFont->CharWidth(theChar);
}

int FreeTypeFont::CharWidthKern(int theChar, int thePrevChar)
{
	return mScaledFont->CharWidthKern(theChar, thePrevChar);
}

Font* FreeTypeFont::Duplicate()
{
	return new FreeTypeFont(*this);
}
