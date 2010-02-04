#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Common.h"
#include "Color.h"
#include "Rect.h"
#include "Point.h"

namespace Sexy
{

struct Span
{
	int						mY;
	int						mX;
	int						mWidth;
};

enum AnimType
{
	AnimType_None,
	AnimType_Once,
	AnimType_PingPong,
	AnimType_Loop
};

struct SEXY_EXPORT AnimInfo
{
	AnimType				mAnimType;
	int						mFrameDelay; // 1/100s
	int						mNumCels;
	std::vector<int>		mPerFrameDelay;
	std::vector<int>		mFrameMap;
	int						mTotalAnimTime;

	AnimInfo();
	void SetPerFrameDelay(int theFrame, int theTime);
	void Compute(int theNumCels, int theBeginFrameTime = 0, int theEndFrameTime = 0);

	int GetPerFrameCel(int theTime);
	int GetCel(int theTime);
};

class Graphics;
class SexyMatrix3;
class SysFont;
class TriVertex;

enum FlipFlags {
	FLIP_NONE      = 0,
	FLIP_WAIT      = 1 << 0,
	FLIP_ONSYNC    = 1 << 1,
	FLIP_BLIT      = 1 << 2,
	FLIP_WAIT_SYNC = FLIP_WAIT | FLIP_ONSYNC
};

enum ImageFlags {
	IMAGE_FLAGS_NONE          = 0,
	IMAGE_FLAGS_DOUBLE_BUFFER = 1 << 0,
	IMAGE_FLAGS_FLIP_AS_COPY  = 1 << 1,
	IMAGE_FLAGS_MINI_SUBDIV   = 1 << 2,
	IMAGE_FLAGS_A4R4G4B4      = 1 << 3,
	IMAGE_FLAGS_A8R8G8B8      = 1 << 4
};

enum WrapMode {
	WRAP_CLAMP,
	WRAP_REPEAT
};

class SEXY_EXPORT Image
{
	friend class			Sexy::SysFont;

public:
	bool					mDrawn;
	DWORD                                   mDrawnTime;
	DWORD                                   mTexMemSize;
	std::string				mFilePath;
	int					mWidth;
	int					mHeight;

	// for image strips
	int					mNumRows;
	int					mNumCols;

	// for animations
	AnimInfo				*mAnimInfo;

        int                                      mFlags;
	int                                      mWrapModeU;
	int                                      mWrapModeV;

public:
	Image();
	Image(const Image& theImage);
	virtual ~Image();

	int						GetWidth();
	int						GetHeight();
	int						GetCelWidth();		// returns the width of just 1 cel in a strip of images
	int						GetCelHeight();	// like above but for vertical strips
	int						GetAnimCel(int theTime); // use animinfo to return appropriate cel to draw at the time
	Rect					GetAnimCelRect(int theTime);
	Rect					GetCelRect(int theCel);				// Gets the rectangle for the given cel at the specified row/col 
	Rect					GetCelRect(int theCol, int theRow);	// Same as above, but for an image with both multiple rows and cols
	void					CopyAttributes(Image *from);
	Graphics*				GetGraphics();

	virtual void			SetBits(uint32* theBits, int theWidth, int theHeight, bool commitBits = true);
        virtual uint32*                 GetBits();

	virtual bool			PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool convex);

	virtual void			FillRect(const Rect& theRect, const Color& theColor, int theDrawMode);	
	virtual void			DrawRect(const Rect& theRect, const Color& theColor, int theDrawMode);
	virtual void			ClearRect(const Rect& theRect);
	virtual void			DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
	virtual void			DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
	virtual void			FillScanLines(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode);
	virtual void			FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const BYTE* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight);
	virtual void			Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
	virtual void			BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode);
	virtual void			BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY);
	virtual void			StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);
	virtual void			BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend);
	virtual void			BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend);

	virtual void			BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode);
	virtual void			StretchBltMirror(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch);

	virtual bool			Palletize();

        virtual void                    Flip(enum FlipFlags flags = FLIP_NONE);
	virtual void                    SetWrapMode(WrapMode u, WrapMode v);
	virtual void			PushTransform(const SexyMatrix3 &theTransform, bool concatenate = true);
	virtual void			PopTransform();
};

}

#endif //__IMAGE_H__
