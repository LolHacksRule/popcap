#include "NativeFont.h"
#include "Image.h"

using namespace Sexy;

NativeFont::NativeFont()
{
}

NativeFont::~NativeFont()
{
}

ImageFont* NativeFont::CreateImageFont()
{
	return 0;
}

int  NativeFont::StringWidth(const SexyString& theString)
{
	return 0;
}

void NativeFont::DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect)
{
}

Font* NativeFont::Duplicate()
{
	return 0;
}
