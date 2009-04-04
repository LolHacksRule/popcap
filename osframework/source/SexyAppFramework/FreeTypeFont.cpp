#include "FreeTypeFont.h"
#include "FreeTypeFontMap.h"
#include "SexyAppBase.h"

#include <assert.h>

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

	LockFace();
	UnlockFace();
}

FreeTypeFont::FreeTypeFont(const FreeTypeFont& theFreeTypeFont)
{
	mApp = theFreeTypeFont.mApp;
	mBaseFont = theFreeTypeFont.mBaseFont;
	mHeight = theFreeTypeFont.mHeight;
	mAscent = theFreeTypeFont.mAscent;

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
