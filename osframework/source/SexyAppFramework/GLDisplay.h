#ifndef __GLINTERFACE_H__
#define __GLINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "NativeDisplay.h"
#include "Rect.h"
#include "Ratio.h"

#ifdef SEXY_OPENGLES
#ifdef __APPLE__
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <GLES/gl.h>
#endif
#elif defined(SEXY_AGL_DRIVER)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef GL_ARB_texture_rectangle
#define GL_TEXTURE_RECTANGLE_ARB          0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ARB  0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_ARB    0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#endif

namespace Sexy
{

class SexyAppBase;
class GLImage;
class Image;
class MemoryImage;

typedef std::set<GLImage*> GLImageSet;

class GLDisplay : public NativeDisplay
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
	bool                                    mIs3D;
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

	MemoryImage*				mCursorImage;
	bool                                    mCursorDirty;
	bool					mCursorDrawn;

	int					mWindowWidth;
	int					mWindowHeight;

	GLint					mMinTextureWidth;
	GLint					mMinTextureHeight;
	GLint					mMaxTextureWidth;
	GLint					mMaxTextureHeight;
	GLint					mGoodTextureSize[10240];
	GLboolean				mTextureNPOT;
	const char*				mGLExtensions;

public:
	GLint					mGLMajor;
	GLint					mGLMinor;
	GLboolean				mTexBGRA;
	GLboolean                               mTexPalette8RGBA;

public:
	GLDisplay(SexyAppBase* theApp);
	virtual ~GLDisplay();

	virtual bool				Is3DAccelerated();
	virtual bool				Is3DAccelerationSupported();
	virtual bool				Is3DAccelerationRecommended();

	virtual Image*				GetScreenImage();
	virtual int				Init();
	virtual bool                            Reinit();
	virtual void				Cleanup();

	virtual bool				CreateImageData(MemoryImage* theImage);
	virtual void				RemoveImageData(MemoryImage* theImage);

	virtual void				SwapBuffers();
	virtual void				InitGL();
	virtual void                            Reshape();

	virtual bool				Redraw(Rect* theClipRect);

	void					GenGoodTexSize();
	void					CalulateBestTexDimensions (int & theWidth, int & theHeight,
									   bool isEdge, bool usePOT);

	virtual bool				EnableCursor(bool enable);
	virtual bool				SetCursorImage(Image* theImage, int theHotX = 0, int theHotY = 0);
	virtual void				SetCursorPos(int theCursorX, int theCursorY);
	virtual bool				DrawCursor(Graphics* g);
	virtual bool				UpdateCursor(int theCursorX, int theCursorY);

 public:
	void                                    DelayedDeleteTexture(GLuint name);
	int                                     GetTextureTarget(void);
};

}

#endif //__GLINTERFACE_H__

