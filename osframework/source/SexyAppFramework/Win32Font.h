#ifndef __WIN32_FONT_H__
#define __WIN32_FONT_H__

#include "Font.h"

namespace Sexy
{

class ImageFont;
class SexyAppBase;

class Win32Font : public Font
{
public:
	HFONT					mHFont;
	SexyAppBase*			        mApp;
	bool					mDrawShadow;
	bool					mSimulateBold;

	void Init(SexyAppBase* theApp, const std::string& theFace, int thePointSize, int theScript, bool bold, bool italics, bool underline, bool useDevCaps);

public:
	Win32Font(const std::string& theFace, int thePointSize, bool bold = false, bool italics = false, bool underline = false);
	Win32Font(SexyAppBase* theApp, const std::string& theFace, int thePointSize, int theScript = ANSI_CHARSET, bool bold = false, bool italics = false, bool underline = false);
	Win32Font(const Win32Font& theWin32Font);

	virtual ~Win32Font();

	virtual int			        StringWidth(const SexyString& theString);
	virtual void			        DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect);

	virtual Font*			        Duplicate();
};

}

#endif

