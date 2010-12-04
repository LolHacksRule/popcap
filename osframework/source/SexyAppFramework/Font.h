#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"
#include "Rect.h"
#include "Color.h"

#include <string>
#include <vector>

namespace Sexy
{

class Graphics;
class Glyph
{
 public:
	float mX;
	float mY;

	int   mIndex;

	int   mWidth;
	int   mHeight;
	int   mAdvanceX;
	int   mAdvanceY;

	int   mReserved[4];
	void* mNativeData[4];

 public:
	Glyph()
	{
		mX = 0;
		mY = 0;
		mIndex = 0;
		mWidth = 0;
		mHeight = 0;
		mAdvanceX = 0;
		mAdvanceY = 0;
		memset (mReserved, 0, sizeof(mReserved));
		memset (mNativeData, 0, sizeof(mNativeData));
	}

	Glyph(const Glyph &other)
	{
		mX = other.mX;
		mY = other.mY;
		mIndex = other.mIndex;
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mAdvanceX = other.mAdvanceX;
		mAdvanceY = other.mAdvanceY;
		memcpy (mReserved, other.mReserved, sizeof(mReserved));
		memcpy (mNativeData, other.mNativeData, sizeof(mNativeData));
	}
};
typedef std::vector<Glyph> GlyphVector;

class Font
{
public:
	int						mAscent;
	int						mAscentPadding; // How much space is above the avg uppercase char
	int						mHeight;
	int						mLineSpacingOffset; // This plus height should get added between lines
	bool                                            mSupportUnicode;

	bool					        mDrawShadow;
	bool					        mSimulateBold;
	bool					        mOutLine;

public:
	Font();
	Font(const Font& theFont);
	virtual ~Font();

	virtual bool                            IsSupportUnicode();
	virtual int				GetAscent();
	virtual int				GetAscentPadding();
	virtual int				GetDescent();
	virtual int				GetHeight();
	virtual int				GetLineSpacingOffset();
	virtual int				GetLineSpacing();
	virtual int				StringWidth(const std::string& theString);
	virtual int				StringWidth(const Sexy::WString& theString);

	virtual int				CharWidth(int theChar);
	virtual int				CharWidthKern(int theChar, int thePrevChar);

	virtual void			DrawString(Graphics* g, int theX, int theY, const std::string& theString, const Color& theColor, const Rect& theClipRect, bool unicode = false);
	virtual void			DrawString(Graphics* g, int theX, int theY, const Sexy::WString& theString, const Color& theColor, const Rect& theClipRect);

	virtual  bool                            StringToGlyphs(const Sexy::WString &theString,
								GlyphVector &theGlyphs);
	virtual  void                            DrawGlyphs(Graphics *g, int theX, int theY,
							    GlyphVector& theGlyphs,
							    size_t from, size_t length,
							    const Color &theColor,
							    const Rect& theClipRect);

	virtual bool                             IsComposited();

	virtual Font*			Duplicate() = 0;
};

}

#endif //__FONT_H__
