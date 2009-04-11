#ifndef __FREETYPE_SCALED_FONT_H__
#define __FREETYPE_SCALED_FONT_H__

#include "Font.h"
#include "FreeTypeBaseFont.h"

namespace Sexy
{
	class Image;
	class ImageFont;
	class SexyAppBase;
	struct FreeTypeGlyphArea;
	struct FreeTypeGlyphEntry;

	struct	FreeTypeExtents {
		float x_bearing;
		float y_bearing;
		float width;
		float height;
		float x_advance;
		float y_advance;
	};

	struct FreeTypeGlyph
	{
		float x;
		float y;
		FT_UInt index;

		FreeTypeGlyphEntry* entry;
	};

	struct FreeTypeGlyphEntry
	{
		// Glyph Image & its rectangle in the image
		Image* mImage;
		int    mXOffSet;
		int    mYOffSet;
		int    mWidth;
		int    mHeight;

		FreeTypeGlyphArea* mArea;

		// Glyph metrics
		FreeTypeExtents mMetrics;
	};

	enum {
		FREETYPE_GLYPH_AREA_EMPTY    = 0,
		FREETYPE_GLYPH_AREA_PARTIAL  = 1,
		FREETYPE_GLYPH_AREA_FULL     = 2,
		FREETYPE_GLYPH_AREA_LOCKED   = 1 << 15
	};

	struct FreeTypeGlyphArea
	{
		short state;
		unsigned short level;
		int x;
		int y;
		int width;
		int height;

		FT_UInt index;

		FreeTypeGlyphArea * children[4];
	};

	const unsigned int MAX_CACHED_IMAGES = 3;
	const unsigned int DEFAULT_CACHE_ORDER = 7;
	const unsigned int MAX_CACHE_LEVEL = 64;

	class FreeTypeScaledFont
	{
	public:
		SexyAppBase*			mApp;

		void Init(SexyAppBase* theApp, const std::string& theFace,
			  int thePointSize, bool bold, bool italics,
			  bool underline);

	public:
		FreeTypeScaledFont(const std::string& theFace, int thePointSize,
				   bool bold = false, bool italics = false,
				   bool underline = false);

		FreeTypeScaledFont(SexyAppBase* theApp, const std::string& theFace,
				   int thePointSize, bool bold = false, bool italics = false,
				   bool underline = false);
		FreeTypeScaledFont(const FreeTypeScaledFont& theFreeTypeScaledFont);

	private:
		~FreeTypeScaledFont();

	public:
		int				StringWidth(const SexyString& theString);
		void				DrawString(Graphics* g, int theX, int theY,
							   const SexyString& theString,
							   const Color& theColor,
							   const Rect& theClipRect,
							   bool drawShadow = false,
							   bool drawOutline = false);
		int				CharWidth(int theChar);
		int				CharWidthKern(int theChar, int thePrevChar);

		FreeTypeScaledFont*		Duplicate();

	private:
		FreeTypeBaseFont	      * mBaseFont;
		FT_Matrix			mMatrix;
		float				mSize;

		FT_Face				mFace;

	public:
		int				mDescent;
		int				mAscent;
		int				mAscentPadding;
		int				mHeight;
		int				mLineSpacingOffset;

	private:
		CritSect			mRefCritSect;
		int				mRefCnt;

		typedef std::map<FT_UInt, FreeTypeGlyphEntry> GlyphMap;
		typedef std::vector<FreeTypeGlyph>  GlyphVector;

		// Glyph info cache
		GlyphMap			mGlyphMap;

		// Glyph Image cache
		Image*				mImages[MAX_CACHED_IMAGES];
		int				mImageSizeOrder[MAX_CACHED_IMAGES];
		FreeTypeGlyphArea		mImageAreas[MAX_CACHED_IMAGES];

		void				LockFace();
		void				UnlockFace();

		FreeTypeGlyphEntry*		LoadGlyph(FT_UInt index, bool render = false);
		FreeTypeGlyphEntry*		LookupGlyph(FT_UInt index, bool render = false);
		FreeTypeExtents*		LookupGlyphMetrics(FT_UInt index);

		// Glyph cached area
		FreeTypeGlyphArea*		FindGlyphAreaInArea(int width, int height, FT_UInt index,
								    FreeTypeGlyphArea* area, bool remove = false);
		FreeTypeGlyphArea*		FindGlyphArea(int width, int height, FT_UInt index, Image** image);
		FreeTypeGlyphArea*		FindAnAreaToRemove(FreeTypeGlyphArea* area);

		void				RemoveGlyphImage(FT_UInt index);

		FreeTypeGlyphArea*		FreeTypeGlyphAreaCreate(int x, int y, int width, int height, int level);
		void				FreeTypeGlyphAreaFree(FreeTypeGlyphArea* area);

		void				ShrinkGlyphCache(void);

		int				Utf8FromString(const std::string& string,
							       std::string& utf8);

		void				GlyphsFromString(const std::string& string, GlyphVector& glyphs,
								 bool render = false);

	public:
		void				Ref();
		void				Unref();
	};

}

#endif
