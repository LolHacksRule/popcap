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

typedef struct {
  GLfloat  tu;
  GLfloat  tv;
  SexyRGBA color;
  GLfloat  sx;
  GLfloat  sy;
  GLfloat  sz;
} SexyGLVertex;

namespace Sexy {
struct GLTextureBlock
{
	GLuint    mTexture;
	GLshort   mWidth;
        GLshort   mHeight;
};

class GLTexture
{
public:
        typedef std::vector<GLTextureBlock> TextureVector;

        TextureVector             mTextures;
        int                       mWidth;
        int                       mHeight;
        int                       mTexVecWidth;
        int                       mTexVecHeight;
        int                       mTexBlockWidth;
        int                       mTexBlockHeight;
        int                       mBitsChangedCount;
        int                       mTexMemSize;
        float                     mMaxTotalU;
        float                     mMaxTotalV;

public:
        GLTexture();
        ~GLTexture();

        void                      ReleaseTextures ();
        void                      CreateTextureDimensions (GLImage *theImage);
        void                      CreateTextures (GLImage *theImage);
        void                      CheckCreateTextures (GLImage *theImage);

        GLuint                    GetTexture (int x, int y, int &width, int &height, float &u1, float &v1,
                                              float &u2, float &v2);
        GLuint                    GetTextureF (float x, float y, float &width, float &height,
                                               float &u1, float &v1, float &u2, float &v2);

        void                      Blt (float theX, float theY, const Rect& theSrcRect, const Color& theColor);
        void                      BltTransformed (const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor,
						  const Rect *theClipRect = NULL, float theX = 0, float theY = 0, bool center = false);
        void                      BltTriangles (const TriVertex theVertices[][3], int theNumTriangles, uint32 theColor,
                                                float tx = 0, float ty = 0);
};

}

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

static GLuint CreateTexture(GLImage* theImage, int x, int y, int width, int height)
{
	GLuint texture;
	int w, h;

	/* Use the texture width and height expanded to powers of 2 */
	w = RoundToPOT (width);
	h = RoundToPOT (height);

	/* Create an OpenGL texture for the image */
	glGenTextures (1, &texture);
	glBindTexture (GL_TEXTURE_2D, texture);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1);

	uint32* bits = theImage->GetBits ();
	uint32* copy = new uint32[w * h];
	if (copy) {
		int i;

		int aWidth = std::min (w, (theImage->GetWidth() - x));
		int aHeight = std::min (h, (theImage->GetHeight() - y));
		int aWidthExtra = w > aWidth ? w - aWidth : 0;
		int aHeightExtra = h > aHeight ? h - aHeight : 0;

		int imageWidth = theImage->GetWidth();
		uint32 * dst = copy;
		uint32 * src = bits + y * imageWidth + x;
		for (i = 0; i < aHeight; i++) {
			memcpy (dst, src, aWidth * 4);
			memset (dst + aWidth, 0, aWidthExtra * 4);
			dst += w;
			src += imageWidth;
		}
		memset (copy + w * aHeight, 0, w * aHeightExtra);

		glTexImage2D (GL_TEXTURE_2D,
			      0,
			      GL_RGBA,
			      w, h,
			      0,
			      GL_BGRA,
			      GL_UNSIGNED_BYTE,
			      copy);
		delete [] copy;
	}

	return texture;
}

GLTexture::GLTexture ()
{
	mWidth = 0;
	mHeight = 0;
	mTexVecWidth = 0;
	mTexVecHeight = 0;
	mBitsChangedCount = 0;
	mTexMemSize = 0;
	mTexBlockWidth = 64;
	mTexBlockHeight = 64;
}

GLTexture::~GLTexture ()
{
	ReleaseTextures ();
}

void GLTexture::ReleaseTextures ()
{
	for(int i = 0; i < (int)mTextures.size(); i++)
		glDeleteTextures (1, &mTextures[i].mTexture);

	mTextures.clear();
	mTexMemSize = 0;
}

