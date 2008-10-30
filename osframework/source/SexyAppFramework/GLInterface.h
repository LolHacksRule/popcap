#ifndef __GLINTERFACE_H__
#define __GLINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "NativeDisplay.h"
#include "Rect.h"
#include "Ratio.h"

namespace Sexy
{

class SexyAppBase;
class GLImage;
class Image;
class MemoryImage;

typedef std::set<GLImage*> GLImageSet;

class GLInterface : public NativeDisplay
{
        friend class GLImage;
public:
	SexyAppBase*			        mApp;

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

	bool					mInitialized;
	bool					mIsWindowed;
	bool					mVideoOnlyDraw;
	ulong					mInitCount;

        GLImage*                                mScreenImage;

public:
	GLInterface(SexyAppBase* theApp);
	virtual ~GLInterface();

	virtual Image*				GetScreenImage();
	virtual int				Init();
	virtual void				Cleanup();

        virtual bool                            Redraw(Rect* theClipRect);
};

}

#endif //__GLINTERFACE_H__

