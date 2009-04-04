#include "FreeTypeFont.h"
#include "FreeTypeFontMap.h"
#include "SexyAppBase.h"
#include "Image.h"

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

	for (unsigned i = 0; i < MAX_CACHED_IMAGES; i++)
	{
		mImages[i] = 0;
		// default to 256x256
		mImageSizeOrder[i] = i + 8;

		// setup glyph cache root area
		FreeTypeGlyphArea* area = &mImageAreas[i];
		area->x = 0;
		area->y = 0;
		area->width = 1 << mImageSizeOrder[i];
		area->height = 1 << mImageSizeOrder[i];
		area->state = FREETYPE_GLYPH_AREA_EMPTY;
		area->level = 0;
		memset(&area->children, 0, sizeof(area->children));
	}

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

	mMatrix = theFreeTypeFont.mMatrix;

	for (unsigned i = 0; i < MAX_CACHED_IMAGES; i++)
	{
		mImages[i] = 0;
		// default to 256x256
		mImageSizeOrder[i] = i + 8;

		// setup glyph cache root area
		FreeTypeGlyphArea* area = &mImageAreas[i];
		area->x = 0;
		area->y = 0;
		area->width = 1 << mImageSizeOrder[i];
		area->height = 1 << mImageSizeOrder[i];
		area->state = FREETYPE_GLYPH_AREA_EMPTY;
		area->level = 0;
		memset(&area->children, 0, sizeof(area->children));
	}

	if (mBaseFont)
		mBaseFont->Ref();
}

FreeTypeFont::~FreeTypeFont()
{
	if (mBaseFont)
		mBaseFont->Unref();

	for (unsigned i = 0; i < MAX_CACHED_IMAGES; i++)
	{
		delete mImages[i];

		for (unsigned j = 0; j < 4; j++)
			FreeTypeGlyphAreaFree(mImageAreas[i].children[j]);
	}
}

FreeTypeGlyphArea* FreeTypeFont::FreeTypeGlyphAreaCreate(int x, int y, int width, int height, int level)
{
	FreeTypeGlyphArea* area = new FreeTypeGlyphArea;

	assert (x >= 0 && y >= 0 && width >= 0 && height >= 0);

	area->x = x;
	area->y = y;
	area->width = width;
	area->height = height;
	area->level = level;
	area->state = FREETYPE_GLYPH_AREA_EMPTY;
	memset (&area->children, 0, sizeof(area->children));
	return area;
}

void FreeTypeFont::FreeTypeGlyphAreaFree(FreeTypeGlyphArea* area)
{
	if (!area)
		return;

	for (unsigned int i = 0; i < 4; i++)
		FreeTypeGlyphAreaFree(area->children[i]);

	if ((area->state & 3) == FREETYPE_GLYPH_AREA_FULL)
		RemoveGlyphImage(area->index);
	delete area;
}

int FreeTypeFont::StringWidth(const SexyString& theString)
{
	if (!mBaseFont)
		return 0;

	LockFace();
	if (!mFace)
	{
		UnlockFace();
		return 0;
	}

	FT_Face face = mFace;
	bool first = true;
        float x = 0, y = 0;
	int min_x = 0, max_x = 0;
	int min_y = 0, max_y = 0;

	for (unsigned int i = 0; i < theString.length(); i++)
	{
		int index = FT_Get_Char_Index (face, theString[i]);
		FreeTypeExtents* metrics = LookupGlyphMetrics(index);
		if (!metrics)
			continue;

		int  left, top;
		int  right, bottom;

		left   = floor(x + metrics->x_bearing);
		top    = floor(y + metrics->y_bearing);
		right  = ceil(x + metrics->x_advance);
		bottom = ceil(y + metrics->y_advance);

		x += metrics->x_advance;
		y += metrics->y_advance;

		if (first) {
			min_x = left;
			max_x = right;
			min_y = top;
			max_y = bottom;
			first = false;
		}
		else
		{
			if (left < min_x)
				min_x = left;
			if (right > max_x)
				max_x = right;
			if (top < min_y)
				min_y = top;
			if (bottom > max_y)
				max_y = bottom;
		}
	}

	UnlockFace();
	return max_x - min_x;
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
	ShrinkGlyphCache();
}

