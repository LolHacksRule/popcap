#include <math.h>

#include "GLImage.h"
#include "GLInterface.h"
#include "Rect.h"
#include "Graphics.h"
#include "SexyAppBase.h"
#include "SexyVector.h"
#include "SexyMatrix.h"
#include "Debug.h"
#include "PerfTimer.h"

using namespace Sexy;

#if 1
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

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_BGRA
#define GL_BGRA  0x80E1
#endif

static GLuint CreateTexture (GLImage* theImage, int x, int y, int width, int height)
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

struct VertexList
{
	enum { MAX_STACK_VERTS = 100 };
	SexyGLVertex mStackVerts[MAX_STACK_VERTS];
	SexyGLVertex *mVerts;
	int mSize;
	int mCapacity;

	typedef int size_type;

	VertexList() : mVerts(mStackVerts), mSize(0),
		       mCapacity (MAX_STACK_VERTS)
	{
	}

	VertexList(const VertexList &theList) : mVerts(mStackVerts),
						mSize(theList.mSize),
						mCapacity(MAX_STACK_VERTS)
	{
		reserve (mSize);
		memcpy (mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
	}

	~VertexList()
	{
		if (mVerts != mStackVerts)
			delete mVerts;
	}

	void reserve(int theCapacity)
	{
		if (mCapacity < theCapacity)
		{
			mCapacity = theCapacity;

			SexyGLVertex *aNewList = new SexyGLVertex[theCapacity];
			memcpy (aNewList, mVerts, mSize * sizeof (mVerts[0]));
			if (mVerts != mStackVerts)
				delete mVerts;

			mVerts = aNewList;
		}
	}

	void push_back(const SexyGLVertex &theVert)
	{
		if (mSize == mCapacity)
			reserve (mCapacity * 2);

		mVerts[mSize++] = theVert;
	}

	void operator = (const VertexList &theList)
	{
		reserve (theList.mSize);
		mSize = theList.mSize;
		memcpy (mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
	}


	SexyGLVertex& operator [](int thePos)
	{
		return mVerts[thePos];
	}

	int size()
	{
		return mSize;
	}

	void clear()
	{
		mSize = 0;
	}
};

static inline float GetCoord (const SexyGLVertex &theVertex, int theCoord)
{
	switch (theCoord)
	{
		case 0: return theVertex.sx;
		case 1: return theVertex.sy;
		case 2: return theVertex.sz;
		case 3: return theVertex.tu;
		case 4: return theVertex.tv;
		default: return 0;
	}
}

static inline SexyGLVertex Interpolate (const SexyGLVertex &v1,
					const SexyGLVertex &v2,
					float t)
{
	SexyGLVertex aVertex = v1;

	aVertex.sx = v1.sx + t * (v2.sx - v1.sx);
	aVertex.sy = v1.sy + t * (v2.sy - v1.sy);
	aVertex.tu = v1.tu + t * (v2.tu - v1.tu);
	aVertex.tv = v1.tv + t * (v2.tv - v1.tv);
	if (v1.color != v2.color)
	{

		int r = v1.color.r + (int)(t * (v2.color.r - v1.color.r));
		int g = v1.color.g + (int)(t * (v2.color.g - v1.color.g));
		int b = v1.color.b + (int)(t * (v2.color.b - v1.color.b));
		int a = v1.color.a + (int)(t * (v2.color.a - v1.color.a));

		aVertex.color = Color (r,g,b,a).ToRGBA();
	}

	return aVertex;
}

template<class Pred>
struct PointClipper
{
	Pred mPred;

	void ClipPoint (int n, float clipVal, const SexyGLVertex &v1,
			const SexyGLVertex &v2, VertexList &out);
	void ClipPoints (int n, float clipVal, VertexList &in, VertexList &out);
};

template<class Pred>
void PointClipper<Pred>::ClipPoint (int n, float clipVal, const SexyGLVertex &v1,
				    const SexyGLVertex &v2, VertexList &out)
{
	if (!mPred (GetCoord (v1, n), clipVal))
	{
		if (!mPred (GetCoord(v2, n), clipVal)) // both inside
			out.push_back (v2);
		else // inside -> outside
		{
			float t = (clipVal - GetCoord(v1, n)) / (GetCoord (v2, n) - GetCoord(v1, n));
			out.push_back (Interpolate (v1, v2, t));
		}
	}
	else
	{
		if (!mPred (GetCoord (v2, n), clipVal)) // outside -> inside
		{
			float t = (clipVal - GetCoord (v1, n)) / (GetCoord (v2, n) - GetCoord (v1, n));
			out.push_back (Interpolate (v1, v2, t));
			out.push_back (v2);
		}
		//else outside -> outside
	}
}

template<class Pred>
void PointClipper<Pred>::ClipPoints (int n, float clipVal, VertexList &in, VertexList &out)
{
	if (in.size() < 2)
		return;

	ClipPoint (n, clipVal, in[in.size () - 1], in[0], out);
	for (VertexList::size_type i = 0; i < in.size ()-1; i++)
		ClipPoint (n, clipVal, in[i], in[i + 1], out);
}

static void DrawPolyClipped(const Rect *theClipRect, const VertexList &theList)
{
	VertexList l1, l2;
	l1 = theList;

	int left = theClipRect->mX;
	int right = left + theClipRect->mWidth;
	int top = theClipRect->mY;
	int bottom = top + theClipRect->mHeight;

	VertexList *in = &l1, *out = &l2;
	PointClipper<std::less<float> > aLessClipper;
	PointClipper<std::greater_equal<float> > aGreaterClipper;

	aLessClipper.ClipPoints (0,left,*in,*out);
	std::swap (in, out);
	out->clear ();

	aLessClipper.ClipPoints (1, top, *in, *out);
	std::swap (in, out);
	out->clear ();

	aGreaterClipper.ClipPoints (0, right, *in, *out);
	std::swap (in,out);
	out->clear ();

	aGreaterClipper.ClipPoints (1, bottom, *in, *out);

	VertexList &aList = *out;

        if (aList.size () >= 3) {
		glBegin (GL_TRIANGLE_FAN);
		for (int i = 0; i < aList.size(); ++i) {
			glColor4ub (aList[i].color.r, aList[i].color.g,
				    aList[i].color.b, aList[i].color.a);
			glTexCoord2f (aList[i].tu, aList[i].tv);
			glVertex2f (aList[i].sx, aList[i].sy);
		}
		glEnd ();
        }
}

static void DoPolyTextureClip (VertexList &theList)
{
	VertexList l2;

	float left = 0;
	float right = 1;
	float top = 0;
	float bottom = 1;

	VertexList *in = &theList, *out = &l2;
	PointClipper<std::less<float> > aLessClipper;
	PointClipper<std::greater_equal<float> > aGreaterClipper;

	aLessClipper.ClipPoints (3, left, *in, *out);
	std::swap (in, out);
	out->clear ();

	aLessClipper.ClipPoints (4, top, *in, *out);
	std::swap (in,out);
	out->clear();

	aGreaterClipper.ClipPoints (3, right, *in, *out);
	std::swap (in,out);
	out->clear();

	aGreaterClipper.ClipPoints (4, bottom, *in, *out);
}

void GLTexture::BltTransformed (const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor,
				const Rect *theClipRect, float theX, float theY, bool center)
{
	int srcLeft = theSrcRect.mX;
	int srcTop = theSrcRect.mY;
	int srcRight = srcLeft + theSrcRect.mWidth;
	int srcBottom = srcTop + theSrcRect.mHeight;
	int srcX, srcY;
	float dstX, dstY;
	int aWidth;
	int aHeight;
	float u1,v1,u2,v2;
	float startx = 0, starty = 0;
	float pixelcorrect = 0.0f;//0.5f;

	if (center)
	{
		startx = -theSrcRect.mWidth / 2.0f;
		starty = -theSrcRect.mHeight / 2.0f;
		pixelcorrect = 0.0f;
	}

	srcY = srcTop;
	dstY = starty;

	SexyRGBA rgba = theColor.ToRGBA();

	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

        glEnable (GL_TEXTURE_2D);
        glColor4ub (rgba.r, rgba.g, rgba.b, rgba.a);

	while (srcY < srcBottom)
	{
		srcX = srcLeft;
		dstX = startx;
		while (srcX < srcRight)
		{
			aWidth = srcRight - srcX;
			aHeight = srcBottom - srcY;
			GLuint aTexture = GetTexture (srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

			float x = dstX;// - pixelcorrect; // - 0.5f; //FIXME correct??
			float y = dstY;// - pixelcorrect; // - 0.5f;

			SexyVector2 p[4] = { SexyVector2 (x, y),           SexyVector2 (x,          y + aHeight),
					     SexyVector2 (x + aWidth, y) , SexyVector2 (x + aWidth, y + aHeight) };
			SexyVector2 tp[4];

			int i;
			for (i = 0; i < 4; i++)
			{
				tp[i] = theTrans * p[i];
				tp[i].x -= pixelcorrect - theX;
				tp[i].y -= pixelcorrect - theY;
			}

			bool clipped = false;
			if (theClipRect != NULL)
			{
				int left = theClipRect->mX;
				int right = left + theClipRect->mWidth;
				int top = theClipRect->mY;
				int bottom = top + theClipRect->mHeight;
				for (i = 0; i < 4; i++)
				{
					if (tp[i].x < left || tp[i].x >= right ||
					    tp[i].y < top || tp[i].y >= bottom)
					{
						clipped = true;
						break;
					}
				}
			}

                        glBindTexture (GL_TEXTURE_2D, aTexture);

                        if (!clipped) {
				glBegin (GL_TRIANGLE_STRIP);
				glTexCoord2f (u1, v1);
				glVertex2f (tp[0].x,tp[0].y);
				glTexCoord2f (u1, v2);
				glVertex2f (tp[1].x,tp[1].y);
				glTexCoord2f (u2, v1);
				glVertex2f (tp[2].x,tp[2].y);
				glTexCoord2f (u2, v2);
				glVertex2f (tp[3].x,tp[3].y);
				glEnd ();
                        }
                        else
			{
				VertexList aList;

                                SexyGLVertex vertex0 = {(GLfloat)u1, (GLfloat)v1, rgba,
							(GLfloat)tp[0].x, (GLfloat)tp[0].y};
                                SexyGLVertex vertex1 = {(GLfloat)u1, (GLfloat)v2, rgba,
							(GLfloat)tp[1].x, (GLfloat)tp[1].y};
                                SexyGLVertex vertex2 = {(GLfloat)u2, (GLfloat)v1, rgba,
							(GLfloat)tp[2].x, (GLfloat)tp[2].y};
                                SexyGLVertex vertex3 = {(GLfloat)u2, (GLfloat)v2, rgba,
							(GLfloat)tp[3].x, (GLfloat)tp[3].y};

				aList.push_back (vertex0);
				aList.push_back (vertex1);
				aList.push_back (vertex3);
				aList.push_back (vertex2);

                                DrawPolyClipped (theClipRect, aList);
			}
			srcX += aWidth;
			dstX += aWidth;

		}
		srcY += aHeight;
		dstY += aHeight;
	}
}

#define GetColorFromTriVertex(theVertex, theColor) (theVertex.color ? theVertex.color : theColor)

void GLTexture::BltTriangles (const TriVertex theVertices[][3], int theNumTriangles, uint32 theColor, float tx, float ty)
{
	glEnable(GL_TEXTURE_2D); //FIXME only set this at start of drawing all

	if ((mMaxTotalU <= 1.0) && (mMaxTotalV <= 1.0))
	{
		glBindTexture(GL_TEXTURE_2D, mTextures[0].mTexture);

		SexyGLVertex aVertexCache[300];
		int aVertexCacheNum = 0;

		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glEnableClientState (GL_COLOR_ARRAY);
		glEnableClientState (GL_VERTEX_ARRAY);

		glTexCoordPointer (2, GL_FLOAT, 5 * sizeof(GLfloat) + 4 * sizeof(GLubyte), aVertexCache);
		glColorPointer (4, GL_UNSIGNED_BYTE, 5 * sizeof(GLfloat) + 4 * sizeof(GLubyte),
				((GLubyte*)aVertexCache) + 2 * sizeof(GLfloat));
		glVertexPointer (3, GL_FLOAT, 5 * sizeof(GLfloat) + 4 * sizeof(GLubyte),
				 ((GLubyte*)aVertexCache) + 2 * sizeof(GLfloat) + 4 * sizeof(GLubyte));

		for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++)
		{
			Color col;
			TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];
			SexyGLVertex* aVertex = &aVertexCache[aVertexCacheNum];

			aVertexCacheNum += 3;

			aVertex[0].sx = aTriVerts[0].x + tx;
			aVertex[0].sy = aTriVerts[0].y + ty;
			aVertex[0].sz = 0;
			col = GetColorFromTriVertex(aTriVerts[0],theColor);
			aVertex[0].color = col.ToRGBA();
			aVertex[0].tu = aTriVerts[0].u * mMaxTotalU;
			aVertex[0].tv = aTriVerts[0].v * mMaxTotalV;

			aVertex[1].sx = aTriVerts[1].x + tx;
			aVertex[1].sy = aTriVerts[1].y + ty;
			aVertex[1].sz = 0;
			col = GetColorFromTriVertex(aTriVerts[0],theColor);
			aVertex[1].color = col.ToRGBA();
			aVertex[1].tu = aTriVerts[1].u * mMaxTotalU;
			aVertex[1].tv = aTriVerts[1].v * mMaxTotalV;

			aVertex[2].sx = aTriVerts[2].x + tx;
			aVertex[2].sy = aTriVerts[2].y + ty;
			aVertex[2].sz = 0;
			col = GetColorFromTriVertex(aTriVerts[0],theColor);
			aVertex[2].color = col.ToRGBA();
			aVertex[2].tu = aTriVerts[2].u * mMaxTotalU;
			aVertex[2].tv = aTriVerts[2].v * mMaxTotalV;

			if ((aVertexCacheNum == 300) || (aTriangleNum == theNumTriangles - 1))
			{


				glDrawArrays (GL_TRIANGLES, 0, 300);

				if (aTriangleNum == theNumTriangles - 1) {
					glDisableClientState (GL_TEXTURE_COORD_ARRAY);
					glDisableClientState (GL_COLOR_ARRAY);
					glDisableClientState (GL_VERTEX_ARRAY);
				}


				aVertexCacheNum = 0;
			}
		}
	}
	else
	{
		for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++)
		{
			TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];

			SexyGLVertex aVertex[3];
			Color col = GetColorFromTriVertex(aTriVerts[0],theColor);

			SexyGLVertex vertex1 = {(GLfloat)(aTriVerts[0].u * mMaxTotalU),
						(GLfloat)(aTriVerts[0].v * mMaxTotalV),
						col.ToRGBA(),
						aTriVerts[0].x + tx, aTriVerts[0].u + ty, 0.0f};

			col = GetColorFromTriVertex(aTriVerts[1],theColor);

			SexyGLVertex vertex2 = {(GLfloat)(aTriVerts[1].u * mMaxTotalU),
						(GLfloat)(aTriVerts[1].v * mMaxTotalV),
						col.ToRGBA(),
						aTriVerts[1].x + tx, aTriVerts[1].u + ty, 0.0f};
			col = GetColorFromTriVertex (aTriVerts[2],theColor);

			SexyGLVertex vertex3 = {(GLfloat)(aTriVerts[2].u * mMaxTotalU),
						(GLfloat)(aTriVerts[2].v * mMaxTotalV),
						col.ToRGBA(),
						aTriVerts[2].x + tx, aTriVerts[2].u + ty, 0.0f};

			aVertex[0] = vertex1;
			aVertex[1]  = vertex2;
			aVertex[2] = vertex3;

			float aMinU = mMaxTotalU, aMinV = mMaxTotalV;
			float aMaxU = 0, aMaxV = 0;

			int i,j,k;
			for (i = 0; i < 3; i++)
			{
				if (aVertex[i].tu < aMinU)
					aMinU = aVertex[i].tu;

				if (aVertex[i].tv < aMinV)
					aMinV = aVertex[i].tv;

				if (aVertex[i].tu > aMaxU)
					aMaxU = aVertex[i].tu;

				if (aVertex[i].tv > aMaxV)
					aMaxV = aVertex[i].tv;
			}

			VertexList aMasterList;
			aMasterList.push_back (aVertex[0]);
			aMasterList.push_back (aVertex[1]);
			aMasterList.push_back (aVertex[2]);

			VertexList aList;

			int aLeft = (int)floorf (aMinU);
			int aTop = (int)floorf (aMinV);
			int aRight = (int)ceilf (aMaxU);
			int aBottom = (int)ceilf (aMaxV);
			if (aLeft < 0)
				aLeft = 0;
			if (aTop < 0)
				aTop = 0;
			if (aRight > mTexVecWidth)
				aRight = mTexVecWidth;
			if (aBottom > mTexVecHeight)
				aBottom = mTexVecHeight;

			GLTextureBlock &aStandardBlock = mTextures[0];
			for (i = aTop; i < aBottom; i++)
			{
				for (j = aLeft; j < aRight; j++)
				{
					GLTextureBlock &aBlock = mTextures[i * mTexVecWidth + j];


					VertexList aList = aMasterList;
					for (k = 0; k < 3; k++)
					{
						aList[k].tu -= j;
						aList[k].tv -= i;
						if (i == mTexVecHeight-1)
							aList[k].tv *= (float)aStandardBlock.mHeight / aBlock.mHeight;
						if (j == mTexVecWidth-1)
							aList[k].tu *= (float)aStandardBlock.mWidth / aBlock.mWidth;
					}

					DoPolyTextureClip (aList);
					if (aList.size() >= 3)
					{
						glBindTexture (GL_TEXTURE_2D, aBlock.mTexture);
						glBegin (GL_TRIANGLE_FAN);
						for (int i = 0; i < aList.size (); ++i) {
							glTexCoord2f (aList[i].tu, aList[i].tv);
							glColor4ub (aList[i].color.r, aList[i].color.g,
								    aList[i].color.b, aList[i].color.a);
							glVertex3f (aList[i].sx, aList[i].sy, aList[i].sz);
						}
						glEnd ();
					}
				}
			}
		}
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
	if (mInterface->GetScreenImage () != this)
		return false;

