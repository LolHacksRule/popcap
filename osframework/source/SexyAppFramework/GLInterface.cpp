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
	Cleanup();

	AutoCrit anAutoCrit(mCritSect);
	mInitialized = false;

	mInitCount++;
	mInitialized = true;

	return 0;
}

void GLInterface::Cleanup()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;
}

bool GLInterface::Redraw(Rect* theClipRect)
{
	AutoCrit anAutoCrit(mCritSect);

	if (!mInitialized)
		return false;
	return true;
}
