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

GLInterface::GLInterface(SexyAppBase* theApp)
{
	mApp = theApp;
	mScreenImage = NULL;
	mInitialized = false;
	mVideoOnlyDraw = false;
	mScanLineFailCount = 0;

	mInitCount = 0;
	mRefreshRate = 60;
	mMillisecondsPerFrame = 1000 / mRefreshRate;

        mMinTextureWidth = 1;
        mMinTextureHeight = 1;
        mMaxTextureWidth = 4096;
        mMaxTextureHeight = 4096;
	mTextureNPOT = GL_FALSE;

	mGLExtensions = NULL;
}

GLInterface::~GLInterface()
{
	Cleanup();
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
	mInitialized = false;

	mGLExtensions = NULL;
}

bool GLInterface::Redraw(Rect* theClipRect)
{
	AutoCrit anAutoCrit(mCritSect);

	if (!mInitialized)
		return false;

	SwapBuffers ();
	return true;
}

void GLInterface::SwapBuffers()
{
}

void GLInterface::InitGL()
{
	glViewport (0, 0, mWidth, mHeight);

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &mMaxTextureWidth);
	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &mMaxTextureHeight);

	printf ("Maximium texture size: %d\n", mMaxTextureHeight);

	mGLExtensions = (const char*)glGetString (GL_EXTENSIONS);
	if (mGLExtensions) {
		if (strstr (mGLExtensions, "GL_ARB_texture_non_power_of_two") ||
		    strstr (mGLExtensions, "GL_EXT_texture_non_power_of_two") ||
		    strstr (mGLExtensions, "GL_ARB_texture_rectangle") ||
		    strstr (mGLExtensions, "GL_EXT_texture_rectangle"))
			mTextureNPOT = GL_TRUE;
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
