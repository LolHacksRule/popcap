#include "FreeTypeScaledFont.h"
#include "FreeTypeFontMap.h"
#include "SexyAppBase.h"
#include "MemoryImage.h"
#include "Graphics.h"
#include "FontUtils.h"

#include "ImageLib.h"

#include <assert.h>
#include <math.h>

#define FLOAT_TO_26_6(d) ((FT_F26Dot6)((d) * 64.0f))
#define FLOAT_FROM_26_6(t) ((float)(t) / 64.0f)
#define FLOAT_TO_16_16(d) ((FT_Fixed)((d) * 65536.0f))
#define FLOAT_FROM_16_16(t) ((float)(t) / 65536.0f)

using namespace Sexy;

FreeTypeScaledFont::FreeTypeScaledFont(const std::string& theFace, int thePointSize, bool bold,
				       bool italics, bool underline)
{
	Init(gSexyAppBase, theFace, thePointSize, bold, italics, underline);
}

FreeTypeScaledFont::FreeTypeScaledFont(SexyAppBase* theApp, const std::string& theFace, int thePointSize,
				       bool bold, bool italics, bool underline)
{
	Init(theApp, theFace, thePointSize, bold, italics, underline);
}

void FreeTypeScaledFont::Init(SexyAppBase* theApp, const std::string& theFace, int thePointSize,
			      bool bold, bool italics, bool underline)
{
	FreeTypeFontMap* aFontMap = FreeTypeFontMap::GetFreeTypeFontMap();

	mRefCnt = 1;
	mApp = theApp;
	mSize = thePointSize;
	mBaseFont = aFontMap->CreateBaseFont(theFace.c_str(), 0);
	if (!mBaseFont)
		mBaseFont = aFontMap->CreateBaseFont(0, 0);
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
		// default to 128x128
		mImageSizeOrder[i] = i + DEFAULT_CACHE_ORDER;

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

		mAscent	 = mFace->ascender * mSize / scale;
		mDescent = -mFace->descender * mSize / scale;
		mHeight	 = mFace->height * mSize / scale;
		mLineSpacingOffset = mHeight - mAscent - mDescent;
	}
	UnlockFace();
}

