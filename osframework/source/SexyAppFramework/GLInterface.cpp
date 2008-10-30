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
	return 0;
}

void GLInterface::Cleanup()
{
	mInitialized = false;
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

	glEnable (GL_BLEND);
	glLineWidth (1.5);
	glDisable (GL_LIGHTING);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_NORMALIZE);
	glDisable (GL_CULL_FACE);
	glShadeModel (GL_FLAT);
	glReadBuffer (GL_BACK);
	glPixelStorei (GL_PACK_ROW_LENGTH, 0);
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);

	glClearColor (0.0, 0.0, 0.0, 0.0);

	glMatrixMode (GL_PROJECTION );
	glLoadIdentity ();
	glOrtho (0, mWidth, mHeight, 0, -1.0, 1.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glClear (GL_COLOR_BUFFER_BIT);
	SwapBuffers ();

	glClear (GL_COLOR_BUFFER_BIT);
	SwapBuffers ();
}

