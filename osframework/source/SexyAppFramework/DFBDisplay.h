#ifndef __DFBINTERFACE_H__
#define __DFBINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "NativeDisplay.h"
#include "Rect.h"
#include "Ratio.h"

#include <directfb.h>

namespace Sexy
{

class SexyAppBase;
class DFBImage;
class Image;
class MemoryImage;

typedef std::set<DFBImage*> DFBImageSet;

class DFBDisplay : public NativeDisplay
{
	friend class DFBImage;
	friend class DFBImageData;
public:
	SexyAppBase*				mApp;

	IDirectFB			      * mDFB;
	IDirectFBSurface		      * mPrimarySurface;

	bool					mIs3D;

	bool					mInRedraw;

	bool					mInitialized;
	DFBImage*				mScreenImage;
	DFBImageSet				mImageSet;
	bool					mVideoOnlyDraw;
	ulong					mInitCount;

public:
	ulong					GetColorRef(ulong theRGB);
	void					AddImage(Image* theImage);
	void					RemoveImage(Image* theImage);

	void					Cleanup();

public:
	DFBDisplay(SexyAppBase* theApp);
	virtual ~DFBDisplay();

	Image*					GetScreenImage();
	int					Init();
	bool					Redraw(Rect* theClipRect = NULL);
	void					SetVideoOnlyDraw(bool videoOnly);
	void					RemapMouse(int& theX, int& theY);

	virtual bool				EnableCursor(bool enable);
	virtual bool				SetCursorImage(Image* theImage, int theHotX = 0, int theHotY = 0);
	virtual void				SetCursorPos(int theCursorX, int theCursorY);
        virtual bool                            UpdateCursor(int theCursorX, int theCursorY);
        virtual bool                            DrawCursor(Graphics* g);

	virtual Image*				CreateImage(SexyAppBase * theApp,
							    int width, int height);
	virtual bool				HasEvent();
	virtual bool				GetEvent(struct Event &event);

	virtual bool                            CreateImageData(MemoryImage *theImage);
	virtual void                            RemoveImageData(MemoryImage *theImage);

	bool                                    IsMainThread(void);
	void                                    DelayedReleaseSurface(IDirectFBSurface* surface);

 private:
	IDirectFBSurface*			CreateDFBSurface(int width, int height);

 private:
	IDirectFBInputDevice		      * mInput;
	IDirectFBEventBuffer		      * mBuffer;
	int					mMouseX;
	int					mMouseY;
        int                                     mCursorX;
        int                                     mCursorY;
        int                                     mCursorOldX;
        int                                     mCursorOldY;
        int                                     mCursorHotX;
        int                                     mCursorHotY;
        bool                                    mCursorEnabled;
        bool                                    mSoftCursor;
        MemoryImage*                            mCursorImage;
        IDirectFBSurface                      * mDFBCursorImage;

	IDirectFBDisplayLayer		      * mLayer;
	IDirectFBWindow			      * mWindow;
};

}

#endif //__DFBINTERFACE_H__

