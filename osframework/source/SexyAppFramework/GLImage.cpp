#include <math.h>

#include "GLImage.h"
#include "GLDisplay.h"
#include "Rect.h"
#include "Graphics.h"
#include "SexyAppBase.h"
#include "SexyVector.h"
#include "SexyMatrix.h"
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

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_BGRA
#define GL_BGRA	 0x80E1
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif

#ifdef SEXY_OPENGLES
#define ftofix(f) (GLfixed)(f * 65536.0f)
#endif

namespace Sexy {

struct GLTextureBlock
{
	GLuint	  mTexture;
	GLshort	  mWidth;
	GLshort	  mHeight;
};

class GLTexture
{
public:
	typedef std::vector<GLTextureBlock> TextureVector;

	TextureVector		  mTextures;
	int			  mWidth;
	int			  mHeight;
	int			  mTexVecWidth;
	int			  mTexVecHeight;
	int			  mTexBlockWidth;
	int			  mTexBlockHeight;
	int			  mBitsChangedCount;
	DWORD			  mTexMemSize;
	float			  mMaxTotalU;
	float			  mMaxTotalV;

	bool			  mRectangleTexture;
	GLenum			  mTarget;

	GLDisplay*		  mInterface;
	int			  mImageFlags;
	Image*			  mImage;

public:
	GLTexture(GLDisplay* theInterface, Image* theImage);
	~GLTexture();

	void			  SetTextureFilter(bool linear);
	void			  ReleaseTextures ();
	void			  CreateTextureDimensions (MemoryImage *theImage);
	void			  CreateTextures (MemoryImage *theImage);
	bool			  CheckCreateTextures (MemoryImage *theImage);

	GLuint			  GetTexture (int x, int y, int &width, int &height, float &u1, float &v1,
					      float &u2, float &v2);
	GLuint			  GetTextureF (float x, float y, float &width, float &height,
					       float &u1, float &v1, float &u2, float &v2);

	void			  Blt (float theX, float theY, const Rect& theSrcRect, const Color& theColor);
	void			  BltTransformed (const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor,
						  const Rect *theClipRect = NULL, float theX = 0, float theY = 0, bool center = false);
	void			  BltTriangles (const TriVertex theVertices[][3], int theNumTriangles, uint32 theColor,
						float tx = 0, float ty = 0);
};
}

static inline int
multiply_alpha (int alpha, int color)
{
    int temp = (alpha * color) + 0x80;
    return ((temp + (temp >> 8)) >> 8);
}