	SexyRGBA aColor = theColor.ToRGBA();

	VertexList aList;
	for (int i = 0; i < theNumVertices; i++)
	{
		SexyGLVertex vert = {
			0, 0, aColor, theVertices[i].mX + tx, theVertices[i].mY + ty, 0
		};
		aList.push_back (vert);
	}

	if (theClipRect != NULL) {
		DrawPolyClipped (theClipRect, aList);
	} else {
		glDisable (GL_TEXTURE_2D);
		glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);
		glBegin (GL_TRIANGLE_FAN);
		for (int i = 0; i < aList.size(); ++i)
			glVertex2f (aList[i].sx, aList[i].sy);
		glEnd ();
        }

	return true;
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

void GLImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY,
			     const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY,
			       const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY,
		       const Color& theColor, int theDrawMode)
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

void GLImage::NormalDrawLineAA(double theStartX, double theStartY,
			       double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::AdditiveDrawLineAA(double theStartX, double theStartY,
				 double theEndX, double theEndY, const Color& theColor)
{
	TRACE_THIS();
}

void GLImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY,
			 const Color& theColor, int theDrawMode)
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
	mNumRows = theWidth;
	mNumCols = theHeight;

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

	if (mInterface->GetScreenImage() == this)
	{
		//glReadPixels ()?
		TRACE_THIS ();
	}
	return bits;
}

