#include <math.h>

#include "DFBImage.h"
#include "DFBInterface.h"
#include "Rect.h"
#include "Graphics.h"
#include "SexyAppBase.h"
#include "Debug.h"
#include "PerfTimer.h"

using namespace Sexy;

#if 0
#define TRACE() printf ("%s:%d\n", __FUNCTION__, __LINE__);
#define TRACE_THIS() printf ("%s:%d this = %p\n", __FUNCTION__, __LINE__, this);
#else
#define TRACE()
#define TRACE_THIS()
#endif

DFBImage::DFBImage(DFBInterface* theInterface) :
	MemoryImage(theInterface->mApp),
	mInterface(theInterface),
	mSurface(0)
{
	Init();
}

DFBImage::DFBImage(IDirectFBSurface * theSurface,
		   DFBInterface* theInterface) :
	MemoryImage(theInterface->mApp),
	mInterface(theInterface),
	mSurface(theSurface)
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
	DeleteSurface();
	DBG_ASSERTE(mLockCount == 0);
}

void DFBImage::Init()
{
	if (mSurface) {
		int width, height;
		mSurface->GetSize(mSurface, &width, &height);
		mWidth = width;
		mHeight = height;
		mNumRows = width;
		mNumCols = height;
		mSurface->GetCapabilities(mSurface, &mCaps);
	}

	mNoLock = false;
	mVideoMemory = false;
	mFirstPixelTrans = false;
	mWantDDSurface = false;
	mDrawToBits = false;
	mSurfaceSet = false;
	mMemoryCount = 0;
	mDFBCount = 1;
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
        if (mSurface)
		mInterface->DelayedReleaseSurface(mSurface);
	mSurface = 0;
	mDFBCount++;
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

	mDFBCount++;
}

void DFBImage::DeleteNativeData()
{
	GetBits();
	MemoryImage::DeleteNativeData();
	DeleteSurface();
}

void DFBImage::DeleteExtraBuffers()
{
	mDFBCount++;
	GetBits();
	MemoryImage::DeleteExtraBuffers();
	DeleteSurface();
}

void DFBImage::SetVideoMemory(bool wantVideoMemory)
{
}

void DFBImage::RehupFirstPixelTrans()
{
}

bool DFBImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect,
			  const Color &theColor, int theDrawMode, int tx, int ty, bool convex)
{
	TRACE_THIS();
	return false;
}

void DFBImage::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
	IDirectFBSurface * dst;

	dst = EnsureSurface();
	if (!dst)
	{
		MemoryImage::FillRect(theRect, theColor, theDrawMode);
		return;
	}

	TRACE_THIS();
	dst->SetDrawingFlags(dst, DSDRAW_BLEND);
	dst->SetColor(dst,
		      theColor.GetRed(), theColor.GetGreen(),
		      theColor.GetBlue(), theColor.GetAlpha());
	dst->FillRectangle(dst, theRect.mX, theRect.mY, theRect.mWidth, theRect.mHeight);
	mDFBCount++;
}

void DFBImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void DFBImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void DFBImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY,
			const Color& theColor, int theDrawMode)
{
	IDirectFBSurface * dst;

	dst = EnsureSurface();
	if (!dst)
	{
		MemoryImage::DrawLine(theStartX, theStartY, theEndX, theEndY,
				      theColor, theDrawMode);
		return;
	}

	dst->SetDrawingFlags(dst, DSDRAW_BLEND);
	dst->SetColor(dst,
		      theColor.GetRed(), theColor.GetGreen(),
		      theColor.GetBlue(), theColor.GetAlpha());
	dst->DrawLine(dst, floor(theStartX), floor(theStartY),
		      ceil(theEndX), ceil(theEndY));
	mDFBCount++;
}

void DFBImage::NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY,
				const Color& theColor)
{
	TRACE_THIS();
}

void DFBImage::AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY,
				  const Color& theColor)
{
	TRACE_THIS();
}

void DFBImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY,
			  const Color& theColor, int theDrawMode)
{
	TRACE_THIS();
}

void DFBImage::CommitBits()
{
	MemoryImage::CommitBits();
}

