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

class DFBInterface : public NativeDisplay
{
public:
	SexyAppBase*			        mApp;

        IDirectFB                             * mDFB;
	IDirectFBSurface                      * mPrimarySurface;

	bool					mIs3D;

	CritSect				mCritSect;
	bool					mInRedraw;
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
	int				        mFullscreenBits;
	DWORD					mRefreshRate;
	DWORD					mMillisecondsPerFrame;
	int					mScanLineFailCount;

	int*					mRedAddTable;
	int*					mGreenAddTable;
	int*					mBlueAddTable;

	uint32					mRedConvTable[256];
	uint32					mGreenConvTable[256];
	uint32					mBlueConvTable[256];

	bool					mInitialized;
	bool					mIsWindowed;
	DFBImage*				mScreenImage;
	DFBImageSet				mImageSet;
	bool					mVideoOnlyDraw;
	ulong					mInitCount;

	int						mCursorWidth;
	int						mCursorHeight;
	int						mNextCursorX;
	int						mNextCursorY;
	int						mCursorX;
	int						mCursorY;
	Image*					mCursorImage;
	bool					mHasOldCursorArea;
	DFBImage*				mOldCursorAreaImage;
	DFBImage*				mNewCursorAreaImage;

	std::string				mErrorString;

public:
	ulong					GetColorRef(ulong theRGB);
	void					AddImage(Image* theImage);
	void					RemoveImage(Image* theImage);
	void					Remove3DData(MemoryImage* theImage); // for 3d texture cleanup

	void					Cleanup();

public:
	DFBInterface(SexyAppBase* theApp);
	virtual ~DFBInterface();

	static std::string		ResultToString(int theResult);

	Image*				        GetScreenImage();
	int					Init();
	bool					Redraw(Rect* theClipRect = NULL);
	void					SetVideoOnlyDraw(bool videoOnly);
	void					RemapMouse(int& theX, int& theY);

	bool					SetCursorImage(Image* theImage);
	void					SetCursorPos(int theCursorX, int theCursorY);
};

}

#endif //__DFBINTERFACE_H__

