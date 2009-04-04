#include "FreeTypeFontMap.h"
#include "FreeTypeBaseFont.h"

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
	FontKey aKey(std::string(path), index);
	FontMap::iterator it;

	it = mFontMap.find(aKey);
	if (it != mFontMap.end())
	{
		font = it->second;
		font->Ref();
		return font;
	}

	PFILE* fp = p_fopen(path, "rb");
	if (!fp)
		return 0;

	font = new FreeTypeBaseFont(fp, index);
	mFontMap.insert(std::pair<FontKey,FreeTypeBaseFont*>(aKey, font));

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