void DFBImage::Create(int theWidth, int theHeight)
{
	MemoryImage::Create(theWidth, theHeight);
}

void DFBImage::BitsChanged()
{
	MemoryImage::BitsChanged();
	if (mSurface && mInterface->IsMainThread()) {
		DFBResult ret;
		int i, pitch;
		unsigned char * dst, * src;

		if (!mBits)
			MemoryImage::GetBits();
		src = (unsigned char*)mBits;
		if (!src)
			return;
		ret = mSurface->Lock(mSurface, DSLF_WRITE, (void **)&dst, &pitch);
		if (ret != DFB_OK)
			return;
		for (i = 0; i < mHeight; i++) {
			memcpy (dst, src, mWidth * 4);
			src += mWidth * 4;
			dst += pitch;
		}
		mSurface->Unlock(mSurface);
		mDFBCount = 0;
		mMemoryCount = mBitsChangedCount;
		TRACE_THIS();
	}
}

uint32* DFBImage::GetBits()
{
	uint32* bits = MemoryImage::GetBits();

	if (!bits)
		return 0;
	TRACE_THIS();
	if (mSurface && mDFBCount && mInterface->IsMainThread()) {
		DFBResult ret;
		int i, pitch;
		unsigned char * dst, * src;

		ret = mSurface->Lock(mSurface, (DFBSurfaceLockFlags)(DSLF_READ | DSLF_WRITE),
				     (void **)&src, &pitch);
		if (ret != DFB_OK)
			return bits;
		dst = (unsigned char*)bits;
		for (i = 0; i < mHeight; i++) {
			memcpy (dst, src, mWidth * 4);
			dst += mWidth * 4;
			src += pitch;
		}
		mSurface->Unlock(mSurface);
		mDFBCount = 0;
		TRACE_THIS();
	}
	return bits;
}

void DFBImage::ClearRect(const Rect& theRect)
{
	IDirectFBSurface * dst;

	TRACE_THIS();

	dst = EnsureSurface();
	if (!dst)
	{
		MemoryImage::ClearRect(theRect);
		return;
	}

	DFBRegion   clip_rect;
	clip_rect.x1 = theRect.mX;
	clip_rect.y1 = theRect.mY;
	clip_rect.x2 = theRect.mX + theRect.mWidth;
	clip_rect.y2 = theRect.mY + theRect.mHeight;

	dst->SetClip (dst, &clip_rect);
	dst->Clear (dst, 0, 0, 0, 0);
	dst->SetClip (dst, NULL);
	mDFBCount++;
}

void DFBImage::Clear()
{
	IDirectFBSurface * dst;

	TRACE_THIS();

	dst = EnsureSurface();
	if (!dst)
	{
		MemoryImage::Clear();
		return;
	}

	dst->Clear (dst, 0, 0, 0, 0);
	mDFBCount++;
}

void DFBImage::NormalFillRect(const Rect& theRect, const Color& theColor)
{
	IDirectFBSurface * dst;

	dst = EnsureSurface();
	if (!dst)
		return;

	dst->SetDrawingFlags(dst, DSDRAW_BLEND);
	dst->SetColor(dst,
		      theColor.GetRed(), theColor.GetGreen(),
		      theColor.GetBlue(), theColor.GetAlpha());
	dst->FillRectangle(dst, theRect.mX, theRect.mY, theRect.mWidth, theRect.mHeight);
	mDFBCount++;
}

void DFBImage::AdditiveFillRect(const Rect& theRect, const Color& theColor)
{
	TRACE_THIS();
}

