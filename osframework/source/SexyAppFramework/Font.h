#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"
#include "Rect.h"
#include "Color.h"

namespace Sexy
{

class Graphics;

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
	bool                                            mOutLine;

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
	virtual int				StringWidth(const SexyString& theString);
	virtual int				CharWidth(int theChar);
	virtual int				CharWidthKern(int theChar, int thePrevChar);

	virtual void			DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect);

	virtual Font*			Duplicate() = 0;
};

}

#endif //__FONT_H__
