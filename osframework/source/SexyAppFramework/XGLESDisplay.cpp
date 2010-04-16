#include "SexyAppBase.h"
#include "XGLESDisplay.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"
#include "SexyMatrix.h"

//#include <GL/glu.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include <cstdio>

using namespace Sexy;

XGLESDisplay::XGLESDisplay (SexyAppBase* theApp)
	: GLDisplay (theApp)
{
	mNativeDpy = 0;
	mNativeWindow = None;
	mDpy = NULL;
	//mWindow = NULL;
	mContext = EGL_NO_CONTEXT;
	mSurface = EGL_NO_SURFACE;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
}

XGLESDisplay::~XGLESDisplay ()
{
	Cleanup();
}

static Bool WaitForMapNotify(Display *d, XEvent *e, char* arg)
{
	if ((e->type == MapNotify) && (e->xmap.window == (Window)arg))
		return GL_TRUE;
	return GL_FALSE;
}

Bool XGLESDisplay::WaitForSubstructureNotify(Display *d, XEvent *e, char* arg)
{
	XGLESDisplay * interface = (XGLESDisplay*)arg;

	if ((e->type == ConfigureNotify) &&
	    (e->xmap.window == interface->mNativeWindow)) {
		interface->mWindowWidth = e->xconfigure.width;
		interface->mWindowHeight = e->xconfigure.height;
		return GL_TRUE;
	}

	return GL_FALSE;
}

