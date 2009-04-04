#include "FreeTypeFont.h"
#include "FreeTypeFontMap.h"
#include "SexyAppBase.h"

#include <assert.h>

#define DOUBLE_TO_26_6(d) ((FT_F26Dot6)((d) * 64.0))
#define DOUBLE_FROM_26_6(t) ((double)(t) / 64.0)
#define DOUBLE_TO_16_16(d) ((FT_Fixed)((d) * 65536.0))
#define DOUBLE_FROM_16_16(t) ((double)(t) / 65536.0)

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

	mApp = theApp;
	mSize = thePointSize;
	mBaseFont = aFontMap->CreateBaseFont(theFace.c_str(), 0);

	mHeight = 0;
	mAscent = 0;
	mDescent = 0;

        mMatrix.xx = 1 << 16;
        mMatrix.yx = 0;
        mMatrix.xy = 0;
        mMatrix.yy = 1 << 16;

	LockFace();
	if (mFace)
	{
		float scale = mFace->units_per_EM;

		mAscent  = mFace->ascender * mSize / scale;
		mDescent = -mFace->descender * mSize / scale;
		mHeight  = mFace->height * mSize / scale;
		mLineSpacingOffset = mHeight - mAscent - mDescent;
	}
	UnlockFace();
}

FreeTypeFont::FreeTypeFont(const FreeTypeFont& theFreeTypeFont)
{
	mApp = theFreeTypeFont.mApp;
	mBaseFont = theFreeTypeFont.mBaseFont;
	mHeight = theFreeTypeFont.mHeight;
	mAscent = theFreeTypeFont.mAscent;
	mDescent = theFreeTypeFont.mDescent;
	mLineSpacingOffset = theFreeTypeFont.mLineSpacingOffset;

	if (mBaseFont)
		mBaseFont->Ref();
}

FreeTypeFont::~FreeTypeFont()
{
	if (mBaseFont)
		mBaseFont->Unref();
}

int FreeTypeFont::StringWidth(const SexyString& theString)
{
	if (!mBaseFont)
		return 0;
	return 0;
}

void FreeTypeFont::DrawString(Graphics* g, int theX, int theY, const SexyString& theString,
			      const Color& theColor, const Rect& theClipRect)
{
	if (!mBaseFont)
		return;
}

Font* FreeTypeFont::Duplicate()
{
	return new FreeTypeFont(*this);
}

void FreeTypeFont::LockFace()
{
	if (mBaseFont)
		mFace = mBaseFont->LockFace(mSize, &mMatrix);
}

void FreeTypeFont::UnlockFace()
{
	mFace = 0;
	if (mBaseFont)
		mBaseFont->UnlockFace();
}