static inline uint
multiply_pixel (uint pixel)
{
	if (!pixel)
		return 0;

	unsigned char  alpha = pixel >> 24;
	unsigned char  red   = (pixel >> 16) & 0xff;
	unsigned char  green = (pixel >>  8) & 0xff;
	unsigned char  blue  = (pixel >>  0) & 0xff;

	if (alpha != 0xff)
	{
		red   = multiply_alpha (alpha, red);
		green = multiply_alpha (alpha, green);
		blue  = multiply_alpha (alpha, blue);
	}
	return	(alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
}

static inline void
multiply_rgba(SexyRGBA &rgba)
{
       rgba.r = multiply_alpha(rgba.r, rgba.a);
       rgba.g = multiply_alpha(rgba.g, rgba.a);
       rgba.b = multiply_alpha(rgba.b, rgba.a);
}

static inline SexyRGBA
ColorToMultipliedRGBA(Color theColor)
{
	SexyRGBA rgba = theColor.ToRGBA();
	multiply_rgba(rgba);
	return rgba;
}

static GLuint CreateTexture (GLDisplay * theInterface, MemoryImage* theImage,
			     GLuint old, int x, int y, int width, int height)
{
	GLuint texture;
	int w, h;

	/* Use the texture width and height expanded to powers of 2 */
	w = RoundToPOT (width);
	h = RoundToPOT (height);
	assert (w == width && h == height);

	//printf ("texture: %dx%d ==> %dx%d\n", width, height, w, h);

	/* Create an OpenGL texture for the image */
	if (!old)
	{
		glGenTextures (1, &texture);
		if (texture == 0)
		{
			printf ("Generating a texture failed.");
			return 0;
		}
	}

	int target = theInterface->GetTextureTarget();
	glBindTexture (target, old ? old : texture);
	glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	uint32* bits = theImage->mBits;
	uint32* tab = theImage->mColorTable;
	uchar*	indices = theImage->mColorIndices;
	uint32* copy = new uint32[w * h];
	if (copy) {
		int i;

		int aWidth = std::min (w, (theImage->GetWidth() - x));
		int aHeight = std::min (h, (theImage->GetHeight() - y));
#if 0
		int aWidthExtra = w > aWidth ? w - aWidth : 0;
		int aHeightExtra = h > aHeight ? h - aHeight : 0;
#endif
		GLenum format, intlformat;

		if (theInterface->mTexBGRA == GL_TRUE)
		{
#if !defined(SEXY_OPENGLES)
			intlformat = GL_RGBA;
#else
			intlformat = GL_BGRA;
#endif
			format = GL_BGRA;
		}
		else
		{
			intlformat = GL_RGBA;
			format = GL_RGBA;
		}
		int imageWidth = theImage->GetWidth();
		uint32 * dst = copy;

		if (indices)
		{
			uint32 pmtab[256];
			uchar * src = indices + y * imageWidth + x;

			for (i = 0; i < 256; i++)
			{
				uint32 pixel = multiply_pixel (tab[i]);
				if (format == GL_RGBA)
					pmtab[i] =
						(pixel	& 0xff00ff00) |
						((pixel & 0x00ff0000) >> 16) |
						((pixel & 0x000000ff) << 16);
				else
					pmtab[i] = pixel;
			}

			for (i = 0; i < aHeight; i++)
			{
				int j;

				for (j = 0; j < aWidth; j++)
					dst[j] = pmtab[src[j]];

				for (j = aWidth; j < w; j++)
					dst[j] = dst[aWidth - 1];

				dst += w;
				src += imageWidth;
			}
		}
		else if (bits)
		{
			uint32 * src = bits + y * imageWidth + x;

			for (i = 0; i < aHeight; i++)
			{
				int j;

				if (format == GL_RGBA)
				{
					for (j = 0; j < aWidth; j++)
					{
						uint32 pixel = multiply_pixel (src[j]);
						dst[j] =
							(pixel	& 0xff00ff00) |
							((pixel & 0x00ff0000) >> 16) |
							((pixel & 0x000000ff) << 16);
					}
				}
				else
				{
					for (j = 0; j < aWidth; j++)
						dst[j] = multiply_pixel (src[j]);
				}

				for (j = aWidth; j < w; j++)
					dst[j] = dst[aWidth - 1];

				dst += w;
				src += imageWidth;
			}
		}
		else
		{
			memset(copy, 0, w * h * sizeof(uint32));
			i = h;
		}
		for (; i < h; i++, dst += w)
			memcpy (dst, copy + w * (aHeight - 1), w * 4);

		glTexImage2D (target,
			      0,
			      intlformat,
			      w, h,
			      0,
			      format,
			      GL_UNSIGNED_BYTE,
			      copy);
		delete [] copy;
	}

	return old ? old : texture;
}

GLTexture::GLTexture (GLDisplay *theInterface, Image* theImage)
{
	mWidth = 0;
	mHeight = 0;
	mTexVecWidth = 0;
	mTexVecHeight = 0;
	mBitsChangedCount = 0;
	mTexMemSize = 0;
	mTexBlockWidth = 64;
	mTexBlockHeight = 64;
	mRectangleTexture = false;
	mTarget = 0;
	mInterface = theInterface;
	mImageFlags = 0;
	mImage = theImage;
}

GLTexture::~GLTexture ()
{
	ReleaseTextures ();
}

void GLTexture::SetTextureFilter(bool linear)
{
	linear = true;
	if (linear)
	{
		glTexParameteri (mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri (mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
}

static void SetBlendFunc(int theDrawMode, bool premultiply = false)
{
	if (!premultiply)
	{
		if (theDrawMode == Graphics::DRAWMODE_NORMAL)
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc (GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		if (theDrawMode == Graphics::DRAWMODE_NORMAL)
			glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc (GL_ONE, GL_ONE);
	}
}

void GLTexture::ReleaseTextures ()
{
	assert (mTexMemSize == mImage->mTexMemSize);
	mInterface->FreeTexMemSpace(mTexMemSize);

	for(size_t i = 0; i < mTextures.size(); i++)
		mInterface->DelayedDeleteTexture(mTextures[i].mTexture);

	mTextures.clear();
	mTexMemSize = 0;
	mImage->mTexMemSize = 0;
}

void GLTexture::CreateTextureDimensions (MemoryImage* theImage)
{
	int aWidth = theImage->GetWidth ();
	int aHeight = theImage->GetHeight ();
	int i;

	// Calculate inner block sizes
	mTexBlockWidth = aWidth;
	mTexBlockHeight = aHeight;
	bool usePOT = true;
	bool miniSubDiv = (theImage->mFlags & IMAGE_FLAGS_MINI_SUBDIV) != 0;

	mTarget = mInterface->GetTextureTarget();
	mInterface->CalulateBestTexDimensions (mTexBlockWidth, mTexBlockHeight, miniSubDiv, usePOT);

	// Calculate right boundary block sizes
	int aRightWidth = aWidth % mTexBlockWidth;
	int aRightHeight = mTexBlockHeight;
	if (aRightWidth > 0)
		mInterface->CalulateBestTexDimensions (aRightWidth, aRightHeight, true, usePOT);
	else
		aRightWidth = mTexBlockWidth;

	// Calculate bottom boundary block sizes
	int aBottomWidth = mTexBlockWidth;
	int aBottomHeight = aHeight % mTexBlockHeight;
	if (aBottomHeight > 0)
		mInterface->CalulateBestTexDimensions (aBottomWidth, aBottomHeight, true, usePOT);
	else
		aBottomHeight = mTexBlockHeight;

	// Calculate corner block size
	int aCornerWidth = aRightWidth;
	int aCornerHeight = aBottomHeight;
	mInterface->CalulateBestTexDimensions (aCornerWidth, aCornerHeight, true, usePOT);

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

	if (mTarget == GL_TEXTURE_RECTANGLE_ARB)
	{
		u1 = left;
		v1 = top;
		u2 = right;
		v2 = bottom;
	}
	else
	{
		u1 = left / (float)aBlock.mWidth;
		v1 = top / (float)aBlock.mHeight;
		u2 = right / (float)aBlock.mWidth;
		v2 = bottom / (float)aBlock.mHeight;
	}

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

	width = right - left;
	height = bottom - top;

	if (mTarget == GL_TEXTURE_RECTANGLE_ARB)
	{
		u1 = left;
		v1 = top;
		u2 = right;
		v2 = bottom;
	}
	else
	{
		u1 = left / aBlock.mWidth;
		v1 = top / aBlock.mHeight;
		u2 = right / aBlock.mWidth;
		v2 = bottom / aBlock.mHeight;
	}

	return aBlock.mTexture;
}

void GLTexture::CreateTextures(MemoryImage* theImage)
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

	size_t i, x, y;

	size_t aHeight = theImage->GetHeight ();
	size_t aWidth = theImage->GetWidth ();
	int aFormatSize = 4;

	DWORD aTextMemSize = 0;
	for(i = 0; i < mTextures.size(); i++)
	{
		GLTextureBlock &aBlock = mTextures[i];
		aTextMemSize += aBlock.mWidth * aBlock.mHeight * aFormatSize;
	}
	if (!mInterface->EnsureTexMemSpace(aTextMemSize))
		printf ("No enough texture memory!\n");

	i = 0;
	for (y = 0; y < aHeight; y += mTexBlockHeight)
	{
		for (x = 0; x < aWidth; x += mTexBlockWidth, i++)
		{
			GLTextureBlock &aBlock = mTextures[i];
			if (createTextures)
			{
				aBlock.mTexture = CreateTexture (mInterface, theImage, 0, x, y,
								 aBlock.mWidth, aBlock.mHeight);
				if (aBlock.mTexture == 0) // create texture failure
					continue;

				mTexMemSize += aBlock.mWidth * aBlock.mHeight * aFormatSize;
			}
			else
			{
				aBlock.mTexture = CreateTexture (mInterface, theImage, aBlock.mTexture, x, y,
								 aBlock.mWidth, aBlock.mHeight);
			}
		}
	}
	if (createTextures)
		mInterface->AllocTexMemSpace(mTexMemSize);
	theImage->mTexMemSize = mTexMemSize;

	mWidth = theImage->mWidth;
	mHeight = theImage->mHeight;
	mBitsChangedCount = theImage->mBitsChangedCount;
	mImageFlags = theImage->mFlags;
}

bool GLTexture::CheckCreateTextures (MemoryImage *theImage)
{
	if (theImage->mWidth != mWidth || theImage->mHeight != mHeight ||
	    theImage->mBitsChangedCount != mBitsChangedCount ||
	    theImage->mFlags != mImageFlags)
	{
		theImage->GetBits();
		CreateTextures (theImage);
		return true;
	}

	return false;
}

static void GLDrawQuad (float x1, float y1, float x2, float y2,
			float u1, float v1, float u2, float v2)
{
	GLfloat verts[4 * 2];
	verts[0] = x1; verts[1] = y1;
	verts[2] = x1; verts[3] = y2;
	verts[4] = x2; verts[5] = y1;
	verts[6] = x2; verts[7] = y2;

	GLfloat coords[4 * 2];
	coords[0] = u1; coords[1] = v1;
	coords[2] = u1; coords[3] = v2;
	coords[4] = u2; coords[5] = v1;
	coords[6] = u2; coords[7] = v2;

	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);

	glVertexPointer (2, GL_FLOAT, 0, verts);
	glTexCoordPointer (2, GL_FLOAT, 0, coords);
	glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
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

	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

	SexyRGBA rgba = theColor.ToRGBA ();
	multiply_rgba (rgba);
	glColor4ub (rgba.r, rgba.g, rgba.b, rgba.a);

	glEnable (mTarget);
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

			glBindTexture (mTarget, aTexture);

			GLDrawQuad (x, y, x + aWidth, y + aHeight,
				    u1, v1, u2, v2);

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

static void GLDrawVertexList (VertexList& aList)
{
	GLubyte* colors;
	GLfloat* coords;
	GLfloat* verts;

	colors = new GLubyte[4 * aList.size()];
	coords = new GLfloat[2 * aList.size()];
	verts = new GLfloat[2 * aList.size()];
	for (int i = 0; i < aList.size(); ++i) {
		colors[i * 4]	  = aList[i].color.r;
		colors[i * 4 + 1] = aList[i].color.g;
		colors[i * 4 + 2] = aList[i].color.b;
		colors[i * 4 + 3] = aList[i].color.a;
		coords[i * 2]	  = aList[i].tu;
		coords[i * 2 + 1] = aList[i].tv;
		verts[i * 2]	  = aList[i].sx;
		verts[i * 2 + 1]  = aList[i].sy;
	}

	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_COLOR_ARRAY);

	glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
	glVertexPointer (2, GL_FLOAT, 0, verts);
	glTexCoordPointer (2, GL_FLOAT, 0, coords);
	glDrawArrays (GL_TRIANGLE_FAN, 0, aList.size());

	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_COLOR_ARRAY);

	delete [] colors;
	delete [] coords;
	delete [] verts;
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

	if (aList.size () >= 3)
		GLDrawVertexList (aList);
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

static void GLDrawQuadTransformed (float x1, float y1,
				   float x2, float y2,
				   float x3, float y3,
				   float x4, float y4,
				   float u1, float v1,
				   float u2, float v2)
{
	GLfloat verts[4 * 2];
	verts[0] = x1; verts[1] = y1;
	verts[2] = x2; verts[3] = y2;
	verts[4] = x3; verts[5] = y3;
	verts[6] = x4; verts[7] = y4;

	GLfloat coords[4 * 2];
	coords[0] = u1; coords[1] = v1;
	coords[2] = u1; coords[3] = v2;
	coords[4] = u2; coords[5] = v1;
	coords[6] = u2; coords[7] = v2;

	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);

	glVertexPointer (2, GL_FLOAT, 0, verts);
	glTexCoordPointer (2, GL_FLOAT, 0, coords);
	glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
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

	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

	SexyRGBA rgba = theColor.ToRGBA();
	multiply_rgba (rgba);

	glEnable (mTarget);
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

			SexyVector2 p[4] = { SexyVector2 (x, y),	   SexyVector2 (x,	    y + aHeight),
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

			glBindTexture (mTarget, aTexture);

			if (!clipped) {
				GLDrawQuadTransformed (tp[0].x, tp[0].y,
						       tp[1].x, tp[1].y,
						       tp[2].x, tp[2].y,
						       tp[3].x, tp[3].y,
						       u1, v1, u2, v2);
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

static void GLDrawVertexList3 (VertexList& aList)
{
	GLubyte* colors;
	GLfloat* coords;
	GLfloat* verts;

	colors = new GLubyte[4 * aList.size()];
	coords = new GLfloat[2 * aList.size()];
	verts = new GLfloat[3 * aList.size()];
	for (int i = 0; i < aList.size(); ++i) {
		colors[i * 4]	  = aList[i].color.r;
		colors[i * 4 + 1] = aList[i].color.g;
		colors[i * 4 + 2] = aList[i].color.b;
		colors[i * 4 + 3] = aList[i].color.a;
		coords[i * 2]	  = aList[i].tu;
		coords[i * 2 + 1] = aList[i].tv;
		verts[i * 3]	  = aList[i].sx;
		verts[i * 3 + 1]  = aList[i].sy;
		verts[i * 3 + 2]  = aList[i].sz;
	}

	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_COLOR_ARRAY);

	glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
	glVertexPointer (3, GL_FLOAT, 0, verts);
	glTexCoordPointer (2, GL_FLOAT, 0, coords);
	glDrawArrays (GL_TRIANGLE_FAN, 0, aList.size());

	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_COLOR_ARRAY);

	delete [] colors;
	delete [] coords;
	delete [] verts;
}

void GLTexture::BltTriangles (const TriVertex theVertices[][3], int theNumTriangles,
			      uint32 theColor, float tx, float ty)
{
	glEnable(mTarget); //FIXME only set this at start of drawing all

	if ((mMaxTotalU <= 1.0) && (mMaxTotalV <= 1.0))
	{
		glBindTexture(mTarget, mTextures[0].mTexture);

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
			aVertex[0].color = ColorToMultipliedRGBA(col);
			aVertex[0].tu = aTriVerts[0].u * mMaxTotalU;
			aVertex[0].tv = aTriVerts[0].v * mMaxTotalV;

			aVertex[1].sx = aTriVerts[1].x + tx;
			aVertex[1].sy = aTriVerts[1].y + ty;
			aVertex[1].sz = 0;
			col = GetColorFromTriVertex(aTriVerts[0],theColor);
			aVertex[1].color = ColorToMultipliedRGBA(col);
			aVertex[1].tu = aTriVerts[1].u * mMaxTotalU;
			aVertex[1].tv = aTriVerts[1].v * mMaxTotalV;

			aVertex[2].sx = aTriVerts[2].x + tx;
			aVertex[2].sy = aTriVerts[2].y + ty;
			aVertex[2].sz = 0;
			col = GetColorFromTriVertex(aTriVerts[0],theColor);
			aVertex[2].color = ColorToMultipliedRGBA(col);
			aVertex[2].tu = aTriVerts[2].u * mMaxTotalU;
			aVertex[2].tv = aTriVerts[2].v * mMaxTotalV;

			if ((aVertexCacheNum == 300) || (aTriangleNum == theNumTriangles - 1))
			{
				glDrawArrays (GL_TRIANGLES, 0, aVertexCacheNum);
				aVertexCacheNum = 0;
			}
		}
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
		glDisableClientState (GL_COLOR_ARRAY);
		glDisableClientState (GL_VERTEX_ARRAY);
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
						ColorToMultipliedRGBA(col),
						aTriVerts[0].x + tx, aTriVerts[0].y + ty, 0.0f};

			col = GetColorFromTriVertex(aTriVerts[1],theColor);

			SexyGLVertex vertex2 = {(GLfloat)(aTriVerts[1].u * mMaxTotalU),
						(GLfloat)(aTriVerts[1].v * mMaxTotalV),
						ColorToMultipliedRGBA(col),
						aTriVerts[1].x + tx, aTriVerts[1].y + ty, 0.0f};
			col = GetColorFromTriVertex (aTriVerts[2],theColor);

			SexyGLVertex vertex3 = {(GLfloat)(aTriVerts[2].u * mMaxTotalU),
						(GLfloat)(aTriVerts[2].v * mMaxTotalV),
						ColorToMultipliedRGBA(col),
						aTriVerts[2].x + tx, aTriVerts[2].y + ty, 0.0f};

			aVertex[0] = vertex1;
			aVertex[1] = vertex2;
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
						glBindTexture (mTarget, aBlock.mTexture);
						GLDrawVertexList3 (aList);
					}
				}
			}
		}
	}
}