int XGLESDisplay::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLDisplay::Init();

	mNativeDpy = XOpenDisplay (NULL);
	if (!mNativeDpy)
		goto fail;

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mScreen = DefaultScreen (mNativeDpy);
	mDesktopWidth = DisplayWidth (mNativeDpy, mScreen);
	mDesktopHeight = DisplayHeight (mNativeDpy, mScreen);
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mWindowWidth = mWidth;
	mWindowHeight = mHeight;
	mIs3D = mApp->mIs3D;

	Colormap colorMap;
	int screen, depth;

	screen = DefaultScreen(mNativeDpy);
	depth = DefaultDepth(mNativeDpy, screen);

	XVisualInfo aVisualInfo;
	XVisualInfo *visualInfo;

	visualInfo = &aVisualInfo;

	XMatchVisualInfo(mNativeDpy, screen, depth, TrueColor, visualInfo);

	/* create an X colormap since probably not using default visual */
	colorMap = XCreateColormap (mNativeDpy, RootWindow (mNativeDpy, visualInfo->screen),
				    visualInfo->visual, AllocNone);

	XSetWindowAttributes swa;
	swa.colormap = colorMap;
	swa.border_pixel = 0;
	swa.event_mask = KeyPressMask | KeyReleaseMask |
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
		EnterWindowMask | LeaveWindowMask | ExposureMask | StructureNotifyMask;
	mNativeWindow = XCreateWindow (mNativeDpy, RootWindow (mNativeDpy, visualInfo->screen), 0, 0,
				       mWindowWidth, mWindowHeight, 0, visualInfo->depth,
				       InputOutput, visualInfo->visual,
				       CWBorderPixel | CWColormap | CWEventMask, &swa);

	XMapWindow (mNativeDpy, mNativeWindow);
	XSync (mNativeDpy, False);


	XEvent event;
	XIfEvent (mNativeDpy, &event, WaitForMapNotify, (char*)mNativeWindow);

	XSync (mNativeDpy, FALSE);

	XIfEvent (mNativeDpy, &event, WaitForSubstructureNotify, (char*)this);

	while (XPending (mNativeDpy))
		XNextEvent (mNativeDpy, &event);

	Pixmap pixmap;
	Cursor cursor;
	XColor black, dummy;
	static char cursor_data[] = {0, 0, 0, 0, 0, 0, 0, 0};

	XAllocNamedColor (mNativeDpy, colorMap, "black", &black, &dummy);
	pixmap = XCreateBitmapFromData (mNativeDpy, mNativeWindow, cursor_data, 8, 8);
	cursor = XCreatePixmapCursor (mNativeDpy, pixmap, pixmap, &black, &black, 0, 0);

	XDefineCursor (mNativeDpy, mNativeWindow, cursor);
	XFreeCursor (mNativeDpy, cursor);
	XFreePixmap (mNativeDpy, pixmap);

	mWMDeleteMessage = XInternAtom (mNativeDpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols (mNativeDpy, mNativeWindow, &mWMDeleteMessage, 1);

	Atom wmState, fullScreen;

	wmState = XInternAtom (mNativeDpy, "_NET_WM_STATE", False);
	fullScreen = XInternAtom (mNativeDpy, "_NET_WM_STATE_FULLSCREEN", False);

	if (!mApp->mIsWindowed)
	{
		memset(&event, 0, sizeof(event));
		event.type = ClientMessage;
		event.xclient.window = mNativeWindow;
		event.xclient.message_type = wmState;
		event.xclient.format = 32;
		event.xclient.data.l[0] = 1;
		event.xclient.data.l[1] = fullScreen;
		event.xclient.data.l[2] = 0;

		XSendEvent (mNativeDpy, DefaultRootWindow (mNativeDpy), False,
			    SubstructureNotifyMask, &event);
		XIfEvent (mNativeDpy,	 &event,  WaitForSubstructureNotify, (char*)this);
	}
	XStoreName (mNativeDpy, mNativeWindow, mApp->mTitle.c_str ());
	XSync(mNativeDpy, False);

	// Initialize egl
	EGLBoolean ret;

	mDpy = eglGetDisplay ((EGLNativeDisplayType)mNativeDpy);
	ret = eglInitialize (mDpy, &mEGLMajor, &mEGLMinor);
	if (ret != EGL_TRUE)
	{
		printf ( "eglInitialize failed!\n");
		goto close_window;
	}

	printf ("OpenGL/ES %d.%d\n", mEGLMajor, mEGLMinor);

	static EGLint attributes[] =
	{
		EGL_BUFFER_SIZE,    24,
		EGL_DEPTH_SIZE,     32,
		EGL_NONE
	};

	EGLConfig configs[2];
	EGLint config_count;

	ret = eglGetConfigs (mDpy, configs, 2, &config_count);
	if (ret != EGL_TRUE)
	{
		printf ("eglGetConfigs failed.\n");
		goto terminate;
	}

	ret = eglChooseConfig (mDpy, attributes, configs,
			       2, &config_count);
	if (ret != EGL_TRUE)
	{
		printf ("eglChooseConfig failed.\n");
		goto terminate;
	}

	mSurface = eglCreateWindowSurface (mDpy, configs[0],
					   (EGLNativeWindowType)mNativeWindow, NULL);
	if (mSurface == EGL_NO_SURFACE)
	{
		printf ( "eglCreateWindowSurface failed.\n");
		goto terminate;
	}

	EGLint width, height;
	if (!eglQuerySurface (mDpy, mSurface, EGL_WIDTH, &width) ||
	    !eglQuerySurface (mDpy, mSurface, EGL_HEIGHT, &height))
	{
		printf ("eglQuerySurface failed\n");
		goto destroy_surface;
	}

	printf ("surface size: %dx%d\n", width, height);

	mContext = eglCreateContext (mDpy, configs[0], EGL_NO_CONTEXT, NULL);
	if (mContext == EGL_NO_CONTEXT)
	{
		printf ("eglCreateContext failed.\n");
		goto destroy_surface;
	}

	ret = eglMakeCurrent (mDpy, mSurface, mSurface, mContext);
	if (ret != EGL_TRUE)
	{
		printf ("eglMakeCurrent failed");
		goto destroy_context;
	}

	mScreenImage = (GLImage*)CreateImage (mApp, mWidth, mHeight);
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mWidth;
	mPresentationRect.mHeight = mHeight;

	InitGL ();

	mInitCount++;
	mInitialized = true;

	return 0;
destroy_context:
	eglDestroyContext (mDpy, mContext);
	mContext = EGL_NO_CONTEXT;
destroy_surface:
	eglDestroySurface (mDpy, mSurface);
	mSurface = EGL_NO_SURFACE;
terminate:
	eglTerminate (mDpy);
	mDpy = 0;
close_window:
	XDestroyWindow (mNativeDpy, mNativeWindow);
	mNativeWindow = None;
close_dpy:
	XCloseDisplay (mNativeDpy);
	mNativeDpy = NULL;
fail:
	return -1;
}

