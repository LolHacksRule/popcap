#ifndef FREETYPEFONTMAP_H
#define FREETYPEFONTMAP_H

#include "AutoCrit.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Sexy
{

class FreeTypeBaseFont;

	class FreeTypeFontMap
	{
		friend class FreeTypeBaseFont;
	public:
		CritSect mCritSect;
		FT_Library mLibrary;

	public:
		static FreeTypeFontMap*    GetFreeTypeFontMap();

		int                        GetMaxFaces();
		int                        GetFaces();
		FT_Library                 GetFreeTypeLibrary();
		FreeTypeBaseFont*          CreateBaseFont(const char *path,
							  int index);
		void                       RemoveBaseFont(FreeTypeBaseFont* font);

		void                       Lock();
		void                       Unlock();

		void                       ReserveFace(FreeTypeBaseFont* font);
		void                       ReleaseFace(FreeTypeBaseFont* font);

	private:
		FreeTypeFontMap();
		~FreeTypeFontMap();

	private:
		int  mMaxFaces;
		int  mFaces;

		typedef std::pair<std::string, int> FontKey;
		typedef std::map<FontKey, FreeTypeBaseFont*> FontMap;

		FontMap                   mFontMap;
	};

}

#endif