void GLTexture::CreateTextureDimensions (GLImage* theImage)
{
	GLInterface* interface = theImage->mInterface;
	int aWidth = theImage->GetWidth ();
	int aHeight = theImage->GetHeight ();
	int i;

	// Calculate inner block sizes
	mTexBlockWidth = aWidth;
	mTexBlockHeight = aHeight;
	bool usePOT = true;

	interface->CalulateBestTexDimensions (mTexBlockWidth, mTexBlockHeight, false, usePOT);

	// Calculate right boundary block sizes
	int aRightWidth = aWidth % mTexBlockWidth;
	int aRightHeight = mTexBlockHeight;
	if (aRightWidth > 0)
		interface->CalulateBestTexDimensions (aRightWidth, aRightHeight, true, usePOT);
	else
		aRightWidth = mTexBlockWidth;

	// Calculate bottom boundary block sizes
	int aBottomWidth = mTexBlockWidth;
	int aBottomHeight = aHeight % mTexBlockHeight;
	if (aBottomHeight > 0)
		interface->CalulateBestTexDimensions (aBottomWidth, aBottomHeight, true, usePOT);
	else
		aBottomHeight = mTexBlockHeight;

	// Calculate corner block size
	int aCornerWidth = aRightWidth;
	int aCornerHeight = aBottomHeight;
	interface->CalulateBestTexDimensions (aCornerWidth, aCornerHeight, true, usePOT);

	// Allocate texture array
	mTexVecWidth = (aWidth + mTexBlockWidth - 1) / mTexBlockWidth;
	mTexVecHeight = (aHeight + mTexBlockHeight - 1) / mTexBlockHeight;
	mTextures.resize (mTexVecWidth * mTexVecHeight);

	// Assign inner blocks
	for(i = 0; i < (int)mTextures.size(); i++)
	{
		GLTextureBlock &aBlock = mTextures[i];
		aBlock.mTexture = 0;
		aBlock.mWidth = mTexBlockWidth;
		aBlock.mHeight = mTexBlockHeight;
	}

	// Assign right blocks
	for(i = mTexVecWidth - 1; i < (int)mTextures.size(); i += mTexVecWidth)
	{
		GLTextureBlock &aBlock = mTextures[i];
		aBlock.mWidth = aRightWidth;
		aBlock.mHeight = aRightHeight;
	}

	// Assign bottom blocks
	for (i = mTexVecWidth * (mTexVecHeight - 1); i < (int)mTextures.size(); i++)
	{
		GLTextureBlock &aBlock = mTextures[i];
		aBlock.mWidth = aBottomWidth;
		aBlock.mHeight = aBottomHeight;
	}

	// Assign corner block
	mTextures.back ().mWidth = aCornerWidth;
	mTextures.back ().mHeight = aCornerHeight;

	mMaxTotalU = aWidth / (float)mTexBlockWidth;
	mMaxTotalV = aHeight / (float)mTexBlockHeight;
}

GLuint GLTexture::GetTexture (int x, int y, int &width, int &height,
			      float &u1, float &v1, float &u2, float &v2)
{
	int tx = x / mTexBlockWidth;
	int ty = y / mTexBlockHeight;

	GLTextureBlock &aBlock = mTextures[ty * mTexVecWidth + tx];

	int left = x % mTexBlockWidth;
	int top = y % mTexBlockHeight;
	int right = left + width;
	int bottom = top + height;

	if (right > aBlock.mWidth)
		right = aBlock.mWidth;

	if (bottom > aBlock.mHeight)
		bottom = aBlock.mHeight;

	width = right-left;
	height = bottom-top;

	u1 = left / (float)aBlock.mWidth;
	v1 = top / (float)aBlock.mHeight;
	u2 = right / (float)aBlock.mWidth;
	v2 = bottom / (float)aBlock.mHeight;

	return aBlock.mTexture;
}

