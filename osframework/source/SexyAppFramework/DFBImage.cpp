#include <math.h>

#include "DFBImage.h"
#include "DFBInterface.h"
#include "Rect.h"
#include "Graphics.h"
#include "SexyAppBase.h"
#include "Debug.h"
#include "PerfTimer.h"

using namespace Sexy;
#define HERE() printf ("%s:%d\n", __FUNCTION__, __LINE__)

DFBImage::DFBImage(DFBInterface* theInterface) :
	MemoryImage(theInterface->mApp),
	mSurface(0),
	mInterface(theInterface)
{
	Init();
}

DFBImage::DFBImage(IDirectFBSurface * theSurface,
		   DFBInterface* theInterface) :
	MemoryImage(theInterface->mApp),
	mSurface(theSurface),
	mInterface(theInterface)
{
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

	if (mSurface)
		mSurface->Release(mSurface);
	DBG_ASSERTE(mLockCount == 0);
}

void DFBImage::Init()
{
	mInterface->AddImage(this);

	if (mSurface) {
		int width, height;
		mSurface->GetSize(mSurface, &width, &height);
		mWidth = width;
		mHeight = height;
	}

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
	HERE();
	return false;
}

void DFBImage::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
	if (!mSurface)
		return;
	mSurface->SetDrawingFlags(mSurface, DSDRAW_BLEND);
	mSurface->SetColor(mSurface,
			   theColor.GetRed(), theColor.GetGreen(),
			   theColor.GetBlue(), theColor.GetAlpha());
	mSurface->FillRectangle(mSurface, theRect.mX, theRect.mY, theRect.mWidth, theRect.mHeight);
}

void DFBImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	HERE();
}

void DFBImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	HERE();
}

void DFBImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
	if (!mSurface)
		return;
	mSurface->SetDrawingFlags(mSurface, DSDRAW_BLEND);
	mSurface->SetColor(mSurface,
			   theColor.GetRed(), theColor.GetGreen(),
			   theColor.GetBlue(), theColor.GetAlpha());
	mSurface->DrawLine(mSurface, floor(theStartX), floor(theStartY),
			   ceil(theEndX), ceil(theEndY));
}

void DFBImage::NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	HERE();
}

void DFBImage::AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	HERE();
}

void DFBImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
	HERE();
}

void DFBImage::CommitBits()
{
	MemoryImage::CommitBits();
	if (!mSurface)
		return;
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
	if (!mSurface)
		return;
	mSurface->SetDrawingFlags(mSurface, DSDRAW_BLEND);
	mSurface->SetColor(mSurface,
			   theColor.GetRed(), theColor.GetGreen(),
			   theColor.GetBlue(), theColor.GetAlpha());
	mSurface->FillRectangle(mSurface, theRect.mX, theRect.mY, theRect.mWidth, theRect.mHeight);
}

void DFBImage::AdditiveFillRect(const Rect& theRect, const Color& theColor)
{
	HERE();
}

void DFBImage::NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
	DFBImage * srcImage = dynamic_cast<DFBImage*>(theImage);
	IDirectFBSurface * src, * dst;

	src = srcImage->EnsureSurface();
	dst = EnsureSurface();
	if (src == NULL || dst == NULL)
		return;

	DFBRectangle src_rect;
	src_rect.x = theSrcRect.mX;
	src_rect.y = theSrcRect.mY;
	src_rect.w = theSrcRect.mWidth;
	src_rect.h = theSrcRect.mHeight;

	DFBSurfaceBlittingFlags flags;
	flags = DSBLIT_BLEND_ALPHACHANNEL;
	if (theColor != Color::White) {
		if (theColor.GetRed() == 0xff &&
		    theColor.GetGreen() == 0xff &&
		    theColor.GetBlue() == 0xff)
			flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_BLEND_COLORALPHA);
		else
			flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_COLORIZE);
	} else {
		flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_COLORIZE);
	}
	dst->SetBlittingFlags(src, flags);
	dst->Blit(dst, src, &src_rect, theX, theY);
}

void DFBImage::NormalBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
	HERE();
}

void DFBImage::AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
	DFBImage * srcImage = dynamic_cast<DFBImage*>(theImage);
	IDirectFBSurface * src, * dst;

	src = srcImage->EnsureSurface();
	dst = EnsureSurface();
	if (src == NULL || dst == NULL)
		return;

	DFBRectangle src_rect;
	src_rect.x = theSrcRect.mX;
	src_rect.y = theSrcRect.mY;
	src_rect.w = theSrcRect.mWidth;
	src_rect.h = theSrcRect.mHeight;

	dst->SetBlittingFlags(src, DSBLIT_BLEND_ALPHACHANNEL);
	dst->Blit(dst, src, &src_rect, theX, theY);
}

void DFBImage::AdditiveBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
	HERE();
}

void DFBImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	DBG_ASSERTE((theColor.mRed >= 0) && (theColor.mRed <= 255));
	DBG_ASSERTE((theColor.mGreen >= 0) && (theColor.mGreen <= 255));
	DBG_ASSERTE((theColor.mBlue >= 0) && (theColor.mBlue <= 255));
	DBG_ASSERTE((theColor.mAlpha >= 0) && (theColor.mAlpha <= 255));

	switch (theDrawMode)
	{
	case Graphics::DRAWMODE_NORMAL:
		NormalBlt(theImage, theX, theY, theSrcRect, theColor);
		break;
	case Graphics::DRAWMODE_ADDITIVE:
		AdditiveBlt(theImage, theX, theY, theSrcRect, theColor);
		break;
	}
}

void DFBImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
	HERE();
}

void DFBImage::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode)
{
	HERE();
}

void DFBImage::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
	HERE();
}

void DFBImage::StretchBlt(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	HERE();
}

void DFBImage::StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	HERE();
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

void DFBImage::SetBits(uint32* theBits, int theWidth, int theHeight, bool commitBits)
{
	if (!mSurface) {
		GetBits();
		MemoryImage::SetBits(theBits, theWidth, theHeight, commitBits);
	} else {
		DFBResult ret;
		unsigned char * addr;
		int i, pitch;
		unsigned char * dst, * src;

		ret = mSurface->Lock(mSurface, DSLF_WRITE, (void **)&addr, &pitch);
		if (ret != DFB_OK)
			return;
		src = (unsigned char*)theBits;
		dst = (unsigned char*)addr;
		for (i = 0; i < mHeight; i++) {
			memcpy (dst, src, theWidth * 4);
			src += theWidth * 4;
			dst += pitch;
		}
		mSurface->Unlock(mSurface);
	}
}

IDirectFBSurface* DFBImage::EnsureSurface()
{
	if (mSurface)
		return mSurface;

	if (!mWidth || !mHeight)
		return 0;

	mSurface = mInterface->CreateDFBSurface(mWidth, mHeight);

	DFBResult ret;
        unsigned char * addr;
	int i, pitch;
	unsigned char * dst, * src;

	ret = mSurface->Lock(mSurface, DSLF_WRITE, (void **)&addr, &pitch);
	if (ret != DFB_OK)
		return mSurface;
	src = (unsigned char*)mBits;
	dst = (unsigned char*)addr;
	for (i = 0; i < mHeight; i++) {
		memcpy (dst, src, mWidth * 4);
		src += mWidth * 4;
		dst += pitch;
	}
	mSurface->Unlock(mSurface);
	HERE();
	return mSurface;
}