FreeTypeGlyphEntry* FreeTypeFont::LoadGlyph(FT_UInt index, bool render)
{
	if (!mFace)
		return 0;

	int load_flags = FT_LOAD_DEFAULT | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;
	FT_Face face = mFace;
	FT_GlyphSlot glyph;
	FT_Error error;

	error = FT_Load_Glyph (face, index, load_flags);
	if (error)
		return 0;

	glyph = face->glyph;

	GlyphMap::iterator it;
	FreeTypeGlyphEntry* entry;

	it = mGlyphMap.find(index);
	if (it == mGlyphMap.end())
	{
		mGlyphMap[index] = FreeTypeGlyphEntry();
		entry = &mGlyphMap[index];
		memset (entry, 0, sizeof (FreeTypeGlyphEntry));

		FT_Glyph_Metrics * metrics = &glyph->metrics;

		entry->mMetrics.width  = FLOAT_FROM_26_6 (metrics->width);
		entry->mMetrics.height = FLOAT_FROM_26_6 (metrics->height);

		entry->mMetrics.x_bearing = FLOAT_FROM_26_6 (metrics->horiBearingX);
		entry->mMetrics.y_bearing = FLOAT_FROM_26_6 (-metrics->horiBearingY);

		entry->mMetrics.x_advance = FLOAT_FROM_26_6 (metrics->horiAdvance);
		entry->mMetrics.y_advance = 0;
	}
	else
	{
		entry = &it->second;
	}

	if (render || true)
	{
		// FIXME: render the glyph to an Image.
		error = FT_Render_Glyph (glyph, FT_RENDER_MODE_NORMAL);
		if (error)
			return 0;

		FT_Bitmap* bitmap = &glyph->bitmap;
		entry->mXOffSet = glyph->bitmap_left;
		entry->mYOffSet = -glyph->bitmap_top;

		// Find a area to upload glyph image data
		entry->mArea = FindGlyphArea(bitmap->width, bitmap->rows, index, &entry->mImage);
		if (entry->mArea)
		{
			printf ("index: %d cached at (%d %d %d %d)\n",
				index, entry->mArea->x, entry->mArea->y,
				entry->mArea->width, entry->mArea->height);
		}
	}

	return entry;
}

FreeTypeGlyphEntry* FreeTypeFont::LookupGlyph(FT_UInt index, bool render)
{
	GlyphMap::iterator it;

	it = mGlyphMap.find(index);
	if (it != mGlyphMap.end() && (!render || (render && it->second.mImage)))
		return &it->second;

	return LoadGlyph(index, render);
}

void FreeTypeFont::RemoveGlyphImage(FT_UInt index)
{
	GlyphMap::iterator it;

	it = mGlyphMap.find(index);
	if (it == mGlyphMap.end())
		return;
	if (it->second.mImage && it->second.mArea)
	{
		it->second.mArea->index = 0;
		it->second.mArea->state = FREETYPE_GLYPH_AREA_EMPTY;
	}
	it->second.mImage = 0;
	it->second.mArea = 0;
}

void FreeTypeFont::ShrinkGlyphCache(void)
{
	GlyphMap::iterator it;

	size_t size = mGlyphMap.size();
	while (size-- > 1024)
	{
		it = mGlyphMap.begin();
		if (it == mGlyphMap.end())
			break;
		if (it->second.mImage && it->second.mArea)
		{
			it->second.mArea->index = 0;
			it->second.mArea->state = FREETYPE_GLYPH_AREA_EMPTY;
		}
		mGlyphMap.erase(it);
	}
}

FreeTypeExtents* FreeTypeFont::LookupGlyphMetrics(FT_UInt index)
{
	FreeTypeGlyphEntry* entry = LookupGlyph(index);
	if (!entry)
		return 0;
	return &entry->mMetrics;
}

FreeTypeGlyphArea* FreeTypeFont::FindAnAreaToRemove(FreeTypeGlyphArea* area)
{
	if (!area)
		return 0;

	int state = area->state & 0x3;
	// got one
	if (state == FREETYPE_GLYPH_AREA_FULL)
	{
		// if the area is locked, return none
		if (area->state & FREETYPE_GLYPH_AREA_LOCKED)
			return 0;
		return area;
	}
	else if (state == FREETYPE_GLYPH_AREA_EMPTY)
	{
		return 0;
	}

	// check children
	for (unsigned int i = 0; i < 4; i++)
	{
		FreeTypeGlyphArea* subarea = FindAnAreaToRemove(area->children[i]);
		if (subarea)
			return subarea;
	}

	return 0;
}