void DFBImage::NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
	DFBImage * srcImage = dynamic_cast<DFBImage*>(theImage);
	IDirectFBSurface * src, * dst;

	src = srcImage->EnsureSurface();
	dst = EnsureSurface();
	if (src == NULL || dst == NULL) {
		MemoryImage::NormalBlt(theImage, theX, theY, theSrcRect, theColor);
		return;
	}


	DFBRectangle src_rect;
	src_rect.x = theSrcRect.mX;
	src_rect.y = theSrcRect.mY;
	src_rect.w = theSrcRect.mWidth;
	src_rect.h = theSrcRect.mHeight;

	DFBSurfaceBlittingFlags flags;
	flags = DSBLIT_BLEND_ALPHACHANNEL;
	if (theColor.GetRed() == 0xff &&
	    theColor.GetGreen() == 0xff &&
	    theColor.GetBlue() == 0xff) {
		if (theColor.GetAlpha() != 0xff)
			flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_BLEND_COLORALPHA);
	} else {
		flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_COLORIZE | DSBLIT_BLEND_COLORALPHA);
	}
	dst->SetBlittingFlags(dst, flags);
	dst->SetColor(dst,
		      theColor.GetRed(), theColor.GetGreen(),
		      theColor.GetBlue(), theColor.GetAlpha());
	dst->Blit(dst, src, &src_rect, theX, theY);
	mDFBCount++;
	TRACE_THIS();
}

void DFBImage::NormalBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig,
			       const Color& theColor)
{
	TRACE_THIS();
}

void DFBImage::AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
	DFBImage * srcImage = dynamic_cast<DFBImage*>(theImage);
	if (!srcImage)
		return;

	IDirectFBSurface * src, * dst;

	src = srcImage->EnsureSurface();
	dst = EnsureSurface();
	if (src == NULL || dst == NULL) {
		MemoryImage::AdditiveBlt(theImage, theX, theY, theSrcRect, theColor);
		return;
	}

	DFBRectangle src_rect;
	src_rect.x = theSrcRect.mX;
	src_rect.y = theSrcRect.mY;
	src_rect.w = theSrcRect.mWidth;
	src_rect.h = theSrcRect.mHeight;

	DFBSurfaceBlittingFlags flags;
	flags = DSBLIT_BLEND_ALPHACHANNEL;
	if (theColor.GetRed() == 0xff &&
	    theColor.GetGreen() == 0xff &&
	    theColor.GetBlue() == 0xff) {
		if (theColor.GetAlpha() != 0xff)
			flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_BLEND_COLORALPHA);
	} else {
		flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_COLORIZE | DSBLIT_BLEND_COLORALPHA);
	}
	dst->SetBlittingFlags(dst, flags);
	dst->SetDstBlendFunction (dst, DSBF_ONE);
	dst->SetColor(dst,
		      theColor.GetRed(), theColor.GetGreen(),
		      theColor.GetBlue(), theColor.GetAlpha());
	dst->Blit(dst, src, &src_rect, theX, theY);
	dst->SetPorterDuff(dst, DSPD_NONE);
	mDFBCount++;
	TRACE_THIS();
}

void DFBImage::AdditiveBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig,
				 const Color& theColor)
{
	TRACE_THIS();
}

void DFBImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	DBG_ASSERTE((theColor.mRed >= 0) && (theColor.mRed <= 255));
	DBG_ASSERTE((theColor.mGreen >= 0) && (theColor.mGreen <= 255));
	DBG_ASSERTE((theColor.mBlue >= 0) && (theColor.mBlue <= 255));
	DBG_ASSERTE((theColor.mAlpha >= 0) && (theColor.mAlpha <= 255));

	TRACE_THIS();

	switch (theDrawMode)
	{
	case Graphics::DRAWMODE_NORMAL:
		NormalBlt(theImage, theX, theY, theSrcRect, theColor);
		break;
	case Graphics::DRAWMODE_ADDITIVE:
		AdditiveBlt(theImage, theX, theY, theSrcRect, theColor);
		break;
	}
	TRACE_THIS();
}

void DFBImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
	MemoryImage::BltMirror(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
}

