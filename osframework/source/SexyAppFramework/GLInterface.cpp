#include "GLInterface.h"
#include "GLImage.h"
#include "SexyAppBase.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"

#include <cstdio>

using namespace Sexy;

#ifdef SEXY_OPENGLES
#define ftofix(f) (GLfixed)(f * 65536.0f)
#endif

#if defined(DEBUG) || defined(SEXY_GL_DEBUG) 
#define SEXY_GL_IS_DEBUG() true
#else
#define SEXY_GL_IS_DEBUG() getenv("SEXY_GL_DEBUG")
#endif

GLInterface::GLInterface(SexyAppBase* theApp)
{
	mApp = theApp;
	mScreenImage = NULL;
	mInitialized = false;
	mVideoOnlyDraw = false;
	mScanLineFailCount = 0;

	mOverScan = 1.0;
	mInitCount = 0;
	mRefreshRate = 60;
	mMillisecondsPerFrame = 1000 / mRefreshRate;

	mMinTextureWidth = 1;
	mMinTextureHeight = 1;
	mMaxTextureWidth = 4096;
	mMaxTextureHeight = 4096;
	mTextureNPOT = GL_FALSE;

	mGLExtensions = NULL;
	mGLMajor = 1;
	mGLMinor = 1;
	mTexBGRA = GL_FALSE;
}

GLInterface::~GLInterface()
{
	Cleanup();
}

bool GLInterface::Is3DAccelerated()
{
    return true;
}

bool GLInterface::Is3DAccelerationSupported()
{
    return true;
}

bool GLInterface::Is3DAccelerationRecommended()
{
    return true;
}

Image* GLInterface::GetScreenImage()
{
	return mScreenImage;
}

int GLInterface::Init(void)
{
	GLInterface::Cleanup();

	mMinTextureWidth = 1;
	mMinTextureHeight = 1;
	mMaxTextureWidth = 4096;
	mMaxTextureHeight = 4096;
	mTextureNPOT = GL_FALSE;

	return 0;
}

void GLInterface::Cleanup()
{
	FlushWork();
	mInitialized = false;

	mGLExtensions = NULL;
	mGLMajor = 1;
	mGLMinor = 1;
	mTexBGRA = GL_FALSE;

	mCursorHotX = 0;
	mCursorHotY = 0;
	mCursorX = 0;
	mCursorY = 0;
	mCursorOldX = 0;
	mCursorOldY = 0;
	mCursorDrawnX = 0;
	mCursorDrawnY = 0;
	mCursorEnabled = false;
	mCursorDrawn = false;
	mCursorImage = 0;
	mCursorDirty = true;
}

bool GLInterface::Redraw(Rect* theClipRect)
{
    return false;
}

void GLInterface::SwapBuffers()
{
	FlushWork();
}

void GLInterface::InitGL()
{
	float x, y, w, h;

	x = mWindowWidth * (1.0f - mOverScan) / 2.0f;
	y = mWindowHeight * (1.0f - mOverScan) / 2.0f;
	w = mWindowWidth * mOverScan;
	h = mWindowHeight * mOverScan;
	glViewport (x, y, w, h);

	const char* version = (const char*)glGetString (GL_VERSION);
	const char* str = version;
	while (!(*str >= '0' && *str <= '9') && *str)
		str++;
	if (*str)
	{
		mGLMajor = atoi (str);
		str = strchr (str, '.') + 1;
		mGLMinor = atoi (str);
	}
	if (SEXY_GL_IS_DEBUG ())
		printf ("GL version: %s(%d.%d)\n", version, mGLMajor, mGLMinor);

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &mMaxTextureWidth);
	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &mMaxTextureHeight);

	if (SEXY_GL_IS_DEBUG ())
		printf ("Maximium texture size: %d\n", mMaxTextureHeight);

	mGLExtensions = (const char*)glGetString (GL_EXTENSIONS);

	if (SEXY_GL_IS_DEBUG ())
		printf ("GL extensions: %s\n", mGLExtensions);

	if (mGLExtensions) {
		if ((strstr (mGLExtensions, "GL_ARB_texture_non_power_of_two") ||
		     strstr (mGLExtensions, "GL_EXT_texture_non_power_of_two") ||
		     strstr (mGLExtensions, "GL_ARB_texture_rectangle") ||
		     strstr (mGLExtensions, "GL_EXT_texture_rectangle")) &&
		    getenv("SEXY_GL_RECTANGLE_TEXTURE"))
			mTextureNPOT = GL_TRUE;
		if (mGLMajor > 2 || (mGLMajor == 1 && mGLMinor >= 2) ||
		    strstr (mGLExtensions, "GL_IMG_texture_format_BGRA888") ||
		    strstr (mGLExtensions, "GL_EXT_bgra"))
			mTexBGRA = GL_TRUE;
		else
			mTexBGRA = GL_FALSE;
	}


	GenGoodTexSize ();
	glEnable (GL_BLEND);
	glLineWidth (1.5);
	glDisable (GL_LIGHTING);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_NORMALIZE);
	glDisable (GL_CULL_FACE);
	glShadeModel (GL_FLAT);