FreeTypeScaledFont::FreeTypeScaledFont(const FreeTypeScaledFont& theFreeTypeScaledFont)
{
	mRefCnt = 1;
	mApp = theFreeTypeScaledFont.mApp;
	mBaseFont = theFreeTypeScaledFont.mBaseFont;
	mHeight = theFreeTypeScaledFont.mHeight;
	mAscent = theFreeTypeScaledFont.mAscent;
	mDescent = theFreeTypeScaledFont.mDescent;
	mLineSpacingOffset = theFreeTypeScaledFont.mLineSpacingOffset;

	mMatrix = theFreeTypeScaledFont.mMatrix;
	mSize = theFreeTypeScaledFont.mSize;

	for (unsigned i = 0; i < MAX_CACHED_IMAGES; i++)
	{
		mImages[i] = 0;
		// default to 128x128
		mImageSizeOrder[i] = i + DEFAULT_CACHE_ORDER;

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

FreeTypeScaledFont::~FreeTypeScaledFont()
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

FreeTypeGlyphArea* FreeTypeScaledFont::FreeTypeGlyphAreaCreate(int x, int y, int width, int height, int level)
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

void FreeTypeScaledFont::FreeTypeGlyphAreaFree(FreeTypeGlyphArea* area)
{
	if (!area)
		return;

	for (unsigned int i = 0; i < 4; i++)
		FreeTypeGlyphAreaFree(area->children[i]);

	if ((area->state & 3) == FREETYPE_GLYPH_AREA_FULL)
		RemoveGlyphImage(area->index);
	delete area;
}

int FreeTypeScaledFont::StringWidth(const SexyString& theString)
{
	if (!mBaseFont)
		return 0;

	LockFace();
	if (!mFace)
	{
		UnlockFace();
		return 0;
	}

	bool first = true;
	float x = 0, y = 0;
	int min_x = 0, max_x = 0;
	int min_y = 0, max_y = 0;

	GlyphVector glyphs;
	GlyphsFromString(theString, glyphs);
	for (unsigned int i = 0; i < glyphs.size(); i++)
	{
		FreeTypeGlyphEntry* entry = glyphs[i].entry;
		if (!entry)
			continue;

		FreeTypeExtents* metrics = &entry->mMetrics;
		int  left, top;
		int  right, bottom;

		left   = floor(x + metrics->x_bearing);
		top    = floor(y + metrics->y_bearing);
		right  = ceil(x + metrics->x_advance);
		bottom = ceil(y + metrics->y_advance);

		x += metrics->x_advance;
		y += metrics->y_advance;

		if (first)
		{
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

		if (entry->mArea)
			entry->mArea->state &= ~FREETYPE_GLYPH_AREA_LOCKED;
	}

	UnlockFace();
	return max_x - min_x;
}

int FreeTypeScaledFont::Utf8FromString(const std::string& string,
				       std::string& utf8)
{
	int len = SexyUtf8Strlen(string.c_str(), -1);
	if (len >= 0)
	{
		utf8 = string;
		return len;
	}

	char* result;
	len = SexyUtf8FromLocale(string.c_str(), -1, &result);
	if (len >= 0)
	{
		utf8 = std::string(result);
		delete [] result;
		return len;
	}

	return -1;
}

void FreeTypeScaledFont::GlyphsFromString(const std::string& string, GlyphVector& glyphs,
					  bool render)
{
	std::string utf8;

	glyphs.clear();

	FT_Face face = mFace;
	int len = Utf8FromString(string, utf8);
	if (len >= 0)
	{
		const char* chars = utf8.c_str();
		int charslen = utf8.length();
		uint32 unichar;
		int unicharlen;

		glyphs.reserve(len);
		for (int i = 0; i < len; i++)
		{
			unicharlen = SexyUtf8ToUcs4Char(chars, &unichar, charslen);
			chars += unicharlen;
			charslen -= unicharlen;

			if (unichar == '\n' || unichar == '\r')
				continue;

			int index = FT_Get_Char_Index (face, unichar);
			FreeTypeGlyphEntry* entry = LookupGlyph(index, render);

			if (!entry)
				continue;

			FreeTypeGlyph glyph;
			glyph.index = index;
			glyph.entry = entry;
			glyphs.push_back(glyph);
			if (entry->mArea && render)
				entry->mArea->state |= FREETYPE_GLYPH_AREA_LOCKED;
		}
	}
	else
	{
		for (unsigned int i = 0; i < string.length(); i++)
		{
			if (string[i] == '\n' || string[i] == '\r')
				continue;

			int index = FT_Get_Char_Index (face, string[i]);
			FreeTypeGlyphEntry* entry = LookupGlyph(index, render);

			if (!entry)
				continue;

			FreeTypeGlyph glyph;
			glyph.index = index;
			glyph.entry = entry;
			glyphs.push_back(glyph);
			if (entry->mArea && render)
				entry->mArea->state |= FREETYPE_GLYPH_AREA_LOCKED;
		}
	}
}

void FreeTypeScaledFont::DrawString(Graphics* g, int theX, int theY, const SexyString& theString,
				    const Color& theColor, const Rect& theClipRect,
				    bool drawShadow)
{
	if (!mBaseFont)
		return;

	LockFace();
	if (!mFace)
	{
		UnlockFace();
		return;
	}

	float x = theX, y = theY;

	Color aFontColor = theColor;
	Color aShadowColor(0, 0, 0);
	uint32 hsl = mApp->RGBToHSL(theColor.GetRed(), theColor.GetGreen(),
				    theColor.GetBlue());
	if (((hsl >> 16) & 0xff) < 127)
	    aShadowColor = Color(255, 255, 255);

	bool colorizeImages = g->GetColorizeImages();
	g->SetColorizeImages(true);
	Color anOrigColor = g->GetColor();
	g->SetColor(theColor);

	GlyphVector glyphs;
	GlyphsFromString(theString, glyphs, true);
	for (unsigned int i = 0; i < glyphs.size(); i++)
	{
		FreeTypeGlyphEntry* entry = glyphs[i].entry;

		if (!entry)
			continue;

		if (entry->mImage)
		{
			if (drawShadow)
			{
				g->SetColor(aShadowColor);
				g->DrawImage(entry->mImage,
					     (int)floor(x + entry->mXOffSet + 1),
					     (int)floor(y + entry->mYOffSet + 1),
					     Rect(entry->mArea->x, entry->mArea->y,
						  entry->mWidth, entry->mHeight));
				g->SetColor(aFontColor);
			}
			g->DrawImage(entry->mImage,
				     (int)floor(x + entry->mXOffSet),
				     (int)floor(y + entry->mYOffSet),
				     Rect(entry->mArea->x, entry->mArea->y,
					  entry->mWidth, entry->mHeight));
		}

		x += entry->mMetrics.x_advance;
		y += entry->mMetrics.y_advance;

		if (entry->mArea)
			entry->mArea->state &= ~FREETYPE_GLYPH_AREA_LOCKED;
	}

	g->SetColor(anOrigColor);
	g->SetColorizeImages(colorizeImages);

#if 0
	ImageLib::Image anImage;
	anImage.mWidth = mImages[0]->mWidth;
	anImage.mHeight = mImages[0]->mHeight;
	anImage.mBits = mImages[0]->GetBits();

	char filename[1024];
	snprintf (filename, sizeof(filename), "font-%p-%f.png", this, mSize);
	WritePNGImage(filename, &anImage );
	anImage.mBits = 0;
#endif
	UnlockFace();
}

int FreeTypeScaledFont::CharWidth(int theChar)
{
	if (!mBaseFont)
		return 0;

	LockFace();
	if (!mFace)
	{
		UnlockFace();
		return 0;
	}

	int width = 0;
	int index = FT_Get_Char_Index (mFace, theChar);
	FreeTypeExtents* metrics = LookupGlyphMetrics(index);
	if (metrics)
		width = metrics->x_advance;

	UnlockFace();
	return width;
}

int FreeTypeScaledFont::CharWidthKern(int theChar, int thePrevChar)
{
	return CharWidth(theChar);
}

FreeTypeScaledFont* FreeTypeScaledFont::Duplicate()
{
	return this;
}

void FreeTypeScaledFont::Ref()
{
	AutoCrit anAutoCrit(mRefCritSect);

	mRefCnt++;
}

void FreeTypeScaledFont::Unref()
{
	{
		AutoCrit anAutoCrit(mRefCritSect);

		if (--mRefCnt)
			return;
	}

	delete this;
}
void FreeTypeScaledFont::LockFace()
{
	if (mBaseFont)
		mFace = mBaseFont->LockFace(mSize, &mMatrix);
	else
		mFace = 0;
}

void FreeTypeScaledFont::UnlockFace()
{
	mFace = 0;
	ShrinkGlyphCache();
	if (mBaseFont)
		mBaseFont->UnlockFace();
}

FreeTypeGlyphEntry* FreeTypeScaledFont::LoadGlyph(FT_UInt index, bool render)
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

	if (render)
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
#if 0
			printf ("this: %p index: %d cached at (%d %d %d %d)\n",
				this, index, entry->mArea->x, entry->mArea->y,
				entry->mArea->width, entry->mArea->height);
#endif
			MemoryImage* anImage = (MemoryImage*)entry->mImage;
			uint32* bits = anImage->GetBits();
			if (bits)
			{
				bits += entry->mArea->y * anImage->GetWidth() + entry->mArea->x;
				unsigned char* srcbits = (unsigned char*)bitmap->buffer;

				int i;
				if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY)
				{
					for (i = 0; i < bitmap->rows; i++)
					{
						int j;
						for (j = 0; j < bitmap->width; j++)
							bits[j] = (((uint32)srcbits[j]) << 24) | 0xffffff;
						for (; j < entry->mArea->width; j++)
							bits[j] = 0;
						srcbits += bitmap->pitch;
						bits += anImage->GetWidth();
					}
					for (; i < entry->mArea->height; i++)
					{
						for (int j = 0; j < entry->mArea->width; j++)
							bits[j] = 0;
						bits += anImage->GetWidth();
					}
				}
				else
				{
					assert (bitmap->pixel_mode == FT_PIXEL_MODE_MONO);

					for (i = 0; i < bitmap->rows; i++)
					{
						int j;
						for (j = 0; j < bitmap->width; j++)
						{
							if (srcbits[(j >> 3)] & (0x80 >> (j & 7)))
								bits[j] = 0xffffffff;
							else
								bits[j] = 0x00ffffff;
						}
						for (; j < entry->mArea->width; j++)
							bits[j] = 0;
						srcbits += bitmap->pitch;
						bits += anImage->GetWidth();
					}
					for (; i < entry->mArea->height; i++)
					{
						for (int j = 0; j < entry->mArea->width; j++)
							bits[j] = 0;
						bits += anImage->GetWidth();
					}
				}

			}
			anImage->BitsChanged();

			entry->mWidth = bitmap->width;
			entry->mHeight = bitmap->rows;
		}
	}

	return entry;
}

