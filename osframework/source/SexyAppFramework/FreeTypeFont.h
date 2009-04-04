#ifndef __FREETYPE_FONT_H__
#define __FREETYPE_FONT_H__

#include "Font.h"
#include "FreeTypeBaseFont.h"

namespace Sexy
{
	class Image;
	class ImageFont;
	class SexyAppBase;
	struct FreeTypeGlyphArea;

	struct  FreeTypeExtents {
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
	};

	struct FreeTypeGlyphEntry
	{
		// Glyph Image & its rectangle in the image
		Image* mImage;
		int    mXOffSet;
		int    mYOffSet;
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

	class FreeTypeFont : public Font
	{
	public:
		SexyAppBase*			mApp;

		void Init(SexyAppBase* theApp, const std::string& theFace,
			  int thePointSize, bool bold, bool italics,
			  bool underline);

	public:
		FreeTypeFont(const std::string& theFace, int thePointSize,
			     bool bold = false, bool italics = false,
			     bool underline = false);

		FreeTypeFont(SexyAppBase* theApp, const std::string& theFace,
			     int thePointSize, bool bold = false, bool italics = false,
			     bool underline = false);
		FreeTypeFont(const FreeTypeFont& theFreeTypeFont);

		virtual ~FreeTypeFont();

		virtual int			StringWidth(const SexyString& theString);
		virtual void			DrawString(Graphics* g, int theX, int theY,
							   const SexyString& theString, const Color& theColor,
							   const Rect& theClipRect);

		virtual Font*			Duplicate();

	private:
		FreeTypeBaseFont              * mBaseFont;
		FT_Matrix                       mMatrix;
		float                           mSize;

		FT_Face                         mFace;

		int                             mDescent;

		typedef std::map<FT_UInt, FreeTypeGlyphEntry> GlyphMap;
		typedef std::vector<FreeTypeGlyph>  GlyphVector;

		// Glyph info cache
		GlyphMap                        mGlyphMap;

		// Glyph Image cache
		Image*                          mImages[MAX_CACHED_IMAGES];
		int                             mImageSizeOrder[MAX_CACHED_IMAGES];
		FreeTypeGlyphArea               mImageAreas[MAX_CACHED_IMAGES];

		void                            LockFace();
		void                            UnlockFace();

		FreeTypeGlyphEntry*	        LoadGlyph(FT_UInt index, bool render = false);
		FreeTypeGlyphEntry*             LookupGlyph(FT_UInt index, bool render = false);
		FreeTypeExtents*                LookupGlyphMetrics(FT_UInt index);

		// Glyph cached area
		FreeTypeGlyphArea*              FindGlyphAreaInArea(int width, int height, FT_UInt index,
								    FreeTypeGlyphArea* area, bool remove = false);
		FreeTypeGlyphArea*              FindGlyphArea(int width, int height, FT_UInt index, Image** image);
		FreeTypeGlyphArea*              FindAnAreaToRemove(FreeTypeGlyphArea* area);

		void                            RemoveGlyphImage(FT_UInt index);

		FreeTypeGlyphArea*              FreeTypeGlyphAreaCreate(int x, int y, int width, int height, int level);
		void                            FreeTypeGlyphAreaFree(FreeTypeGlyphArea* area);

		void                            ShrinkGlyphCache(void);
	};

}

#endif
