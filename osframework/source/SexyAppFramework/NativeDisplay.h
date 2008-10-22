#ifndef __NATIVEDISPLAY_H__
#define __NATIVEDISPLAY_H__
#include "Common.h"
#include "Rect.h"

namespace Sexy
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Font;
class Image;
class SexyAppBase;
class NativeDisplay
{
public:
	int						mRGBBits;
	ulong					mRedMask;
	ulong					mGreenMask;
	ulong					mBlueMask;
	int						mRedBits;
	int						mGreenBits;
	int						mBlueBits;
	int						mRedShift;
	int						mGreenShift;
	int						mBlueShift;

public:
	NativeDisplay();
	virtual ~NativeDisplay();

 public:
        virtual int                                Init();
        virtual Font *                             CreateFont(SexyAppBase * theApp,
                                                              const std::string theFace,
                                                              int thePointSize,
                                                              bool bold = false,
                                                              bool italics = false,
                                                              bool underline = false);

	virtual Image*			            GetScreenImage() = 0;
	virtual bool				    Redraw(Rect* theClipRect = 0) = 0;
	virtual void				    RemapMouse(int& theX, int& theY) = 0;

	virtual bool				    SetCursorImage(Image* theImage)= 0;
	virtual void				    SetCursorPos(int theCursorX, int theCursorY) = 0;
};

};


#endif
