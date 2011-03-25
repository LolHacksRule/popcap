#ifndef __NATIVEDISPLAY_H__
#define __NATIVEDISPLAY_H__
#include "Common.h"
#include "Rect.h"
#include "CritSect.h"
#include "Ratio.h"
#include "Event.h"
#include "SexyThread.h"
#include "InputInterface.h"

namespace Sexy
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Font;
class Image;
class SexyAppBase;
class Graphics;
class MemoryImage;
class Widget;
struct InputInfo;

class DelayedWork
{
public:
	virtual ~DelayedWork()
	{
	}

	virtual void Work(void) = 0;
};

enum KeyboardMode {
	KEYBOARD_NORMAL,
	KEYBOARD_PASSWORD,
	KEYBOARD_URL,
	KEYBOARD_EMAIL
};

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

	std::list<DelayedWork*>                 mWorkQueue;
	CritSect				mWorkQueuCritSect;
	Thread                                  mMainThread;

	CritSect				mTexMemSpaceCritSect;
	bool                                    mTraceTexMemAlloc;
	DWORD                                   mCurTexMemSpace;
	DWORD                                   mMaxTexMemSpace;
	SexyAppBase*				mApp;

 public:
	NativeDisplay();
	virtual ~NativeDisplay();

 public:
        virtual int                                Init();
	virtual bool                               CanReinit();
	virtual bool                               Reinit();

	virtual bool				   Is3DAccelerated();
	virtual bool				   Is3DAccelerationSupported();
	virtual bool				   Is3DAccelerationRecommended();

	virtual bool                               CanFullscreen();
	virtual bool                               CanWindowed();

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
	virtual bool				    SetCursorImage(Image* theImage,
								   int theHotX = 0,
								   int theHotY = 0);
	virtual void				    SetCursorPos(int theCursorX, int theCursorY);


 public:
	virtual void                                EnsureImageData(MemoryImage* theMemoryImage,
								    bool thePurgeBits = false,
								    bool theForce = false);
	virtual bool                                CreateImageData(MemoryImage* theMemoryImage);
	virtual void                                RemoveImageData(MemoryImage * theMemoryImage);

 public:
        virtual bool                                HasEvent();
        virtual bool                                GetEvent(struct Event & event);
	virtual bool                                GetInputInfo(InputInfo &anInfo);

	virtual bool                                ShowKeyboard(Widget* theWidget,
								 KeyboardMode mode,
								 const std::string &title,
								 const std::string &hint,
								 const std::string &initial);
	virtual void                                HideKeyboard();

 public:
	void                                        FlushWork(void);
	void                                        PushWork(DelayedWork* theWork);
	bool                                        IsWorkPending(void);

	virtual void                                Update();

 public:
	virtual void                                AllocTexMemSpace(DWORD theTexMemSize);
	virtual void                                FreeTexMemSpace(DWORD theTexMemSize);
	virtual bool                                EnsureTexMemSpace(DWORD theTexMemSize);

 private:
	DelayedWork*                                PopWork(void);
};

};


#endif