GLTexture* GLImage::EnsureSrcTexture(GLDisplay* theInterface, Image* theImage)
{
	GLImage * srcImage = dynamic_cast<GLImage*>(theImage);

	if (srcImage)
		return srcImage->EnsureTexture();

	MemoryImage* aMemoryImage = (MemoryImage*)theImage;
	GLTexture* aTexture = (GLTexture*)aMemoryImage->mNativeData;
	if (!aTexture)
	{
		aTexture = new GLTexture(theInterface, theImage);
		aMemoryImage->mNativeData = (void*)aTexture;
	}
	aTexture->CheckCreateTextures (aMemoryImage);
	aMemoryImage->mDrawnTime = Sexy::GetTickCount();

	return aTexture;
}

GLImage::GLImage(GLDisplay* theInterface) :
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
	mInterface = dynamic_cast<GLDisplay *>(gSexyAppBase->mDDInterface);
	mTexture = 0;
	Init();
}

GLImage::~GLImage()
{
	DBG_ASSERTE(mLockCount == 0);
	DeleteSurface ();
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
	if (!mTexture)
		return;

	delete mTexture;
	mTexture = 0;
}

void GLImage::ReAttach(NativeDisplay *theNative)
{
	mInterface = (GLDisplay*)theNative;
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
	MemoryImage::DeleteNativeData();
	DeleteSurface();
}

