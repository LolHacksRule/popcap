#include "NativeDisplay.h"
#include "MemoryImage.h"
#include "Font.h"

#ifdef SEXY_FREETYPE_FONT
#include "FreeTypeFont.h"
#endif

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

	mCursorX = 0;
	mCursorY = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NativeDisplay::~NativeDisplay()
{
}

bool NativeDisplay::Is3DAccelerated()
{
    return false;
}

bool NativeDisplay::Is3DAccelerationSupported()
{
    return false;
}

bool NativeDisplay::Is3DAccelerationRecommended()
{
    return false;
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
#ifdef SEXY_FREETYPE_FONT
	return new FreeTypeFont(theFace, thePointSize * 96 / 72.0f, bold,
				italics, underline);
#endif
	return 0;
}

Image* NativeDisplay::CreateImage(SexyAppBase * theApp,
				  int width, int height)
{
	return 0;
}

bool  NativeDisplay::EnableCursor(bool enable)
{
	return false;
}

bool  NativeDisplay::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	return false;
}

void NativeDisplay::SetCursorPos(int theCursorX, int theCursorY)
{
}

bool  NativeDisplay::HasEvent()
{
	return false;
}

bool NativeDisplay::GetEvent(struct Event & event)
{
	event.type = EVENT_NONE;
	return false;
}

bool NativeDisplay::UpdateCursor(int theCursorX, int theCursorY)
{
	return false;
}

bool NativeDisplay::DrawCursor(Sexy::Graphics* g)
{
	return false;
}

void NativeDisplay::RemoveImageData(MemoryImage * theMemoryImage)
{
}
