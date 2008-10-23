#include "DFBInterface.h"
#include "DFBImage.h"
#include "SexyAppBase.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"

#include <cstdio>

using namespace Sexy;

DFBInterface::DFBInterface(SexyAppBase* theApp)
{
	mApp = theApp;
	mDFB = NULL;
	mScreenImage = NULL;
	mRedAddTable = NULL;
	mGreenAddTable = NULL;
	mBlueAddTable = NULL;
	mInitialized = false;
	mVideoOnlyDraw = false;
	mScanLineFailCount = 0;

	//TODO: Standards, anyone?
	mNextCursorX = 0;
	mNextCursorY = 0;
	mCursorX = 0;
	mCursorY = 0;
	mInRedraw = false;
	//mCursorWidth = 54;
	//mCursorHeight = 54;
	mCursorWidth = 64;
	mCursorHeight = 64;
	mHasOldCursorArea = false;
	mNewCursorAreaImage = NULL;
	mOldCursorAreaImage = NULL;
	mInitCount = 0;
	mRefreshRate = 60;
	mMillisecondsPerFrame = 1000 / mRefreshRate;
}

DFBInterface::~DFBInterface()
{
	delete [] mRedAddTable;
	delete [] mGreenAddTable;
	delete [] mBlueAddTable;

	Cleanup();

	if (mDFB)
		mDFB->Release(mDFB);
}

std::string DFBInterface::ResultToString(int theResult)
{
	return "RESULT_UNKNOWN";
}

Image* DFBInterface::GetScreenImage()
{
	return mScreenImage;
}

int DFBInterface::Init(void)
{
	Cleanup();

	AutoCrit anAutoCrit(mCritSect);
	mInitialized = false;

	if (!mDFB) {
		DirectFBInit(NULL, NULL);
		DirectFBCreate(&mDFB);
		mDFB->SetCooperativeLevel(mDFB, DFSCL_FULLSCREEN);
	}

	IDirectFBSurface * surface;
	DFBSurfaceDescription surface_desc;
	int width, height;
	int ret;

	surface_desc.flags = DSDESC_CAPS;
	surface_desc.caps =
		(DFBSurfaceCapabilities)(DSCAPS_PRIMARY | DSCAPS_VIDEOONLY | DSCAPS_DOUBLE);
	ret = mDFB->CreateSurface(mDFB, &surface_desc, &surface);
	if (ret != DFB_OK) {
		surface_desc.caps =
			(DFBSurfaceCapabilities)(surface_desc.caps  & ~DSCAPS_DOUBLE);
		ret = mDFB->CreateSurface(mDFB, &surface_desc, &surface);
		DBG_ASSERT (ret == DFB_OK);
	}
	surface->GetSize (surface, &width, &height);
	surface->Clear (surface, 0, 0, 0, 0);
	mPrimarySurface = surface;
	mWidth = width;
	mHeight = height;

	mScreenImage = new DFBImage(surface, this);
	mInitCount++;
	mInitialized = true;

	return 0;
}

void DFBInterface::SetVideoOnlyDraw(bool videoOnlyDraw)
{
	AutoCrit anAutoCrit(mCritSect);

	mVideoOnlyDraw = videoOnlyDraw;
}

void DFBInterface::RemapMouse(int& theX, int& theY)
{
	if (mInitialized)
	{
		theX = ( theX - mPresentationRect.mX ) * mWidth / mPresentationRect.mWidth;
		theY = ( theY - mPresentationRect.mY ) * mHeight / mPresentationRect.mHeight;
	}
}

ulong DFBInterface::GetColorRef(ulong theRGB)
{
	return theRGB;
}

void DFBInterface::AddImage(Image* theImage)
{
	//AutoCrit anAutoCrit(mCritSect);

	mImageSet.insert((DFBImage*)theImage);
}

void DFBInterface::RemoveImage(Image* theImage)
{
	//AutoCrit anAutoCrit(mCritSect);

	DFBImageSet::iterator anItr = mImageSet.find((DFBImage*)theImage);
	if (anItr != mImageSet.end())
		mImageSet.erase(anItr);
}

void DFBInterface::Remove3DData(MemoryImage* theImage) // for 3d texture cleanup
{
}

void DFBInterface::Cleanup()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	if (mOldCursorAreaImage != NULL)
	{
		delete mOldCursorAreaImage;
		mOldCursorAreaImage = NULL;
	}

	if (mNewCursorAreaImage != NULL)
	{
		delete mNewCursorAreaImage;
		mNewCursorAreaImage = NULL;
	}

	if (mScreenImage != NULL)
	{
		delete mScreenImage;
		mScreenImage = NULL;
	}

	if (mPrimarySurface)
		mPrimarySurface = NULL;

	if (mDFB)
		mDFB->Release(mDFB);
}

bool DFBInterface::Redraw(Rect* theClipRect)
{
	AutoCrit anAutoCrit(mCritSect);

	if (!mInitialized)
		return false;

	if (mPrimarySurface)
		mPrimarySurface->Flip(mPrimarySurface, 0, DSFLIP_NONE);

	return true;
}

bool DFBInterface::SetCursorImage(Image* theImage)
{
	AutoCrit anAutoCrit(mCritSect);

	if (mCursorImage != theImage)
	{
		// Wait until next Redraw or cursor move to draw new cursor
		mCursorImage = theImage;
		return true;
	}
	else
		return false;
}

void DFBInterface::SetCursorPos(int theCursorX, int theCursorY)
{
	mNextCursorX = theCursorX;
	mNextCursorY = theCursorY;

	if (mInRedraw)
		return;

	AutoCrit anAutoCrit(mCritSect);

	if (mHasOldCursorArea)
	{
		;
	}
	else
	{
		mCursorX = theCursorX;
		mCursorY = theCursorY;
	}
}

IDirectFBSurface* DFBInterface::CreateDFBSurface(int width, int height)
{
	IDirectFBSurface * aSurface;

	if (!mInitialized)
	{
		return 0;
	}
	else if (width && height)
	{
		AutoCrit anAutoCrit(mCritSect);

		DFBSurfaceDescription desc;

		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH |
							  DSDESC_HEIGHT |
							  DSDESC_PIXELFORMAT);
		desc.width = width;
		desc.height = height;
		desc.pixelformat = DSPF_ARGB;
		if (mDFB->CreateSurface(mDFB, &desc, &aSurface))
			return 0;
	} else {
		aSurface = 0;
	}
	return aSurface;
}

Image* DFBInterface::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	IDirectFBSurface * aSurface;

	if (!mInitialized)
	{
		return 0;
	}
	else if (width && height)
	{
		AutoCrit anAutoCrit(mCritSect);

		DFBSurfaceDescription desc;

		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH |
							  DSDESC_HEIGHT |
							  DSDESC_PIXELFORMAT);
		desc.width = width;
		desc.height = height;
		desc.pixelformat = DSPF_ARGB;
		if (mDFB->CreateSurface(mDFB, &desc, &aSurface))
			return 0;
	} else {
		aSurface = 0;
	}
	return new DFBImage(aSurface, this);
}