FreeTypeGlyphArea* FreeTypeFont::FindGlyphAreaInArea(int width, int height, FT_UInt index,
						     FreeTypeGlyphArea* area, bool remove)
{
	if (area->width < width || area->height < height)
		return 0;

	int state = area->state & 0x3;
	FreeTypeGlyphEntry* entry;
	switch (state)
	{
	case FREETYPE_GLYPH_AREA_FULL:
		if (!remove || (area->state & FREETYPE_GLYPH_AREA_LOCKED))
		    return 0;

		entry = LookupGlyph(area->index);
		if (entry)
		{
			entry->mImage = 0;
			entry->mArea = 0;
		}
		area->state = FREETYPE_GLYPH_AREA_EMPTY;

	case FREETYPE_GLYPH_AREA_EMPTY:
		if (area->level == MAX_CACHE_LEVEL || (area->width == width && area->height == height))
		{
			area->index = index;
			area->state = FREETYPE_GLYPH_AREA_FULL;
			return area;
		}
		else
		{
			int leftWidth = area->width - width;
			int leftHeight = area->height - height;

			area->children[0] =
				FreeTypeGlyphAreaCreate (area->x, area->y, width, height,
							 area->level + 1);
			area->children[1] =
				FreeTypeGlyphAreaCreate (area->x + width, area->y,
							 leftWidth, leftHeight,
							 area->level + 1);
			area->children[2] =
				FreeTypeGlyphAreaCreate (area->x, area->y + height,
							 width, leftHeight,
							 area->level + 1);
			area->children[3] =
				FreeTypeGlyphAreaCreate (area->x + width, area->y + height,
							 leftWidth, leftHeight,
							 area->level + 1);
		}

		area->state = FREETYPE_GLYPH_AREA_PARTIAL;
		area = area->children[0];

		if (!area)
			return 0;

		area->state = FREETYPE_GLYPH_AREA_FULL;
		area->index = index;

		return area;

	case FREETYPE_GLYPH_AREA_PARTIAL:
		bool failed;

		for (unsigned i = 0; i < 4; i++) {
			if (area->children[i])
			{
				FreeTypeGlyphArea* child = area->children[i];
				if (child->width >= width && child->height >= height)
				{
					if (FindGlyphAreaInArea (width, height, index,
								 child, remove))
						return area;

					failed = true;
				}
			}
		}

		if (failed || !remove)
			return 0;

		area = FindAnAreaToRemove(area);
		if (!area)
			return 0;

		for (unsigned i = 0; i < 4; i++)
		{
			FreeTypeGlyphAreaFree (area->children[i]);
			area->children[i] = 0;
		}

		area->state = FREETYPE_GLYPH_AREA_EMPTY;
		return FindGlyphAreaInArea(width, height, index, area, remove);
	}

	return 0;
}

FreeTypeGlyphArea* FreeTypeFont::FindGlyphArea(int width, int height, FT_UInt index, Image** image)
{
	FreeTypeGlyphArea* area;
	for (unsigned i = 0; i < MAX_CACHED_IMAGES; i++)
	{
		area = FindGlyphAreaInArea (width, height, index,  &mImageAreas[i]);
		if (area)
		{
			if (!mImages[i])
				mImages[i] =
					mApp->CreateImage(1 << mImageSizeOrder[i], 1 << mImageSizeOrder[i]);
			*image = mImages[i];
			return area;
		}
	}

	for (unsigned i = 0; i < MAX_CACHED_IMAGES; i++)
	{
		area = FindGlyphAreaInArea (width, height, index,  &mImageAreas[i], true);
		if (area)
		{
			if (!mImages[i])
				mImages[i] =
					mApp->CreateImage(1 << mImageSizeOrder[i], 1 << mImageSizeOrder[i]);
			*image = mImages[i];
			return area;
		}
	}

	return 0;
}
