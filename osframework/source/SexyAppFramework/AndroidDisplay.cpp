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
#include "InputManager.h"

#include "GameView.h"

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
	mOverScan = 1.0f;

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

	awAddViewEventListener(HandleEvents, this);

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

void AndroidDisplay::HandleKeyEvent(const awEvent*event)
{
	if (!mEvents.empty())
		mEvents.clear();
}

void AndroidDisplay::HandlePointerEvent(const awEvent*event)
{
	SexyAppBase *app = mApp;

	std::list<Event>& events = mEvents;

	int x = event->u.pointer.x * mWidth / mWindowWidth;
	int y = event->u.pointer.y * mHeight / mWindowHeight;

	Event evt;
	switch(event->type)
	{
	case AW_POINTER_DOWN_EVENT:
		evt.u.touch.state = TOUCH_DOWN;
		break;
	case AW_POINTER_MOVE_EVENT:
		evt.u.touch.state = TOUCH_MOVE;
		break;
	case AW_POINTER_UP_EVENT:
		evt.u.touch.state = TOUCH_UP;
		break;
	case AW_POINTER_CANCEL_EVENT:
		evt.u.touch.state = TOUCH_CANCEL;
		break;
	}

	evt.type = EVENT_TOUCH;
	evt.flags = 0;
	evt.id = 0;
	evt.subid = 0;
	if (event->flags & 1)
		evt.flags |= EVENT_FLAGS_INCOMPLETE;
	evt.u.touch.id = event->u.pointer.id;
	evt.u.touch.x = x;
	evt.u.touch.y = y;
	evt.u.touch.pressure = event->u.pointer.pressure * 100;
	if (evt.u.touch.pressure > 100)
		evt.u.touch.pressure = 100;

	events.push_back(evt);
	if (!(evt.flags & EVENT_FLAGS_INCOMPLETE))
	{
		app->mInputManager->PushEvents(events);
		events.clear();
	}
}

void AndroidDisplay::HandleEvents(const awEvent*event,
				  void*         data)
{
	AndroidDisplay* dpy = (AndroidDisplay*)data;

	if (!dpy)
		return;

	switch(event->type)
	{
	case AW_POINTER_DOWN_EVENT:
	case AW_POINTER_MOVE_EVENT:
	case AW_POINTER_UP_EVENT:
	case AW_POINTER_CANCEL_EVENT:
		dpy->HandlePointerEvent(event);
		break;
	case AW_KEY_UP_EVENT:
	case AW_KEY_DOWN_EVENT:
		dpy->HandleKeyEvent(event);
		break;
	default:
		break;
	}
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
