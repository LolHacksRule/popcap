#include "SexyAppBase.h"
#include "AndroidDisplay.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"

#include <cstdio>
#include <cstdlib>

using namespace Sexy;

AndroidDisplay::AndroidDisplay (SexyAppBase* theApp)
	: GLDisplay (theApp)
{
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
}

AndroidDisplay::~AndroidDisplay ()
{
	Cleanup();
}

static long GetEnvInt(const char*name,
		     int default_value = 0)
{
    const char *s = getenv(name);

    if (!s)
	return default_value;

    return atol(s);
}

int AndroidDisplay::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLDisplay::Init();

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mOverScan = 0.9;

	if (mApp->mIsWindowed)
		mOverScan = 1.0f;
	else
		mOverScan = 0.9f;

	int width = GetEnvInt("ANDROID_GLVIEW_WIDTH", 480);
	int height = GetEnvInt("ANDROID_GLVIEW_HEIGHT", 320);

	mWindowWidth = width;
	mWindowHeight = height;
	if (mApp->mIsWindowed)
	{
		mWidth = width;
		mHeight = height;
		mDisplayWidth = width;
		mDisplayHeight = height;
	}
	else
	{
		mWidth = mApp->mWidth;
		mHeight = mApp->mHeight;
		mDisplayWidth = mApp->mWidth;
		mDisplayHeight = mApp->mHeight;
	}
	mPresentationRect = Rect(0, 0, mDisplayWidth, mDisplayHeight);

	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	InitGL ();

	mInitCount++;
	mInitialized = true;

	return 0;
}

bool AndroidDisplay::CanReinit (void)
{
	return mInitialized;
}

bool AndroidDisplay::Reinit (void)
{
	if (mApp->mIsWindowed)
		mOverScan = 1.0f;
	else
		mOverScan = 0.9f;

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mIs3D = mApp->mIs3D;
	if (mApp->mIsWindowed)
	{
		mWidth = mWindowWidth;
		mHeight = mWindowHeight;
		mDisplayWidth = mWindowWidth;
		mDisplayHeight = mWindowHeight;
	}
	else
	{
		mWidth = mApp->mWidth;
		mHeight = mApp->mHeight;
		mDisplayWidth = mApp->mWidth;
		mDisplayHeight = mApp->mHeight;
	}
	mPresentationRect = Rect(0, 0, mDisplayWidth, mDisplayHeight);

	delete mScreenImage;
	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	return GLDisplay::Reinit();
}

void AndroidDisplay::Cleanup ()
{
	AutoCrit anAutoCrit(mCritSect);

	GLDisplay::Cleanup ();

	mInitialized = false;

	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = NULL;
}

void AndroidDisplay::RemapMouse(int& theX, int& theY)
{
}

Image* AndroidDisplay::CreateImage(SexyAppBase * theApp,
				     int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

bool AndroidDisplay::HasEvent()
{
	return false;
}

bool AndroidDisplay::GetEvent(struct Event &event)
{
	return false;
}

void AndroidDisplay::SwapBuffers()
{
}

class AndroidVideoDriver: public VideoDriver {
public:
	AndroidVideoDriver ()
	 : VideoDriver("Android", 10)
	{
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new AndroidDisplay (theApp);
	}
};

static AndroidVideoDriver aAndroidVideoDriver;
VideoDriverRegistor aAndroidVideoDriverRegistor (&aAndroidVideoDriver);
VideoDriver* GetAndroidVideoDriver()
{
	return &aAndroidVideoDriver;
}