GLuint GLTexture::GetTextureF (float x, float y, float &width, float &height,
			       float &u1, float &v1, float &u2, float &v2)
{
	int tx = (int) (x / mTexBlockWidth);
	int ty = (int) (y / mTexBlockHeight);

	GLTextureBlock &aBlock = mTextures[ty * mTexVecWidth + tx];

	float left = x - tx * mTexBlockWidth;
	float top = y - ty * mTexBlockHeight;
	float right = left + width;
	float bottom = top + height;

	if (right > aBlock.mWidth)
		right = aBlock.mWidth;

	if (bottom > aBlock.mHeight)
		bottom = aBlock.mHeight;

	width = right-left;
	height = bottom-top;

	u1 = left / aBlock.mWidth;
	v1 = top / aBlock.mHeight;
	u2 = right / aBlock.mWidth;
	v2 = bottom / aBlock.mHeight;

	return aBlock.mTexture;
}

void GLTexture::CreateTextures(GLImage* theImage)
{
	theImage->CommitBits();

	// Release texture if image size has changed
	bool createTextures = false;
	if (mWidth != theImage->mWidth || mHeight!=theImage->mHeight)
	{
		ReleaseTextures ();
		CreateTextureDimensions (theImage);
		createTextures = true;
	}

	int i, x, y;

	int aHeight = theImage->GetHeight ();
	int aWidth = theImage->GetWidth ();
	int aFormatSize = 4;

	i = 0;
	for (y = 0; y < aHeight; y += mTexBlockHeight)
	{
		for (x = 0; x < aWidth; x += mTexBlockWidth, i++)
		{
			GLTextureBlock &aBlock = mTextures[i];
			if (createTextures)
			{
				aBlock.mTexture = CreateTexture (theImage, x, y,
								 aBlock.mWidth, aBlock.mHeight);
				if (aBlock.mTexture == 0) // create texture failure
					return;

				mTexMemSize += aBlock.mWidth * aBlock.mHeight * aFormatSize;
			}
		}
	}

	mWidth = theImage->mWidth;
	mHeight = theImage->mHeight;
	mBitsChangedCount = theImage->mBitsChangedCount;
}

void GLTexture::CheckCreateTextures (GLImage *theImage)
{
	if (theImage->mWidth != mWidth || theImage->mHeight != mHeight ||
	    theImage->mBitsChangedCount != mBitsChangedCount)
		CreateTextures (theImage);
}