void XGLESDisplay::Cleanup ()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLDisplay::Cleanup ();

	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = NULL;

	if (mContext)
		eglDestroyContext (mDpy, mContext);
	mContext = EGL_NO_CONTEXT;

	if (mSurface)
		eglDestroySurface (mDpy, mSurface);
	mSurface = EGL_NO_SURFACE;

	if (mDpy)
		eglMakeCurrent (mDpy,
				EGL_NO_SURFACE, EGL_NO_SURFACE,
				EGL_NO_CONTEXT);

	if (mDpy)
		eglTerminate (mDpy);
	mDpy = NULL;

	if (mNativeWindow)
		XDestroyWindow (mNativeDpy, mNativeWindow);
	mNativeWindow = None;

	if (mNativeDpy)
		XCloseDisplay (mNativeDpy);
	mNativeDpy = NULL;
}

bool XGLESDisplay::CanReinit(void)
{
	return mInitialized;
}

bool XGLESDisplay::Reinit (void)
{
	XEvent event;

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mWindowWidth = mWidth;
	mWindowHeight = mHeight;
	mIs3D = mApp->mIs3D;

	Atom wmState, fullScreen;

	wmState = XInternAtom (mNativeDpy, "_NET_WM_STATE", False);
	fullScreen = XInternAtom (mNativeDpy, "_NET_WM_STATE_FULLSCREEN", False);

	memset(&event, 0, sizeof(event));
	event.type = ClientMessage;
	event.xclient.window = mNativeWindow;
	event.xclient.message_type = wmState;
	event.xclient.format = 32;
	event.xclient.data.l[0] = mApp->mIsWindowed ? 0 : 1;
	event.xclient.data.l[1] = fullScreen;
	event.xclient.data.l[2] = 0;

	XSendEvent (mNativeDpy, DefaultRootWindow (mNativeDpy), False,
		    SubstructureNotifyMask, &event);
	XIfEvent (mNativeDpy,	 &event,  WaitForSubstructureNotify, (char*)this);

	delete mScreenImage;
	mScreenImage = static_cast<GLImage*>(CreateImage (mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mWidth;
	mPresentationRect.mHeight = mHeight;

	return GLDisplay::Reinit();
}

void XGLESDisplay::RemapMouse(int& theX, int& theY)
{
	theX *= (float)mWidth / mWindowWidth;
	theY *= (float)mHeight / mWindowHeight;
}

Image* XGLESDisplay::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

bool XGLESDisplay::HasEvent()
{
	if (!mNativeDpy)
		return false;

	if (XPending (mNativeDpy) > 0)
		return true;
	return false;
}

static int XKsymToKeyCode(KeySym keysym)
{
	static const struct {
		KeySym keySym;
		int    keyCode;
	} keymap[] = {
		{ XK_Left, KEYCODE_LEFT },
		{ XK_Right, KEYCODE_RIGHT },
		{ XK_Up, KEYCODE_UP },
		{ XK_Down, KEYCODE_DOWN },
		{ XK_Return, KEYCODE_RETURN },
		{ XK_Escape, KEYCODE_ESCAPE },
		{ XK_BackSpace, KEYCODE_BACK },
		{ 0, 0 }
	};
	int i;

	for (i = 0; keymap[i].keySym; i++)
		if (keymap[i].keySym == keysym)
			return keymap[i].keyCode;

	if (keysym >= XK_A && keysym <= XK_Z)
		return 'a' + keysym - XK_A;
	if (keysym >= XK_0 && keysym <= XK_9)
		return '0' + keysym - XK_0;
	if (isalnum (keysym))
		return toupper(keysym);
	return 0;
}

bool XGLESDisplay::GetEvent(struct Event &event)
{
	if (!mNativeDpy)
		return false;

	if (XPending (mNativeDpy) == 0)
		return false;

	XEvent xevent;
	XNextEvent (mNativeDpy, &xevent);

	event.type = EVENT_NONE;
	event.flags = 0;
	KeySym key;
	const char * keystr;

	//printf ("XEvent type %d\n", xevent.type);
	switch (xevent.type) {
	case KeyPress:
		XLookupString ((XKeyEvent *)&xevent, NULL, 0, &key, NULL);
		keystr = XKeysymToString(key);
		event.type = EVENT_KEY_DOWN;
		event.u.key.keyCode = XKsymToKeyCode (key);
		if (keystr && keystr[0] && !keystr[1])
			event.u.key.keyChar = keystr[0];
		else if (keystr && !strcmp(keystr, "space"))
			event.u.key.keyChar = ' ';
		else
			event.u.key.keyChar = 0;
		if (event.u.key.keyChar)
			event.flags |= EVENT_FLAGS_KEY_CHAR;
		break;
	case KeyRelease:
	{
		XKeyEvent * ke = (XKeyEvent *)&xevent;

		XLookupString ((XKeyEvent *)&xevent, NULL, 0, &key, NULL);
		event.type = EVENT_KEY_UP;
		event.u.key.keyCode = XKsymToKeyCode (key);
		if (key == XK_Escape &&
		    (ke->state & (ControlMask | ShiftMask)) == (ControlMask | ShiftMask))
			event.type = EVENT_QUIT;
		break;
	}
	case ButtonPress:
	{
		XButtonPressedEvent * be = (XButtonPressedEvent*)&xevent;
		if (be->button >= Button1 && be->button <= Button3) {
			event.type = EVENT_MOUSE_BUTTON_PRESS;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (be->button) {
			case Button1:
				event.u.mouse.button = 1;
				break;
			case Button2:
				event.u.mouse.button = 3;
				break;
			case Button3:
				event.u.mouse.button = 2;
				break;
			default:
				break;
			}

			event.u.mouse.x = be->x;
			event.u.mouse.y = be->y;
		}
		break;
	}
	case ButtonRelease:
	{
		XButtonReleasedEvent * be = (XButtonReleasedEvent*)&xevent;
		if (be->button >= Button1 && be->button <= Button3) {
			event.type = EVENT_MOUSE_BUTTON_RELEASE;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (be->button) {
			case Button1:
				event.u.mouse.button = 1;
				break;
			case Button2:
				event.u.mouse.button = 3;
				break;
			case Button3:
				event.u.mouse.button = 2;
				break;
			default:
				break;
			}

			event.u.mouse.x = be->x;
			event.u.mouse.y = be->y;
		}
		break;
	}
	case MotionNotify:
	{
		XMotionEvent * me = (XMotionEvent*)&xevent;
		event.type = EVENT_MOUSE_MOTION;
		event.flags = EVENT_FLAGS_AXIS;

		event.u.mouse.x = me->x;
		event.u.mouse.y = me->y;
		break;
	}
	case Expose:
		event.type = EVENT_EXPOSE;
		Redraw (NULL);
		break;
	case ConfigureNotify:
		if (xevent.xmap.window == mNativeWindow)
		{
			mWindowWidth = xevent.xconfigure.width;
			mWindowHeight = xevent.xconfigure.height;
			Reshape();
			return GL_TRUE;
		}
		break;
	case EnterNotify:
		event.type = EVENT_ACTIVE;
		event.u.active.active = true;
		break;
	case LeaveNotify:
		event.type = EVENT_ACTIVE;
		event.u.active.active = false;
		break;
	case ClientMessage:
		if (xevent.xclient.data.l[0] == (long)mWMDeleteMessage)
			event.type = EVENT_QUIT;
		break;
	default:
		break;
	}

        if (event.type != EVENT_NONE && event.flags & EVENT_FLAGS_AXIS)
        {
                event.u.mouse.x *= (float)mWidth / mWindowWidth;
                event.u.mouse.y *= (float)mHeight / mWindowHeight;
        }

	return true;
}

void XGLESDisplay::SwapBuffers()
{
	if (mDpy && mSurface)
		eglSwapBuffers (mDpy, mSurface);
}

class XGLESVideoDriver: public VideoDriver {
public:
	XGLESVideoDriver ()
	: VideoDriver("XGLES", 10)
        {
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new XGLESDisplay (theApp);
	}
};

static XGLESVideoDriver aXGLESVideoDriver;
VideoDriverRegistor aXGLESVideoDriverRegistor (&aXGLESVideoDriver);
VideoDriver* GetXGLESVideoDriver()
{
	return &aXGLESVideoDriver;
}

