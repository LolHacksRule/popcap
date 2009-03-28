#ifndef __GLINTERFACE_H__
#define __GLINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "NativeDisplay.h"
#include "Rect.h"
#include "Ratio.h"

#ifdef SEXY_OPENGLES
#include <GLES/gl.h>
#elif defined(SEXY_AGL_DRIVER)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace Sexy
{

class SexyAppBase;
class GLImage;
class Image;
class MemoryImage;

typedef std::set<GLImage*> GLImageSet;

class GLInterface : public NativeDisplay
{
	friend class GLImage;
public:
	SexyAppBase*				mApp;

	float                                   mOverScan;
	int					mFullscreenBits;
	DWORD					mRefreshRate;
	DWORD					mMillisecondsPerFrame;
	int					mScanLineFailCount;

	bool					mInitialized;
	bool					mIsWindowed;
	bool					mVideoOnlyDraw;
	ulong					mInitCount;

	GLImage*				mScreenImage;

	bool					mCursorEnabled;
	int					mCursorX;
	int					mCursorY;
	int					mCursorDrawnX;
	int					mCursorDrawnY;
	int					mCursorOldX;
	int					mCursorOldY;
	int					mCursorHotX;
	int					mCursorHotY;

	GLImage*				mCursorImage;
	bool                                    mCursorDirty;
	bool					mCursorDrawn;

	int					mWindowWidth;
	int					mWindowHeight;

	GLint					mMinTextureWidth;
	GLint					mMinTextureHeight;
	GLint					mMaxTextureWidth;
	GLint					mMaxTextureHeight;
	GLint					mGoodTextureSize[8096];
	GLboolean				mTextureNPOT;
	const char*				mGLExtensions;

public:
	GLint					mGLMajor;
	GLint					mGLMinor;
	GLboolean				mTexBGRA;

public:
	GLInterface(SexyAppBase* theApp);
	virtual ~GLInterface();

	virtual bool				Is3DAccelerated();
	virtual bool				Is3DAccelerationSupported();
	virtual bool				Is3DAccelerationRecommended();

	virtual Image*				GetScreenImage();
	virtual int				Init();
	virtual void				Cleanup();

	virtual void				SwapBuffers();
	virtual void				InitGL();

	virtual bool				Redraw(Rect* theClipRect);

	void					GenGoodTexSize();
	void					CalulateBestTexDimensions (int & theWidth, int & theHeight,
									   bool isEdge, bool usePOT);

	virtual bool				EnableCursor(bool enable);
	virtual bool				SetCursorImage(Image* theImage, int theHotX = 0, int theHotY = 0);
	virtual void				SetCursorPos(int theCursorX, int theCursorY);
	virtual bool				DrawCursor(Graphics* g);
	virtual bool				UpdateCursor(int theCursorX, int theCursorY);
};

}

#endif //__GLINTERFACE_H__