void GLTexture::Blt (float theX, float theY, const Rect& theSrcRect,
		     const Color& theColor)
{

	int srcLeft = theSrcRect.mX;
	int srcTop = theSrcRect.mY;
	int srcRight = srcLeft + theSrcRect.mWidth;
	int srcBottom = srcTop + theSrcRect.mHeight;
	int srcX, srcY;
	float dstX, dstY;
	int aWidth, aHeight;
	float u1, v1, u2, v2;

	srcY = srcTop;
	dstY = theY;

        SexyRGBA rgba = theColor.ToRGBA ();
        glColor4ub (rgba.r, rgba.g, rgba.b, rgba.a);
	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

	glEnable (GL_TEXTURE_2D);
	while (srcY < srcBottom)
	{
		srcX = srcLeft;
		dstX = theX;
		while (srcX < srcRight)
		{
			aWidth = srcRight - srcX;
			aHeight = srcBottom - srcY;

			GLuint aTexture = GetTexture (srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

                        float x = dstX;// - 0.5f;
                        float y = dstY;// 0.5f;

                        glBindTexture (GL_TEXTURE_2D, aTexture);
                        glBegin (GL_TRIANGLE_STRIP);
                        glTexCoord2f (u1, v1);
                        glVertex2f (x, y);
                        glTexCoord2f (u1, v2);
                        glVertex2f (x, y + aHeight);
                        glTexCoord2f (u2, v1);
                        glVertex2f (x + aWidth, y);
                        glTexCoord2f (u2, v2);
                        glVertex2f (x + aWidth, y + aHeight);
                        glEnd ();

			srcX += aWidth;
			dstX += aWidth;

		}

		srcY += aHeight;
		dstY += aHeight;
	}
}

GLImage::GLImage(GLInterface* theInterface) :
	MemoryImage(theInterface->mApp),
	mInterface(theInterface),
	mDirty(0)
{
	mTexture = 0;
	Init();
}

GLImage::GLImage() :
	MemoryImage(gSexyAppBase)
{
	mInterface = dynamic_cast<GLInterface *>(gSexyAppBase->mDDInterface);
	mTexture = 0;
	Init();
}

GLImage::~GLImage()
{
	DBG_ASSERTE(mLockCount == 0);
	delete mTexture;
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
	if (mInterface->GetScreenImage () != this)
	{
		MemoryImage::FillRect (theRect, theColor, theDrawMode);
		return;
	}

	glDisable (GL_TEXTURE_2D);

	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);

	SexyRGBA aColor = theColor.ToRGBA();
	float x = theRect.mX;// - 0.5f;
	float y = theRect.mY;// - 0.5f;
	float aWidth = theRect.mWidth;
	float aHeight = theRect.mHeight;

	SexyGLVertex aVertex[4] =
	{
		{ 0, 0, aColor, x,	      y,           0},
		{ 0, 0, aColor, x,	      y + aHeight, 0},
		{ 0, 0, aColor, x + aWidth, y,           0},
		{ 0, 0, aColor, x + aWidth, y + aHeight, 0}
	};

        glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);
        glBegin (GL_TRIANGLE_STRIP);
        glVertex2f (aVertex[0].sx, aVertex[0].sy);
        glVertex2f (aVertex[1].sx, aVertex[1].sy);
        glVertex2f (aVertex[2].sx, aVertex[2].sy);
        glVertex2f (aVertex[3].sx, aVertex[3].sy);
        glEnd ();
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
	if (mInterface->GetScreenImage () != this)
	{
		MemoryImage::DrawLine (theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
		return;
	}

        glDisable (GL_TEXTURE_2D);

	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);

	float x1, y1, x2, y2;
	SexyRGBA aColor = theColor.ToRGBA ();

	x1 = theStartX;
	y1 = theStartY;
	x2 = theEndX;
	y2 = theEndY;

        glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);

        glBegin (GL_LINE_STRIP);

        glVertex2f (x1, y1);
        glVertex2f (x2, y2);

        glEnd ();
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
	if (mInterface->GetScreenImage () != this)
	{
		MemoryImage::DrawLineAA (theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
		return;
	}

        glDisable (GL_TEXTURE_2D);

	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);

	float x1, y1, x2, y2;
	SexyRGBA aColor = theColor.ToRGBA ();

	x1 = theStartX;
	y1 = theStartY;
	x2 = theEndX;
	y2 = theEndY;

        glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);

        glBegin (GL_LINE_STRIP);

        glVertex2f (x1, y1);
        glVertex2f (x2, y2);

        glEnd ();
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

	BitsChanged ();
}

void GLImage::BitsChanged()
{
	MemoryImage::BitsChanged();
}

uint32* GLImage::GetBits()
{
	uint32* bits = MemoryImage::GetBits ();

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
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::Blt(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
		return;
	}

	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;

	srcImage->EnsureTexture();
	if (!srcImage->mTexture)
		return;

	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);
	srcImage->mTexture->Blt (theX, theY, theSrcRect, theColor);
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

	srcImage->EnsureTexture();
	if (!srcImage->mTexture)
		return;

	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);
	srcImage->mTexture->Blt (theX, theY, theSrcRect, theColor);
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
	if (theBits && !mBits)
		MemoryImage::GetBits();
	MemoryImage::SetBits(theBits, theWidth, theHeight, commitBits);

	if (mTexture)
		mTexture->CheckCreateTextures (this);
}

void GLImage::Flip(enum FlipFlags flags)
{
	if (mInterface->mScreenImage)
		mInterface->SwapBuffers();
}

void GLImage::EnsureTexture()
{
	if (!mTexture)
		mTexture = new GLTexture();

	mTexture->CheckCreateTextures (this);
}