void DFBImage::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect,
		    const Rect &theClipRect, const Color& theColor, int theDrawMode)
{
	if (true)
		MemoryImage::BltF(theImage, theX, theY, theSrcRect, theClipRect,
				  theColor, theDrawMode);

	DFBImage * srcImage = dynamic_cast<DFBImage*>(theImage);
	IDirectFBSurface * src, * dst;

	src = srcImage->EnsureSurface();
	dst = EnsureSurface();
	if (src == NULL || dst == NULL) {
		MemoryImage::BltF(theImage, theX, theY, theSrcRect, theClipRect,
				  theColor, theDrawMode);
		return;
	}


	DFBRectangle src_rect;
	src_rect.x = theSrcRect.mX;
	src_rect.y = theSrcRect.mY;
	src_rect.w = theSrcRect.mWidth;
	src_rect.h = theSrcRect.mHeight;

	DFBRegion clip_reg;
	clip_reg.x1 = theClipRect.mX;
	clip_reg.y1 = theClipRect.mY;
	clip_reg.x2 = clip_reg.x1 + theClipRect.mWidth;
	clip_reg.y2 = clip_reg.y1 + theClipRect.mHeight;

	DFBSurfaceBlittingFlags flags;
	flags = DSBLIT_BLEND_ALPHACHANNEL;
	if (theColor.GetRed() == 0xff &&
	    theColor.GetGreen() == 0xff &&
	    theColor.GetBlue() == 0xff) {
		if (theColor.GetAlpha() != 0xff)
			flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_BLEND_COLORALPHA);
	} else {
		flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_COLORIZE | DSBLIT_BLEND_COLORALPHA);
	}
	dst->SetBlittingFlags(dst, flags);
	if (theDrawMode == Graphics::DRAWMODE_ADDITIVE)
		dst->SetDstBlendFunction (dst, DSBF_ONE);
	dst->SetColor(dst,
		      theColor.GetRed(), theColor.GetGreen(),
		      theColor.GetBlue(), theColor.GetAlpha());
	dst->SetClip(dst, &clip_reg);
	dst->Blit(dst, src, &src_rect, theX, theY);
	dst->SetPorterDuff(dst, DSPD_NONE);
	dst->SetClip(dst, NULL);
	mDFBCount++;
}

void DFBImage::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect,
			  const Rect& theClipRect, const Color& theColor, int theDrawMode,
			  double theRot, float theRotCenterX, float theRotCenterY)
{
	MemoryImage::BltRotated(theImage, theX, theY, theSrcRect, theClipRect, theColor,
				theDrawMode, theRot, theRotCenterX, theRotCenterY);
}

void DFBImage::StretchBlt(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig,
			  const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	DFBImage * srcImage = dynamic_cast<DFBImage*>(theImage);
	IDirectFBSurface * src, * dst;

	if (!srcImage)
		return;

	src = srcImage->EnsureSurface();
	dst = EnsureSurface();
	if (src == NULL || dst == NULL) {
		MemoryImage::StretchBlt(theImage, theDestRectOrig, theSrcRectOrig,
					theClipRect, theColor, theDrawMode, fastStretch);
		return;
	}

	theImage->mDrawn = true;
	DFBRectangle src_rect, dest_rect;
	DFBRegion clip_reg;
	src_rect.x = theSrcRectOrig.mX;
	src_rect.y = theSrcRectOrig.mY;
	src_rect.w = theSrcRectOrig.mWidth;
	src_rect.h = theSrcRectOrig.mHeight;

	dest_rect.x = theDestRectOrig.mX;
	dest_rect.y = theDestRectOrig.mY;
	dest_rect.w = theDestRectOrig.mWidth;
	dest_rect.h = theDestRectOrig.mHeight;

	clip_reg.x1 = theClipRect.mX;
	clip_reg.y1 = theClipRect.mY;
	clip_reg.x2 = clip_reg.x1 + theClipRect.mWidth;
	clip_reg.y2 = clip_reg.y1 + theClipRect.mHeight;

	DFBSurfaceBlittingFlags flags;
	flags = DSBLIT_BLEND_ALPHACHANNEL;
	if (theColor.GetRed() == 0xff &&
	    theColor.GetGreen() == 0xff &&
	    theColor.GetBlue() == 0xff) {
		if (theColor.GetAlpha() != 0xff)
			flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_BLEND_COLORALPHA);
	} else {
		flags = (DFBSurfaceBlittingFlags)(flags | DSBLIT_COLORIZE | DSBLIT_BLEND_COLORALPHA);
	}
	dst->SetBlittingFlags(dst, flags);
	dst->SetClip(dst, &clip_reg);
	dst->SetColor(dst,
		      theColor.GetRed(), theColor.GetGreen(),
		      theColor.GetBlue(), theColor.GetAlpha());
	if (theDrawMode == Graphics::DRAWMODE_ADDITIVE)
		dst->SetDstBlendFunction(dst, DSBF_ONE);
	dst->StretchBlit(dst, src, &src_rect, &dest_rect);
	dst->SetPorterDuff(dst, DSPD_NONE);
	dst->SetClip(dst, NULL);
	mDFBCount++;
}

