#include <math.h>

#include "DFBImage.h"
#include "DFBInterface.h"
#include "Rect.h"
#include "Graphics.h"
#include "SexyAppBase.h"
#include "Debug.h"
#include "PerfTimer.h"

using namespace Sexy;

DFBImage::DFBImage(DFBInterface* theInterface) :
	MemoryImage(theInterface->mApp),
	mSurface(0)
{
	mInterface = theInterface;
	Init();
}

DFBImage::DFBImage() :
	MemoryImage(gSexyAppBase)
{
	mInterface = dynamic_cast<DFBInterface *>(gSexyAppBase->mDDInterface);
	Init();
}

DFBImage::~DFBImage()
{
	mInterface->RemoveImage(this);

	DBG_ASSERTE(mLockCount == 0);
}

void DFBImage::Init()
{
	mInterface->AddImage(this);

	mNoLock = false;
	mVideoMemory = false;
	mFirstPixelTrans = false;
	mWantDDSurface = false;
	mDrawToBits = false;
	mSurfaceSet = false;

	mLockCount = 0;
}

bool DFBImage::LockSurface()
{
	if (mLockCount == 0)
	{
		//lock
	}

	mLockCount++;

	DBG_ASSERTE(mLockCount < 8);

	return true;
}

bool DFBImage::UnlockSurface()
{
	--mLockCount;

	if (mLockCount == 0)
	{
		//unlock
	}

	DBG_ASSERTE(mLockCount >= 0);

	return true;
}

void DFBImage::DeleteSurface()
{
}

void DFBImage::ReInit()
{
	MemoryImage::ReInit();
}

void DFBImage::PurgeBits()
{
	if (mSurfaceSet)
		return;

	mPurgeBits = true;

	CommitBits();
	MemoryImage::PurgeBits();
}

void DFBImage::DeleteAllNonSurfaceData()
{
	delete [] mBits;
	mBits = NULL;

	delete [] mNativeAlphaData;
	mNativeAlphaData = NULL;

	delete [] mRLAdditiveData;
	mRLAdditiveData = NULL;

	delete [] mRLAlphaData;
	mRLAlphaData = NULL;

	delete [] mColorTable;
	mColorTable = NULL;

	delete [] mColorIndices;
	mColorIndices = NULL;
}

void DFBImage::DeleteNativeData()
{
	if (mSurfaceSet)
		return;

	MemoryImage::DeleteNativeData();
	DeleteSurface();
}

void DFBImage::DeleteExtraBuffers()
{
	if (mSurfaceSet)
		return;

	MemoryImage::DeleteExtraBuffers();
	DeleteSurface();
}

void DFBImage::SetVideoMemory(bool wantVideoMemory)
{
}

void DFBImage::RehupFirstPixelTrans()
{
}

bool DFBImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool convex)
{
}

void DFBImage::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
}

void DFBImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
}

void DFBImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
}

void DFBImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
}

void DFBImage::NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
}

void DFBImage::AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
}

void DFBImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
}

void DFBImage::CommitBits()
{
	MemoryImage::CommitBits();
}

void DFBImage::Create(int theWidth, int theHeight)
{
	delete [] mBits;

	mWidth = theWidth;
	mHeight = theHeight;

	mBits = NULL;

	BitsChanged();
}

void DFBImage::BitsChanged()
{
	MemoryImage::BitsChanged();
}

uint32* DFBImage::GetBits()
{
	return 0;
}

void DFBImage::NormalFillRect(const Rect& theRect, const Color& theColor)
{
}

void DFBImage::AdditiveFillRect(const Rect& theRect, const Color& theColor)
{
}

void DFBImage::NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
}

void DFBImage::NormalBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
}

void DFBImage::AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
}

void DFBImage::AdditiveBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
}

void DFBImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
}

void DFBImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
}

void DFBImage::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode)
{
}

void DFBImage::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
}

void DFBImage::StretchBlt(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
}

void DFBImage::StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
}

void DFBImage::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
	theImage->mDrawn = true;
 	if (!LockSurface())
		return;
	UnlockSurface();

	DeleteAllNonSurfaceData();

}

void DFBImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
	theTexture->mDrawn = true;

 	if (!LockSurface())
		return;

	UnlockSurface();
	DeleteAllNonSurfaceData();
}

bool DFBImage::Palletize()
{
	if (MemoryImage::Palletize())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DFBImage::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const BYTE* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight)
{
	if (theSpanCount == 0) return;

	if (!LockSurface())
		return;

	UnlockSurface();
	DeleteAllNonSurfaceData();
}
