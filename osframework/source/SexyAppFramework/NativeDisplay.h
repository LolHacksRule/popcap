#ifndef __NATIVEDISPLAY_H__
#define __NATIVEDISPLAY_H__
#include "Common.h"
#include "Rect.h"
#include "CritSect.h"
#include "Ratio.h"
#include "Event.h"

namespace Sexy
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Font;
class Image;
class SexyAppBase;
class Graphics;
class MemoryImage;

class NativeDisplay
{
public:
	CritSect				mCritSect;

	int					mRGBBits;
	ulong					mRedMask;
	ulong					mGreenMask;
	ulong					mBlueMask;
	int					mRedBits;
	int					mGreenBits;
	int					mBlueBits;
	int					mRedShift;
	int					mGreenShift;
	int				        mBlueShift;

        int                                     mCursorX;
        int                                     mCursorY;

	int					mWidth;
	int					mHeight;
	Ratio					mAspect;
	int					mDesktopWidth;
	int					mDesktopHeight;
	Ratio					mDesktopAspect;
	bool					mIsWidescreen;
	int					mDisplayWidth;
	int					mDisplayHeight;
	Ratio					mDisplayAspect;

	Rect					mPresentationRect;
public:
	NativeDisplay();
	virtual ~NativeDisplay();

 public:
        virtual int                                Init();

	virtual bool				   Is3DAccelerated();
	virtual bool				   Is3DAccelerationSupported();
	virtual bool				   Is3DAccelerationRecommended();

        virtual Font *                             CreateFont(SexyAppBase * theApp,
                                                              const std::string theFace,
                                                              int thePointSize,
                                                              bool bold = false,
                                                              bool italics = false,
                                                              bool underline = false);
        virtual Image *                             CreateImage(SexyAppBase * theApp,
                                                                int width, int height);

	virtual Image*			            GetScreenImage() = 0;
	virtual bool				    Redraw(Rect* theClipRect = 0) = 0;
	virtual void				    RemapMouse(int& theX, int& theY) = 0;

        virtual bool                                UpdateCursor(int theCursorX, int theCursorY);
        virtual bool                                DrawCursor(Graphics* g);

        virtual bool                                EnableCursor(bool enable);
	virtual bool				    SetCursorImage(Image* theImage, int theHotX = 0, int theHotY = 0);
	virtual void				    SetCursorPos(int theCursorX, int theCursorY);

	virtual void                                RemoveImageData(MemoryImage * theMemoryImage);

 public:
        virtual bool                                HasEvent();
        virtual bool                                GetEvent(struct Event & event);
};

};


#endif
