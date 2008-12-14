#include "DFBFont.h"
#include "DFBImage.h"
#include "SexyAppBase.h"
#include "Graphics.h"
#include "ImageFont.h"
#include "MemoryImage.h"
#include "DFBInterface.h"
#include "WidgetManager.h"

#include <stdlib.h>

using namespace Sexy;

DFBFont::DFBFont(const std::string& theFace, int thePointSize, bool bold, bool italics, bool underline)
{
	Init(gSexyAppBase,theFace,thePointSize,bold,italics,underline);
}

DFBFont::DFBFont(SexyAppBase* theApp, const std::string& theFace, int thePointSize, bool bold, bool italics, bool underline)
{
	Init(theApp,theFace,thePointSize,bold,italics,underline);
}

void DFBFont::Init(SexyAppBase* theApp, const std::string& theFace, int thePointSize, bool bold, bool italics, bool underline)
{
	mApp = theApp;

	mHeight = 0;
	mAscent = 0;

	mDrawShadow = false;
	mSimulateBold = false;
}

DFBFont::DFBFont(const DFBFont& theDFBFont)
{
	mApp = theDFBFont.mApp;
	mHeight = theDFBFont.mHeight;
	mAscent = theDFBFont.mAscent;

	mDrawShadow = false;
	mSimulateBold = false;
}

DFBFont::~DFBFont()
{
}

ImageFont* DFBFont::CreateImageFont()
{
	return 0;
}

int	DFBFont::StringWidth(const SexyString& theString)
{
	return 0;
}

void DFBFont::DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect)
{
}

Font* DFBFont::Duplicate()
{
	return new DFBFont(*this);
}
