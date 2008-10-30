#include <math.h>

#include "GLImage.h"
#include "GLInterface.h"
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

#define SURFACE_DIRTY (1 << 0)
#define BITS_DIRTY    (1 << 1)

namespace Sexy {
class GLImageAutoFallback
{
public:
	GLImageAutoFallback(GLImage * dst, GLImage * src) :
		mSrc(src), mDst(dst)
        {
		mDst->GetBits();
		if (mSrc)
			mSrc->GetBits();
	};

	~GLImageAutoFallback()
        {
	}
private:
	GLImage * mSrc;
	GLImage * mDst;
};
}

GLImage::GLImage(GLInterface* theInterface) :
	MemoryImage(theInterface->mApp),
	mInterface(theInterface),
	mDirty(0)
{
	Init();
}

GLImage::GLImage() :
	MemoryImage(gSexyAppBase)
{
	mInterface = dynamic_cast<GLInterface *>(gSexyAppBase->mDDInterface);
	Init();
}

GLImage::~GLImage()
{
	DBG_ASSERTE(mLockCount == 0);
}

void GLImage::Init()
{
	mNoLock = false;
	mVideoMemory = false;
	mFirstPixelTrans = false;
	mWantDDSurface = false;
	mDrawToBits = false;
	mSurfaceSet = false;
	mDirty = 0;

	mLockCount = 0;
}

bool GLImage::LockSurface()
{
	if (mLockCount == 0)
	{
		//lock
	}

	mLockCount++;

	DBG_ASSERTE(mLockCount < 8);

	return true;
}

bool GLImage::UnlockSurface()
{
	--mLockCount;

	if (mLockCount == 0)
	{
		//unlock
	}

	DBG_ASSERTE(mLockCount >= 0);

	return true;
}

void GLImage::DeleteSurface()
{
}

void GLImage::ReInit()
{
	MemoryImage::ReInit();
}

void GLImage::PurgeBits()
{
	if (mSurfaceSet)
		return;

	mPurgeBits = true;

	CommitBits();
	MemoryImage::PurgeBits();
}

void GLImage::DeleteAllNonSurfaceData()
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

void GLImage::DeleteNativeData()
{
	if (mSurfaceSet)
		return;

	MemoryImage::DeleteNativeData();
	DeleteSurface();
}

void GLImage::DeleteExtraBuffers()
{
	if (mSurfaceSet)
		return;

	MemoryImage::DeleteExtraBuffers();
	DeleteSurface();
}

void GLImage::SetVideoMemory(bool wantVideoMemory)
{
	TRACE_THIS();
}

void GLImage::RehupFirstPixelTrans()
{
}

bool GLImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool convex)
{
	TRACE_THIS();
	return false;
}

void GLImage::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
}

void GLImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
}

void GLImage::NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
	TRACE_THIS();
}

void GLImage::CommitBits()
{
	MemoryImage::CommitBits();
}

void GLImage::Create(int theWidth, int theHeight)
{
	delete [] mBits;

	mWidth = theWidth;
	mHeight = theHeight;

	mBits = NULL;

	BitsChanged();
}

void GLImage::BitsChanged()
{
	MemoryImage::BitsChanged();
}

uint32* GLImage::GetBits()
{
	uint32* bits = MemoryImage::GetBits();

	if (!bits)
		return 0;
	return bits;
}

void GLImage::Clear()
{
}

void GLImage::NormalFillRect(const Rect& theRect, const Color& theColor)
{
}

void GLImage::AdditiveFillRect(const Rect& theRect, const Color& theColor)
{
}

void GLImage::NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::NormalBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::AdditiveBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRectOrig, const Color& theColor)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
        if (srcImage)
 		return;
}

void GLImage::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::StretchBlt(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRectOrig, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;
}

void GLImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theTexture);
	if (!srcImage)
		return;
}

bool GLImage::Palletize()
{
	return false;
}

void GLImage::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const BYTE* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight)
{
	if (theSpanCount == 0)
		return;
}

void GLImage::SetBits(uint32* theBits, int theWidth, int theHeight, bool commitBits)
{
}

void GLImage::Flip(enum FlipFlags flags)
{
}
