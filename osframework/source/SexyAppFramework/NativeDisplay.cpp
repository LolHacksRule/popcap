#include "NativeDisplay.h"
#include "Font.h"

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NativeDisplay::NativeDisplay()
{
	mRGBBits = 0;

	mRedMask = 0;
	mGreenMask = 0;
	mBlueMask = 0;

	mRedBits = 0;
	mGreenBits = 0;
	mBlueBits = 0;

	mRedShift = 0;
	mGreenShift = 0;
	mBlueShift = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NativeDisplay::~NativeDisplay()
{
}

int NativeDisplay::Init()
{
	return -1;
}


Font* NativeDisplay::CreateFont(SexyAppBase * theApp,
				 const std::string theFace,
				 int thePointSize,
				 bool bold,
				 bool italics,
				 bool underline)
{
	return 0;
}

Image* NativeDisplay::CreateImage(SexyAppBase * theApp,
				  int width, int height)
{
	return 0;
}