void GLImage::DeleteExtraBuffers()
{
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

bool GLImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect,
			 const Color &theColor, int theDrawMode, int tx, int ty, bool convex)
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

		if (!mTransformStack.empty())
		{
			SexyVector2 v(vert.sx, vert.sy);
			v = mTransformStack.back() * v;
			vert.sx = v.x;
			vert.sy = v.y;
		}
		aList.push_back (vert);
	}

	if (theClipRect != NULL) {
		DrawPolyClipped (theClipRect, aList);
	} else {
		glDisable (GetTextureTarget());
		glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);

		GLfloat* verts;

		verts = new GLfloat[2 * aList.size()];
		for (int i = 0; i < aList.size(); ++i) {
			verts[i * 2]	  = aList[i].sx;
			verts[i * 2 + 1]  = aList[i].sy;
		}

		glEnableClientState (GL_VERTEX_ARRAY);

		glVertexPointer (2, GL_FLOAT, 0, verts);
		glDrawArrays (GL_TRIANGLE_FAN, 0, aList.size());

		glDisableClientState (GL_VERTEX_ARRAY);

		delete [] verts;
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

	glDisable (GetTextureTarget());
	SetBlendFunc(theDrawMode);

	SexyRGBA aColor = theColor.ToRGBA();
	float x = theRect.mX;// - 0.5f;
	float y = theRect.mY;// - 0.5f;
	float aWidth = theRect.mWidth;
	float aHeight = theRect.mHeight;

	glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);
	GLfloat verts[4 * 2];
	verts[0] = x;	       verts[1] = y;
	verts[2] = x;	       verts[3] = y + aHeight;
	verts[4] = x + aWidth; verts[5] = y;
	verts[6] = x + aWidth; verts[7] = y + aHeight;

	if (!mTransformStack.empty())
	{
		SexyVector2 p[4] = { SexyVector2(x, y),
				     SexyVector2(x, y + aHeight),
				     SexyVector2(x + aWidth, y),
				     SexyVector2(x + aWidth, y + aHeight) };

		int i;
		for (i = 0; i < 4; i++)
		{
			p[i] = mTransformStack.back()*p[i];
			p[i].x -= 0.5f;
			p[i].y -= 0.5f;
			verts[i * 2 + 0] = p[i].x;
			verts[i * 2 + 1] = p[i].y;
		}
	}

	glEnableClientState (GL_VERTEX_ARRAY);

	glVertexPointer (2, GL_FLOAT, 0, verts);
	glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState (GL_VERTEX_ARRAY);
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

	glDisable (GetTextureTarget());
	SetBlendFunc (theDrawMode);

	float x1, y1, x2, y2;
	SexyRGBA aColor = theColor.ToRGBA ();

	if (!mTransformStack.empty())
	{
		SexyVector2 p1(theStartX, theStartY);
		SexyVector2 p2(theEndX, theEndY);
		p1 = mTransformStack.back() * p1;
		p2 = mTransformStack.back() * p2;

		x1 = p1.x;
		y1 = p1.y;
		x2 = p2.x;
		y2 = p2.y;
	}
	else
	{
		x1 = theStartX;
		y1 = theStartY;
		x2 = theEndX;
		y2 = theEndY;
	}

	glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);

	GLfloat verts[2 * 2];
	verts[0] = x1; verts[1] = y1;
	verts[2] = x2; verts[3] = y2;

	static GLubyte indices[] = { 0, 1};

	glEnableClientState (GL_VERTEX_ARRAY);

	glVertexPointer (2, GL_FLOAT, 0, verts);
	glDrawElements (GL_LINES, 2, GL_UNSIGNED_BYTE, indices);

	glDisableClientState (GL_VERTEX_ARRAY);
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

	glDisable (GetTextureTarget());
	SetBlendFunc (theDrawMode);

	float x1, y1, x2, y2;
	SexyRGBA aColor = theColor.ToRGBA ();

	x1 = theStartX;
	y1 = theStartY;
	x2 = theEndX;
	y2 = theEndY;

	if (!mTransformStack.empty())
	{
		SexyVector2 p1(theStartX, theStartY);
		SexyVector2 p2(theEndX, theEndY);
		p1 = mTransformStack.back() * p1;
		p2 = mTransformStack.back() * p2;

		x1 = p1.x;
		y1 = p1.y;
		x2 = p2.x;
		y2 = p2.y;
	}
	else
	{
		x1 = theStartX;
		y1 = theStartY;
		x2 = theEndX;
		y2 = theEndY;
	}

	glColor4ub (aColor.r, aColor.g, aColor.b, aColor.a);

	GLfloat verts[2 * 2];
	verts[0] = x1; verts[1] = y1;
	verts[2] = x2; verts[3] = y2;

	static GLubyte indices[] = { 0, 1};

	glEnableClientState (GL_VERTEX_ARRAY);

	glVertexPointer (2, GL_FLOAT, 0, verts);
	glDrawElements (GL_LINES, 2, GL_UNSIGNED_BYTE, indices);

	glDisableClientState (GL_VERTEX_ARRAY);
}

