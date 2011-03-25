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

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>

#define  LOG_TAG    "AndroidDisplay"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define printf LOGI
#endif

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

	AGViewAddEventListener(HandleEvents, this);

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
       if (mApp->mUpdateAppDepth > 1)
       {
               AGViewSwapBuffers();
               AGViewUpdate();
       }
}

void AndroidDisplay::HandleKeyEvent(const AGEvent*event)
{
	if (!mEvents.empty())
		mEvents.clear();
}

void AndroidDisplay::HandlePointerEvent(const AGEvent*event)
{
	SexyAppBase *app = mApp;

	std::list<Event>& events = mEvents;

	int x = event->u.pointer.x * mWidth / mWindowWidth;
	int y = event->u.pointer.y * mHeight / mWindowHeight;

	Event evt;
	switch (event->type)
	{
	case AG_POINTER_DOWN_EVENT:
		evt.u.touch.state = TOUCH_DOWN;
		break;
	case AG_POINTER_MOVE_EVENT:
		evt.u.touch.state = TOUCH_MOVE;
		break;
	case AG_POINTER_UP_EVENT:
		evt.u.touch.state = TOUCH_UP;
		break;
	case AG_POINTER_CANCEL_EVENT:
		evt.u.touch.state = TOUCH_CANCEL;
		break;
	}

	printf("MotionEvent: type = %d id = %d x = %f y = %f\n",
	       event->type, event->u.pointer.id,
	       event->u.pointer.x, event->u.pointer.y);

	evt.type = EVENT_TOUCH;
	evt.flags = 0;
	evt.id = 0;
	evt.subid = 0;

	evt.u.touch.id = event->u.pointer.id;
	evt.u.touch.x = x;
	evt.u.touch.y = y;
	evt.u.touch.pressure = event->u.pointer.pressure * 100;
	if (evt.u.touch.pressure > 100)
		evt.u.touch.pressure = 100;
	if (event->type == AG_POINTER_DOWN_EVENT ||
	    event->type == AG_POINTER_UP_EVENT)
	{
		// ignore some events...
		if (!(event->flags & AG_EVENT_STATE_CHANGED))
		    return;
	}
	else
	{
		if (event->flags & AG_EVENT_FOLLOW)
			evt.flags |= EVENT_FLAGS_INCOMPLETE;
	}

	events.push_back(evt);
	if (!(evt.flags & EVENT_FLAGS_INCOMPLETE))
	{
		app->mInputManager->PushEvents(events);
		events.clear();
	}
}

void AndroidDisplay::InjectKeyEvent(int keycode, int keychar)
{
	Event evt;

	evt.type = EVENT_KEY_DOWN;
	evt.flags = EVENT_FLAGS_KEY_CODE;
	evt.id = 0;
	evt.subid = 0;
	evt.u.key.keyCode = keycode;

	if (keychar > 0 && keychar <= 127)
	{
		evt.flags |= EVENT_FLAGS_KEY_CHAR;
		evt.u.key.keyChar = keychar;
	}
	mApp->mInputManager->PushEvent(evt);

	evt.type = EVENT_KEY_UP;
	evt.flags &= ~EVENT_FLAGS_KEY_CHAR;
	mApp->mInputManager->PushEvent(evt);
}

void AndroidDisplay::HandleInputEvents(const AGEvent *event)
{
	const char *s = AGViewGetTextInput();

	InjectKeyEvent(KEYCODE_CLEAR, 0);
	for (; *s; s++)
	{
		int c = (*s) & 0xff;
		int kc = c;

		if (isalpha(c))
			kc = toupper(c);
		InjectKeyEvent(kc, c);
	}
	InjectKeyEvent(KEYCODE_RETURN, 0);
}

void AndroidDisplay::HandleEvents(const AGEvent*event,
				  void*         data)
{
	AndroidDisplay* dpy = (AndroidDisplay*)data;

	if (!dpy)
		return;

	switch(event->type)
	{
	case AG_POINTER_DOWN_EVENT:
	case AG_POINTER_MOVE_EVENT:
	case AG_POINTER_UP_EVENT:
	case AG_POINTER_CANCEL_EVENT:
		dpy->HandlePointerEvent(event);
		break;
	case AG_KEY_UP_EVENT:
	case AG_KEY_DOWN_EVENT:
		dpy->HandleKeyEvent(event);
		break;
	default:
		break;
	}
}

bool AndroidDisplay::ShowKeyboard(Widget* theWidget,
				 KeyboardMode mode,
				 const std::string &title,
				 const std::string &hint,
				 const std::string &initial)
{
	AGViewShowKeyboard((AGKeyboardMode)mode, title.c_str(),
			   hint.c_str(), initial.c_str());
	return true;
}

void AndroidDisplay::HideKeyboard()
{
	AGViewHideKeyboard();
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