void DFBImage::StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig,
				const Rect& theClipRect, const Color& theColor, int theDrawMode,
				bool fastStretch)
{
	MemoryImage::StretchBltMirror(theImage, theDestRectOrig, theSrcRectOrig, theClipRect,
				      theColor, theDrawMode, fastStretch);
}

void DFBImage::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix,
			 const Rect& theClipRect, const Color& theColor, int theDrawMode,
			 const Rect &theSrcRect, bool blend)
{
	MemoryImage::BltMatrix(theImage, x, y, theMatrix, theClipRect, theColor, theDrawMode, theSrcRect, blend);
}

void DFBImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles,
			       const Rect& theClipRect, const Color &theColor, int theDrawMode,
			       float tx, float ty, bool blend)
{
	MemoryImage::BltTrianglesTex(theTexture, theVertices, theNumTriangles, theClipRect, theColor,
				     theDrawMode, tx, ty, blend);
}

bool DFBImage::Palletize()
{
	return false;
}

void DFBImage::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor,
					 int theDrawMode, const BYTE* theCoverage, int theCoverX,
					 int theCoverY, int theCoverWidth, int theCoverHeight)
{
	if (theSpanCount == 0)
		return;

	MemoryImage::FillScanLinesWithCoverage(theSpans, theSpanCount, theColor, theDrawMode, theCoverage,
					       theCoverX, theCoverY, theCoverWidth, theCoverHeight);
}

void DFBImage::SetBits(uint32* theBits, int theWidth, int theHeight, bool commitBits)
{
	TRACE_THIS();
	if (!mSurface) {
		MemoryImage::GetBits();
		MemoryImage::SetBits(theBits, theWidth, theHeight, commitBits);
	} else if (theBits) {
		DFBResult ret;
		unsigned char * addr;
		int i, pitch;
		unsigned char * dst, * src;

		if (mBits)
			MemoryImage::SetBits(theBits, theWidth, theHeight, commitBits);
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

	mDFBCount = 0;
	mMemoryCount = mBitsChangedCount;
}

IDirectFBSurface* DFBImage::EnsureSurface()
{
	if (!mInterface->IsMainThread())
		return 0;

	if (mSurface)
	{
		if (mMemoryCount != mBitsChangedCount && mBits)
			SetBits(mBits, mWidth, mHeight);
		return mSurface;
	}

	if (!mWidth || !mHeight)
		return 0;

	mSurface = mInterface->CreateDFBSurface(mWidth, mHeight);
	mSurface->GetCapabilities(mSurface, &mCaps);
	if (mBits)
		SetBits(mBits, mWidth, mHeight);
	DeleteAllNonSurfaceData();

	return mSurface;
}

void DFBImage::Flip(enum FlipFlags flags)
{
	if (!mSurface)
		return;

	if ((mCaps & DSCAPS_PRIMARY) || mSurface == mInterface->mPrimarySurface)
		flags = (FlipFlags)(flags | FLIP_BLIT);
	DFBSurfaceFlipFlags  dfbflags = DSFLIP_NONE;
	if (flags & FLIP_BLIT)
		dfbflags = (DFBSurfaceFlipFlags)(dfbflags | DSFLIP_BLIT);
	if (flags & FLIP_WAIT)
		dfbflags = (DFBSurfaceFlipFlags)(dfbflags | DSFLIP_WAIT);
	if (flags & FLIP_ONSYNC)
		dfbflags = (DFBSurfaceFlipFlags)(dfbflags | DSFLIP_ONSYNC);
	mSurface->Flip (mSurface, NULL, dfbflags);
}