void GLImage::CommitBits()
{
	MemoryImage::CommitBits();
}

void GLImage::Create(int theWidth, int theHeight)
{
	MemoryImage::Create(theWidth, theHeight);
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
	ClearRect(Rect (0, 0, mWidth, mHeight));
}

void GLImage::ClearRect(const Rect& theRect)
{
	if (mInterface->GetScreenImage () != this)
	{
		MemoryImage::ClearRect (theRect);
		return;
	}

	glDisable (GetTextureTarget());
	glDisable (GL_BLEND);

	SexyRGBA aColor = Color(0, 0, 0, 0).ToRGBA();
	float x = theRect.mX;// - 0.5f;
	float y = theRect.mY;// - 0.5f;
	float aWidth = theRect.mWidth;
	float aHeight = theRect.mHeight;


	glColor4ub (0, 0, 0, 0);

	GLfloat verts[4 * 2];
	verts[0] = x;	       verts[1] = y;
	verts[2] = x;	       verts[3] = y + aHeight;
	verts[4] = x + aWidth; verts[5] = y;
	verts[6] = x + aWidth; verts[7] = y + aHeight;

	if (!mTransformStack.empty())
	{
		SexyVector2 p[4] = { SexyVector2(x, y),
				     SexyVector2(x, y + aHeight),
				     SexyVector2(x + aWidth, y),
				     SexyVector2(x + aWidth, y + aHeight) };

		int i;
		for (i = 0; i < 4; i++)
		{
			p[i] = mTransformStack.back()*p[i];
			p[i].x -= 0.5f;
			p[i].y -= 0.5f;
			verts[i * 2 + 0] = p[i].x;
			verts[i * 2 + 1] = p[i].y;
		}
	}

	glEnableClientState (GL_VERTEX_ARRAY);

	glVertexPointer (2, GL_FLOAT, 0, verts);
	glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState (GL_VERTEX_ARRAY);

	glEnable (GL_BLEND);
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

void GLImage::Bltf(Image* theImage, float theX, float theY, const Rect& theSrcRect,
		   const Color& theColor, int theDrawMode)
{
	GLTexture *aTexture = EnsureSrcTexture(mInterface, theImage);
	if (!aTexture)
		return;

	SetBlendFunc (theDrawMode, true);
	aTexture->SetTextureFilter(true);
	aTexture->Blt (theX, theY, theSrcRect, theColor);
}

void GLImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect,
		  const Color& theColor, int theDrawMode)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::Blt(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
		return;
	}

	if (!mTransformStack.empty ())
	{
		SexyTransform2D aTransform;
		aTransform.Translate(theX, theY);

		BltTransformed(theImage, NULL,theColor, theDrawMode, theSrcRect,
			       aTransform, true);
		return;
	}

	GLTexture *aTexture = EnsureSrcTexture(mInterface, theImage);
	if (!aTexture)
		return;

	SetBlendFunc (theDrawMode, true);
	aTexture->SetTextureFilter(true);
	aTexture->Blt (theX, theY, theSrcRect, theColor);
}

void GLImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect,
			const Color& theColor, int theDrawMode)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::BltMirror(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
		return;
	}

	SexyTransform2D aTransform;

	aTransform.Translate (-theSrcRect.mWidth, 0);
	aTransform.Scale (-1, 1);
	aTransform.Translate (theX, theY);

	BltTransformed (theImage, NULL, theColor, theDrawMode, theSrcRect, aTransform, false);
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
			aTransform.Translate(theX, theY);

			BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect,
					aTransform, true);
		}
	}
	else
	{
		if (!mTransformStack.empty ())
		{
			SexyTransform2D aTransform;
			aTransform.Translate(theX, theY);

			BltTransformed(theImage, NULL,theColor, theDrawMode, theSrcRect,
				       aTransform, true);
			return;
		}

		GLImage::Bltf (theImage, theX, theY, theSrcRect, theColor, theDrawMode);
	}
}

void GLImage::BltTransformed (Image* theImage, const Rect* theClipRect, const Color& theColor,
			      int theDrawMode, const Rect &theSrcRect,
			      const SexyMatrix3 &theTransform, bool linear,
			      float theX, float theY, bool center)
{
	GLTexture *aTexture = EnsureSrcTexture(mInterface, theImage);
	if (!aTexture)
		return;

	SetBlendFunc (theDrawMode, true);
	if (!mTransformStack.empty())
	{
		aTexture->SetTextureFilter(true);
		if (theX != 0.0f || theY != 0.0f)
		{
			SexyTransform2D aTransform;

			if (center)
				aTransform.Translate(-theSrcRect.mWidth / 2.0f,
						     -theSrcRect.mHeight / 2.0f);

			aTransform = theTransform * aTransform;
			aTransform.Translate(theX, theY);
			aTransform = mTransformStack.back() * aTransform;

			aTexture->BltTransformed(aTransform, theSrcRect,
						 theColor, theClipRect);
		}
		else
		{
			SexyTransform2D aTransform = mTransformStack.back()*theTransform;
			aTexture->BltTransformed (aTransform, theSrcRect, theColor,
						  theClipRect, theX, theY, center);
		}
	}
	else
	{
		aTexture->SetTextureFilter(linear);
		aTexture->BltTransformed (theTransform, theSrcRect, theColor,
					  theClipRect, theX, theY, center);
	}
}