#ifndef SEXY_OPENGLES
	glReadBuffer (GL_BACK);

	glPixelStorei (GL_PACK_ROW_LENGTH, 0);
#endif
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

#ifdef GL_TEXTURE_GEN_S
	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);
#endif
	glClearColor (0.0, 0.0, 0.0, 0.0);

	glMatrixMode (GL_PROJECTION) ;
	glLoadIdentity ();

#ifndef SEXY_OPENGLES
	glOrtho (0, mWidth, mHeight, 0, -1.0, 1.0);
#else
	glOrthox (ftofix (0.0), ftofix (mWidth),
		  ftofix (mHeight), ftofix (0),
		  ftofix (-1.0), ftofix (1.0));
#endif
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glClear (GL_COLOR_BUFFER_BIT);
	SwapBuffers ();

	glClear (GL_COLOR_BUFFER_BIT);
	SwapBuffers ();
}

void GLInterface::GenGoodTexSize()
{
	DBG_ASSERT(mMaxTextureWidth <= 8096);

	int i;
	int pot = 1;

	for (i = 0; i < mMaxTextureWidth; i++)
	{
		if (i > pot)
			pot <<= 1;

		int value = pot;
		if ((value - i ) > 64)
		{
			value >>= 1;
			while (1)
			{
				int leftover = i % value;
				if (leftover < 64 || IsPOT (leftover))
					break;

				value >>= 1;
			}
		}
		mGoodTextureSize[i] = value;
	}
}

void GLInterface::CalulateBestTexDimensions (int & theWidth, int & theHeight,
					     bool isEdge, bool usePOT)
{
	int aWidth = theWidth;
	int aHeight = theHeight;

	if (usePOT)
	{
		if (isEdge)
		{
			aWidth = aWidth >= mMaxTextureWidth ? mMaxTextureWidth : RoundToPOT (aWidth);
			aHeight = aHeight >= mMaxTextureHeight ? mMaxTextureHeight : RoundToPOT (aHeight);
		}
		else
		{
			aWidth = aWidth >= mMaxTextureWidth ? mMaxTextureWidth : mGoodTextureSize[aWidth];
			aHeight = aHeight >= mMaxTextureHeight ? mMaxTextureHeight : mGoodTextureSize[aHeight];
		}
	}

	if (aWidth < mMinTextureWidth)
		aWidth = mMinTextureWidth;

	if (aHeight < mMinTextureHeight)
		aHeight = mMinTextureHeight;

#if 0
	if (aWidth > aHeight)
	{
		while (aWidth > mMaxTextureAspectRatio * aHeight)
			aHeight <<= 1;
	}
	else if (aHeight > aWidth)
	{
		while (aHeight > mMaxTextureAspectRatio * aWidth)
			aWidth <<= 1;
	}
#endif
	theWidth = aWidth;
	theHeight = aHeight;
}

bool GLInterface::EnableCursor(bool enable)
{
	mCursorEnabled = enable;
	mCursorDirty = true;
	return true;
}

bool GLInterface::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	MemoryImage * aMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	mCursorDirty = true;
	mCursorImage = aMemoryImage;
	mCursorHotX = theHotX;
	mCursorHotY = theHotY;
	return true;
}

void GLInterface::SetCursorPos(int theCursorX, int theCursorY)
{
	mCursorOldX = mCursorX;
	mCursorOldY = mCursorY;
	mCursorX = theCursorX;
	mCursorY = theCursorY;
}

bool GLInterface::UpdateCursor(int theCursorX, int theCursorY)
{
	SetCursorPos (theCursorX, theCursorY);
	if (mCursorDrawnX != mCursorX || mCursorDrawnY != mCursorY)
	    mCursorDirty = true;
	return mCursorDirty;
}

bool GLInterface::DrawCursor(Graphics* g)
{
	mCursorDirty = false;

	if (mCursorImage && mCursorEnabled)
		g->DrawImage (mCursorImage,
			      mCursorX - mCursorHotX,
			      mCursorY - mCursorHotY);

	mCursorDrawnX = mCursorX;
	mCursorDrawnY = mCursorY;
	return true;
}

int GLInterface::GetTextureTarget()
{
	if (mTextureNPOT)
	{
		if (mGLMajor >= 2)
			return GL_TEXTURE_2D;
		return GL_TEXTURE_RECTANGLE_ARB;
	}

	return GL_TEXTURE_2D;
}

void GLInterface::RemoveImageData(MemoryImage* theImage)
{
	if (!theImage->mNativeData)
		return;

	GLTexture* aData = (GLTexture*)theImage->mNativeData;
	theImage->mNativeData = 0;
	delete aData;
}

namespace Sexy {
class DelayedDeleteTextureWork: public DelayedWork
{
public:
	DelayedDeleteTextureWork(GLuint tex) : mTex(tex) {}

public:
	virtual void Work()
	{
		glDeleteTextures(1, &mTex);
	}

private:
	GLuint mTex;
};
}

void GLInterface::DelayedDeleteTexture(GLuint name)
{
	if (mMainThread != Thread::Self())
		PushWork(new DelayedDeleteTextureWork(name));
	else
		glDeleteTextures(1, &name);
}
