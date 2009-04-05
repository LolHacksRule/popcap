#include "Win32Font.h"
#include "SexyAppBase.h"
#include "Graphics.h"
#include "ImageFont.h"
#include "MemoryImage.h"
#include "WidgetManager.h"

#include <stdlib.h>

using namespace Sexy;

Win32Font::Win32Font(const std::string& theFace, int thePointSize, bool bold, bool italics, bool underline)
{
	Init(gSexyAppBase,theFace,thePointSize,ANSI_CHARSET,bold,italics,underline,false);
}

Win32Font::Win32Font(SexyAppBase* theApp, const std::string& theFace, int thePointSize, int theScript, bool bold, bool italics, bool underline)
{
	Init(theApp,theFace,thePointSize,theScript,bold,italics,underline,true);
}

void Win32Font::Init(SexyAppBase* theApp, const std::string& theFace, int thePointSize, int theScript, bool bold, bool italics, bool underline, bool useDevCaps)
{
	mApp = theApp;

	HDC aDC = ::GetDC(mApp->mHWnd);

	int aHeight = -MulDiv(thePointSize, useDevCaps?GetDeviceCaps(aDC, LOGPIXELSY):96, 72);

	mHFont = CreateFontA(aHeight, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, italics, underline,
			false, theScript, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, theFace.c_str());

	TEXTMETRIC aTextMetric;
	HFONT anOldFont = (HFONT) SelectObject(aDC, mHFont);
	GetTextMetrics(aDC, &aTextMetric);
	SelectObject(aDC, anOldFont);
	ReleaseDC(mApp->mHWnd, aDC);

	mHeight = aTextMetric.tmHeight;
	mAscent = aTextMetric.tmAscent;

	mDrawShadow = false;
	mSimulateBold = false;
}

Win32Font::Win32Font(const Win32Font& theWin32Font)
{
	LOGFONT aLogFont;

	GetObject(theWin32Font.mHFont, sizeof(LOGFONT), &aLogFont);
	mHFont = CreateFontIndirect(&aLogFont);
	mApp = theWin32Font.mApp;
	mHeight = theWin32Font.mHeight;
	mAscent = theWin32Font.mAscent;

	mDrawShadow = false;
	mSimulateBold = false;
}

Win32Font::~Win32Font()
{
	DeleteObject(mHFont);
}

int	Win32Font::StringWidth(const SexyString& theString)
{
	HDC aDC = ::GetDC(mApp->mHWnd);
	HFONT anOldFont = (HFONT)::SelectObject(aDC, mHFont);
	int aWidth = 0;

#ifdef _USE_WIDE_STRING
	if (CheckFor98Mill())
	{
		SIZE aSize = { 0, 0 };
		GetTextExtentPoint32W(aDC, theString.c_str(), theString.length(), &aSize);
		aWidth = int(aSize.cx);
	}
	else
#endif
	{
		RECT aRect = {0, 0, 0, 0};
		DrawTextEx(aDC, (SexyChar*)theString.c_str(), theString.length(), &aRect, DT_CALCRECT | DT_NOPREFIX, NULL);
		aWidth = aRect.right;
	}

	::SelectObject(aDC, anOldFont);
	::ReleaseDC(mApp->mHWnd, aDC);

	return aWidth;
}

void Win32Font::DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect)
{
	HDC aDC = CreateCompatibleDC(NULL);
	HFONT anOldFont = (HFONT) SelectObject(aDC, mHFont);

	int aWidth = StringWidth(theString);
	int aHeight = mHeight;

	BITMAPINFOHEADER bih;
	memset(&bih, 0, sizeof(bih));

	bih.biPlanes = 1;
	bih.biWidth = aWidth;
	bih.biHeight = -aHeight;
	bih.biCompression = BI_RGB;
	bih.biBitCount = 32;
	bih.biSize = sizeof(BITMAPINFOHEADER);

	uint32* whiteBits, * blackBits;
	HBITMAP whiteBitmap = (HBITMAP)CreateDIBSection(aDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&whiteBits, NULL, 0);
	HBITMAP blackBitmap = (HBITMAP)CreateDIBSection(aDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&blackBits, NULL, 0);

	RECT rc = { 0, 0, aWidth, aHeight };

#define DRAW_BITMAP(bmp, brush)						\
	{								\
		HBITMAP oldBmp = (HBITMAP) SelectObject(aDC, bmp);	\
		::FillRect(aDC, &rc, brush);				\
		SetBkMode(aDC, TRANSPARENT);				\
									\
		if (mDrawShadow)					\
		{							\
			SetTextColor(aDC, RGB(0,0,0));			\
			TextOut(aDC, 1, 1, theString.c_str(), theString.length()); \
			if (mSimulateBold)				\
				TextOut(aDC, 2, 1, theString.c_str(), theString.length()); \
		}							\
		SetTextColor(aDC, RGB(theColor.GetRed(), theColor.GetGreen(), theColor.GetBlue())); \
		TextOut(aDC, 0, 0, theString.c_str(), theString.length()); \
		if (mSimulateBold)					\
			TextOut(aDC, 1, 0, theString.c_str(), theString.length()); \
									\
		SelectObject(aDC, oldBmp);				\
	}

	DRAW_BITMAP(whiteBitmap, (HBRUSH)GetStockObject(WHITE_BRUSH));
	DRAW_BITMAP(blackBitmap, (HBRUSH)GetStockObject(BLACK_BRUSH));

	SelectObject(aDC, anOldFont);

	MemoryImage* aTempImage = (MemoryImage*)mApp->CreateImage(aWidth, aHeight);

	int aCount = aHeight * aWidth;
	uint32* ptr1 = whiteBits, *ptr2 = blackBits;
	while (aCount > 0)
	{
		if (*ptr1 == *ptr2)
			*ptr1 |= 0xFF000000;
		else if ((*ptr1 & 0xFFFFFF) != 0xFFFFFF || (*ptr2 & 0xFFFFFF) != 0x000000) // if not the background of either, it's a 'blend'
		{
			int ba = 255 + (*ptr2 & 0xFF) - (*ptr1 & 0xFF);
			int ga = 255 + ((*ptr2 >> 8) & 0xFF) - ((*ptr1 >> 8) & 0xFF);
			int ra = 255 + ((*ptr2 >> 16) & 0xFF) - ((*ptr1 >> 16) & 0xFF);
			int aBlue = 255 * (*ptr2 & 0xFF) / ba;
			int aGreen = 255 * ((*ptr2 >> 8) & 0xFF) / ga;
			int aRed = 255 * ((*ptr2 >> 16) & 0xFF) / ra;
			int anAlpha = std::min(ra, std::min(ga, ba));
			*ptr1 = (aBlue) | (aGreen << 8) | (aRed << 16) | (anAlpha << 24);
		}
		else *ptr1 &= 0;

		ptr1++;
		ptr2++;
		--aCount;
	}

	memcpy(aTempImage->GetBits(), whiteBits, aWidth * aHeight * sizeof(uint32));
	g->DrawImage(aTempImage, theX, theY - mAscent);

	delete aTempImage;

	DeleteObject(whiteBitmap);
	DeleteObject(blackBitmap);
	DeleteDC(aDC);
}

Font* Win32Font::Duplicate()
{
	return new Win32Font(*this);
}