void GLImage::BltRotated (Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect,
			  const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
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

	BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform, false);
}

void GLImage::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect,
			 const Color& theColor, int theDrawMode, bool fastStretch)
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
	BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform,
			!fastStretch);
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
	BltTransformed (theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform,
			!fastStretch);
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

void GLImage::BltTrianglesTex(Image *theImage, const TriVertex theVertices[][3], int theNumTriangles,
			      const Rect& theClipRect, const Color &theColor, int theDrawMode,
			      float tx, float ty, bool blend)
{
	if (mInterface->GetScreenImage() != this)
	{
		MemoryImage::BltTrianglesTex (theImage, theVertices, theNumTriangles, theClipRect, theColor,
					      theDrawMode, tx, ty, blend);
		return;
	}

	GLTexture *aTexture = EnsureSrcTexture(mInterface, theImage);
	if (!aTexture)
		return;

	SetBlendFunc (theDrawMode, true);
	aTexture->SetTextureFilter(blend);
	aTexture->BltTriangles (theVertices, theNumTriangles, (uint32)theColor.ToInt(), tx, ty);
}

bool GLImage::Palletize()
{
	return MemoryImage::Palletize();
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
}

void GLImage::Flip(enum FlipFlags flags)
{
	if (mInterface->mScreenImage)
		mInterface->SwapBuffers();
}

GLTexture* GLImage::EnsureTexture()
{
	if (!mTexture)
		mTexture = new GLTexture(mInterface, this);

	mTexture->CheckCreateTextures (this);
	mDrawnTime = Sexy::GetTickCount();

	return mTexture;
}

void GLImage::PushTransform(const SexyMatrix3 &theTransform, bool concatenate)
{
	if (mTransformStack.empty() || !concatenate)
	{
		mTransformStack.push_back(theTransform);
	}
	else
	{
		SexyMatrix3 &aTrans = mTransformStack.back();
		mTransformStack.push_back(theTransform*aTrans);
	}
}

void GLImage::PopTransform()
{
	if (!mTransformStack.empty())
		mTransformStack.pop_back();
}

int GLImage::GetTextureTarget()
{
	return mInterface->GetTextureTarget();
}

void GLImage::RemoveImageData(MemoryImage* theImage)
{
	if (!theImage->mNativeData)
		return;

	GLTexture* aData = (GLTexture*)theImage->mNativeData;
	assert (aData->mTexMemSize == theImage->mTexMemSize);
	theImage->mNativeData = 0;
	delete aData;
}
