#ifndef __DFBFONT_H__
#define __DFBFONT_H__

#include "Font.h"
#include <directfb.h>

namespace Sexy
{

class ImageFont;
class SexyAppBase;

class DFBFont : public Font
{
public:
	SexyAppBase*			mApp;
	bool					mDrawShadow;
	bool					mSimulateBold;

	void Init(SexyAppBase* theApp, const std::string& theFace, int thePointSize, bool bold, bool italics, bool underline);

public:
	DFBFont(const std::string& theFace, int thePointSize, bool bold = false, bool italics = false, bool underline = false);
	DFBFont(SexyAppBase* theApp, const std::string& theFace, int thePointSize, bool bold, bool italics, bool underline);
	DFBFont(const DFBFont& theDFBFont);

	virtual ~DFBFont();

	ImageFont*			CreateImageFont();
	virtual int			StringWidth(const SexyString& theString);
	virtual void			DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect);

	virtual Font*			Duplicate();
};

}

#endif //__SYSFONT_H__
