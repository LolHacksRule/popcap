#ifndef __GLIMAGE_H__
#define __GLIMAGE_H__

#include "MemoryImage.h"

namespace Sexy
{

class GLInterface;
class GLFont;
class GLImageAutoFallback;
class GLTexture;

class GLImage : public MemoryImage
{
protected:
	friend class			GLFont;
	friend class			GLImageAutoFallback;
	friend class			GLTexture;

	void				DeleteAllNonSurfaceData();

protected:
	GLInterface*			mInterface;
        GLTexture*                      mTexture;

public:
        int                             mDirty;
	bool				mSurfaceSet;
	bool				mNoLock;
	bool				mVideoMemory;
	bool				mFirstPixelTrans;
	bool				mWantDDSurface;
	bool				mDrawToBits;
	int				mLockCount;

private:
	void				Init();

public:
	virtual void			ReInit();
	virtual void			SetVideoMemory(bool wantVideoMemory);
	virtual void			RehupFirstPixelTrans();

	virtual void			BitsChanged();
	virtual void			CommitBits();

	virtual void			NormalFillRect(const Rect& theRect, const Color& theColor);
	virtual void			AdditiveFillRect(const Rect& theRect, const Color& theColor);
	virtual void			NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
	virtual void			AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
	virtual void			NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
	virtual void			AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
	virtual void			NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
	virtual void			AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);


	virtual void			NormalBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
	virtual void			AdditiveBltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);

	virtual void			FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const BYTE* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight);

public:
	GLImage();
	GLImage(GLInterface* theInterface);
	virtual ~GLImage();

        void                            DeleteSurface();

	virtual bool			LockSurface();
	virtual bool			UnlockSurface();

	virtual void			Create(int theWidth, int theHeight);
	virtual void			SetBits(uint32* theBits, int theWidth, int theHeight, bool commitBits = true);
	virtual uint32*			GetBits();
	virtual void			Clear();

	virtual bool			PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool comvex);
	virtual void			FillRect(const Rect& theRect, const Color& theColor, int theDrawMode);
	virtual void			DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
	virtual void			DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
	virtual void			Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
	virtual void			BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode);
	virtual void			BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY);
	virtual void			StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);
	virtual void			BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend);
	virtual void			BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend);

	virtual void			BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
	virtual void			StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);

	virtual bool			Palletize();
	virtual void			PurgeBits();
	virtual void			DeleteNativeData();
	virtual void			DeleteExtraBuffers();

        virtual void                    Flip(enum FlipFlags flags = FLIP_NONE);

        void                            EnsureTexture();
};

}

#endif //__DDIMAGE_H__