void GLImage::Clear()
{
	TRACE_THIS();
}

void GLImage::NormalFillRect(const Rect& theRect, const Color& theColor)
{
	TRACE_THIS ();
}

void GLImage::AdditiveFillRect(const Rect& theRect, const Color& theColor)
{
	TRACE_THIS ();
}

void GLImage::NormalBlt(Image* theImage, int theX, int theY,
			const Rect& theSrcRect, const Color& theColor)
{
	TRACE_THIS ();
}

void GLImage::NormalBltMirror(Image* theImage, int theX, int theY,
			      const Rect& theSrcRectOrig, const Color& theColor)
{
	TRACE_THIS ();
}

void GLImage::AdditiveBlt(Image* theImage, int theX, int theY,
			  const Rect& theSrcRect, const Color& theColor)
{
	TRACE_THIS ();
}

void GLImage::AdditiveBltMirror(Image* theImage, int theX, int theY,
				const Rect& theSrcRectOrig, const Color& theColor)
{
	TRACE_THIS ();
}

void GLImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect,
		  const Color& theColor, int theDrawMode)
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

void GLImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect,
			const Color& theColor, int theDrawMode)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::BltMirror(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
		return;
	}

	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);
	if (!srcImage)
		return;

	srcImage->EnsureTexture();
	if (!srcImage->mTexture)
		return;

	SexyTransform2D aTransform;

	aTransform.Translate (-theSrcRect.mWidth, 0);
	aTransform.Scale (-1, 1);
	aTransform.Translate (theX, theY);

	BltTransformed (theImage, NULL, theColor, theDrawMode, theSrcRect, aTransform);
}

