#ifndef __NATIVE_FONT_H__
#define __NATIVE_FONT_H__

#include "Common.h"
#include "Rect.h"
#include "Color.h"
#include "Font.h"

namespace Sexy
{

class Graphics;
class ImageFont;

class SEXY_EXPORT NativeFont : public Font
{
public:
        NativeFont();
	virtual ~NativeFont();

	ImageFont*			CreateImageFont();
	virtual int			StringWidth(const SexyString& theString);
	virtual void			DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect);

	virtual Font*			Duplicate();
};

}

#endif //__FONT_H__
