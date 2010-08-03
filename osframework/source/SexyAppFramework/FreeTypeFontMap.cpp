#include "SexyAppBase.h"
#include "FreeTypeFontMap.h"
#include "FreeTypeBaseFont.h"
#include "FreeTypeScaledFont.h"

using namespace Sexy;

FreeTypeFontMap::FreeTypeFontMap()
{
	mFaces = 0;
	mMaxFaces = 10;

	FT_Init_FreeType (&mLibrary);
}

FreeTypeFontMap::~FreeTypeFontMap()
{
	 FT_Done_FreeType (mLibrary);
}

FreeTypeFontMap* FreeTypeFontMap::GetFreeTypeFontMap()
{
	static FreeTypeFontMap* aFontMap = 0;
	if (!aFontMap)
		aFontMap = new FreeTypeFontMap();
	return aFontMap;
}


void FreeTypeFontMap::Lock()
{
	mCritSect.Enter();
}

void FreeTypeFontMap::Unlock()
{
	mCritSect.Leave();
}

FreeTypeBaseFont* FreeTypeFontMap::CreateBaseFont(const char * path,
						  int          index)
{
	AutoCrit anAutoCrit(mCritSect);
	FreeTypeBaseFont* font;
	FontMap::iterator it;

	if (path && *path)
	{
		FontKey aKey(std::string(path), index);
		it = mFontMap.find(aKey);
	}
	else
	{
		it = mFontMap.begin();
	}
	if (it != mFontMap.end())
	{
		font = it->second;
		font->Ref();
		return font;
	}

	if (!path || !*path)
		return 0;

	PFILE* fp = p_fopen(path, "rb");
	if (!fp)
		return 0;

	font = new FreeTypeBaseFont(fp, index);
	mFontMap.insert(std::pair<FontKey,FreeTypeBaseFont*>(FontKey(std::string(path), index),
							     font));

	return font;
}

void FreeTypeFontMap::RemoveBaseFont(FreeTypeBaseFont* font)
{
	AutoCrit anAutoCrit(mCritSect);
	FontMap::iterator it;

	for (it = mFontMap.begin(); it != mFontMap.end(); ++it)
	{
		if (it->second == font)
		{
			mFontMap.erase(it);
			return;
		}
	}
}

FreeTypeScaledFont* FreeTypeFontMap::CreateScaledFont(SexyAppBase* theApp,
						      const std::string& theFace,
						      int thePointSize,
						      bool bold,
						      bool italics,
						      bool underline)
{
	FreeTypeScaledFont* result = 0;
	ScaledFontKey key(theApp, theFace, thePointSize, bold, italics, underline);

	AutoCrit anAutoCrit(mScaledFontCritSect);

	ScaledFontMap::iterator it = mScaledFontMap.find(key);
	if (it != mScaledFontMap.end())
	{
		result = it->second;
		result->Ref();
		return result;
	}

	result = new FreeTypeScaledFont(theApp, theFace, thePointSize, bold,
					italics, underline);

	mScaledFontMap.insert(ScaledFontMap::value_type(key, result));
	return result;
}

void FreeTypeFontMap::FreeScaledFont(FreeTypeScaledFont *theFont)
{
	if (!theFont)
		return;

	AutoCrit anAutoCrit(mScaledFontCritSect);

	ScaledFontMap::iterator it;
	for (it = mScaledFontMap.begin(); it != mScaledFontMap.end(); ++it)
	{
		if (it->second == theFont)
		{
			if (!theFont->Unref())
				mScaledFontMap.erase(it);
			return;
		}
	}

	theFont->Unref();
}

FreeTypeScaledFont* FreeTypeFontMap::CreateScaledFont(const std::string& theFace,
						      int thePointSize,
						      bool bold,
						      bool italics,
						      bool underline)
{
	return CreateScaledFont(gSexyAppBase, theFace, bold, italics, underline);
}

void FreeTypeFontMap::ReserveFace(FreeTypeBaseFont* font)
{
	AutoCrit anAutoCrit(mCritSect);

	while (mFaces >= mMaxFaces)
	{
		FontMap::iterator it;

		for (it = mFontMap.begin(); it != mFontMap.end(); ++it)
		{
			FreeTypeBaseFont* aFont = it->second;

			if (aFont != font && aFont->DropUnlockedFace())
			{
				mFaces--;
				break;
			}
		}
	}
	mFaces++;
}

void FreeTypeFontMap::ReleaseFace(FreeTypeBaseFont* font)
{
	AutoCrit anAutoCrit(mCritSect);
	mFaces--;
}

int FreeTypeFontMap::GetMaxFaces()
{
	return mMaxFaces;
}

int FreeTypeFontMap::GetFaces()
{
	return mFaces;
}

FT_Library FreeTypeFontMap::GetFreeTypeLibrary()
{
	return mLibrary;
}