void GLImage::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect,
		   const Color& theColor, int theDrawMode)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::BltF (theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode);
		return;
	}

	FRect aClipRect (theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight);
	FRect aDestRect (theX, theY, theSrcRect.mWidth, theSrcRect.mHeight);

	FRect anIntersect = aDestRect.Intersection (aClipRect);
	if (anIntersect.mWidth != aDestRect.mWidth || anIntersect.mHeight != aDestRect.mHeight)
	{
		if (anIntersect.mWidth != 0 && anIntersect.mHeight != 0)
		{
			SexyTransform2D aTransform;
			aTransform.Translate( theX, theY);

			BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform);
		}
	}
	else
	{
		GLImage::Blt (theImage, theX, theY, theSrcRect, theColor, theDrawMode);
	}
}

void GLImage::BltTransformed (Image* theImage, const Rect* theClipRect, const Color& theColor,
			      int theDrawMode, const Rect &theSrcRect,
			      const SexyMatrix3 &theTransform,
			      float theX, float theY, bool center)
{
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
	srcImage->mTexture->BltTransformed (theTransform, theSrcRect, theColor,
					    theClipRect, theX, theY, center);
}

void GLImage::BltRotated (Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::BltRotated (theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode,
					 theRot, theRotCenterX, theRotCenterY);
		return;
	}

	SexyTransform2D aTransform;

	aTransform.Translate (-theRotCenterX, -theRotCenterY);
	aTransform.RotateRad (theRot);
	aTransform.Translate (theX + theRotCenterX, theY + theRotCenterY);

	BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform);
}