FreeTypeGlyphEntry* FreeTypeScaledFont::LookupGlyph(FT_UInt index, bool render)
{
	GlyphMap::iterator it;

	it = mGlyphMap.find(index);
	if (it != mGlyphMap.end() && (!render || (render && it->second.mImage)))
		return &it->second;

	return LoadGlyph(index, render);
}

void FreeTypeScaledFont::RemoveGlyphImage(FT_UInt index)
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

void FreeTypeScaledFont::ShrinkGlyphCache(void)
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

FreeTypeExtents* FreeTypeScaledFont::LookupGlyphMetrics(FT_UInt index)
{
	FreeTypeGlyphEntry* entry = LookupGlyph(index);
	if (!entry)
		return 0;
	return &entry->mMetrics;
}

FreeTypeGlyphArea* FreeTypeScaledFont::FindAnAreaToRemove(FreeTypeGlyphArea* area)
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

FreeTypeGlyphArea* FreeTypeScaledFont::FindGlyphAreaInArea(int width, int height, FT_UInt index,
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
#if 0
		printf ("this %p kicked %d\n", this, area->index);
#endif

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
							 leftWidth, height,
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
		bool failed = false;

		for (unsigned i = 0; i < 4; i++) {
			if (area->children[i])
			{
				FreeTypeGlyphArea* child = area->children[i];
				if (child->width >= width && child->height >= height)
				{
					FreeTypeGlyphArea* result;
					result = FindGlyphAreaInArea (width, height, index,
								      child, remove);
					if (result)
						return result;

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

FreeTypeGlyphArea* FreeTypeScaledFont::FindGlyphArea(int width, int height, FT_UInt index, Image** image)
{
	if (!width || !height)
		return 0;

	width = (width + 4) & ~3;
	height = (height + 4) & ~3;

	FreeTypeGlyphArea* area;
	for (unsigned i = 0; i < MAX_CACHED_IMAGES; i++)
	{
		area = FindGlyphAreaInArea (width, height, index,  &mImageAreas[i]);
		if (area)
		{
			if (!mImages[i])
			{
				mImages[i] =
					mApp->CreateImage(1 << mImageSizeOrder[i], 1 << mImageSizeOrder[i]);
				mImages[i]->Palletize();
			}
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
			{
				mImages[i] =
					mApp->CreateImage(1 << mImageSizeOrder[i], 1 << mImageSizeOrder[i]);
				mImages[i]->Palletize();
			}
			*image = mImages[i];
			return area;
		}
	}

	return 0;
}