void GLImage::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::StretchBlt (theImage, theDestRect, theSrcRect, theClipRect,
					 theColor, theDrawMode, fastStretch);
		return;
	}

	float xScale = (float)theDestRect.mWidth / theSrcRect.mWidth;
	float yScale = (float)theDestRect.mHeight / theSrcRect.mHeight;

	SexyTransform2D aTransform;
	bool mirror = false;
	if (mirror)
	{
		aTransform.Translate (-theSrcRect.mWidth, 0);
		aTransform.Scale (-xScale, yScale);
	}
	else
	{
		aTransform.Scale (xScale, yScale);
	}

	aTransform.Translate (theDestRect.mX, theDestRect.mY);
	BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform);
}

void GLImage::StretchBltMirror(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect,
			       const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::StretchBltMirror (theImage, theDestRect, theSrcRect, theClipRect,
					       theColor, theDrawMode, fastStretch);
		return;
	}

	float xScale = (float)theDestRect.mWidth / theSrcRect.mWidth;
	float yScale = (float)theDestRect.mHeight / theSrcRect.mHeight;

	SexyTransform2D aTransform;
	bool mirror = true;
	if (mirror)
	{
		aTransform.Translate (-theSrcRect.mWidth, 0);
		aTransform.Scale (-xScale, yScale);
	}
	else
	{
		aTransform.Scale (xScale, yScale);
	}

	aTransform.Translate (theDestRect.mX, theDestRect.mY);
	BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform);
}

void GLImage::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect,
			const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::BltMatrix (theImage, x, y, theMatrix, theClipRect, theColor,
					theDrawMode, theSrcRect, blend);
		return;
	}

	BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, theMatrix, blend, x, y);
}

void GLImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles,
			      const Rect& theClipRect, const Color &theColor, int theDrawMode,
			      float tx, float ty, bool blend)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::BltTrianglesTex (theTexture, theVertices, theNumTriangles, theClipRect, theColor,
					      theDrawMode, tx, ty, blend);
		return;
	}

	GLImage * srcImage = dynamic_cast<GLImage*>(theTexture);
        if (srcImage)
 		return;

	srcImage->EnsureTexture();
	if (!srcImage->mTexture)
		return;

	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);
	srcImage->mTexture->BltTriangles (theVertices, theNumTriangles, (uint32)theColor.ToInt(), tx, ty);
}

bool GLImage::Palletize()
{
	return false;
}

void GLImage::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor,
					int theDrawMode, const BYTE* theCoverage, int theCoverX, int theCoverY,
					int theCoverWidth, int theCoverHeight)
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
