#include "SexyAppBase.h"
#include "GLXInterface.h"
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

GLXInterface::GLXInterface (SexyAppBase* theApp)
	: GLInterface (theApp)
{
	mDpy = 0;
	mWindow = None;
	mContext = NULL;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
}

GLXInterface::~GLXInterface ()
{
	Cleanup();
}

static Bool WaitForMapNotify(Display *d, XEvent *e, char* arg)
{
	if ((e->type == MapNotify) && (e->xmap.window == (Window)arg))
		return GL_TRUE;
	return GL_FALSE;
}

Bool GLXInterface::WaitForSubstructureNotify(Display *d, XEvent *e, char* arg)
{
	GLXInterface * interface = (GLXInterface*)arg;

	if ((e->type == ConfigureNotify) &&
	    (e->xmap.window == interface->mWindow)) {
		interface->mWindowWidth = e->xconfigure.width;
		interface->mWindowHeight = e->xconfigure.height;
		return GL_TRUE;
	}

	return GL_FALSE;
}

int GLXInterface::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLInterface::Init();

	mDpy = XOpenDisplay (NULL);
	if (!mDpy)
		goto fail;

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mScreen = DefaultScreen (mDpy);
	mDesktopWidth = DisplayWidth (mDpy, mScreen);
	mDesktopHeight = DisplayHeight (mDpy, mScreen);
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mWindowWidth = mWidth;
	mWindowHeight = mHeight;
	if (!glXQueryExtension (mDpy, NULL, NULL))
		goto close_dpy;

	static int attributes[] = {GLX_RGBA, GLX_ALPHA_SIZE, 8, GLX_DOUBLEBUFFER, None};
	XVisualInfo* visualInfo;

	visualInfo = glXChooseVisual (mDpy, DefaultScreen (mDpy), attributes);
	if (!visualInfo)
		goto close_dpy;

	Colormap colorMap;

	/* create an X colormap since probably not using default visual */
	colorMap = XCreateColormap (mDpy, RootWindow (mDpy, visualInfo->screen),
				    visualInfo->visual, AllocNone);

	XSetWindowAttributes swa;
	swa.colormap = colorMap;
	swa.border_pixel = 0;
	swa.event_mask = KeyPressMask | KeyReleaseMask |
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
		EnterWindowMask | LeaveWindowMask | ExposureMask | StructureNotifyMask;
	mWindow = XCreateWindow (mDpy, RootWindow (mDpy, visualInfo->screen), 0, 0,
				 mWindowWidth, mWindowHeight, 0, visualInfo->depth,
				 InputOutput, visualInfo->visual,
				 CWBorderPixel | CWColormap | CWEventMask, &swa);

	XSync(mDpy, False);

	/* create an OpenGL rendering context */
	mContext = glXCreateContext (mDpy, visualInfo,	None, GL_TRUE);
	if (!mContext)
		goto close_window;
	glXMakeCurrent (mDpy, mWindow, mContext);

	XMapWindow (mDpy, mWindow);
	XSync (mDpy, False);

	XEvent event;
	XIfEvent (mDpy,	 &event,  WaitForMapNotify, (char*)mWindow);

	XSync (mDpy, FALSE);

	XIfEvent (mDpy,	 &event,  WaitForSubstructureNotify, (char*)this);

	while (XPending (mDpy))
		XNextEvent (mDpy, &event);

	Pixmap pixmap;
	Cursor cursor;
	XColor black, dummy;
	static char cursor_data[] = {0, 0, 0, 0, 0, 0, 0, 0};

	XAllocNamedColor (mDpy, colorMap, "black", &black, &dummy);
	pixmap = XCreateBitmapFromData (mDpy, mWindow, cursor_data, 8, 8);
	cursor = XCreatePixmapCursor (mDpy, pixmap, pixmap, &black, &black, 0, 0);

	XDefineCursor (mDpy, mWindow, cursor);
	XFreeCursor (mDpy, cursor);
	XFreePixmap (mDpy, pixmap);

	mWMDeleteMessage = XInternAtom (mDpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols (mDpy, mWindow, &mWMDeleteMessage, 1);

	Atom wmState, fullScreen;

	wmState = XInternAtom (mDpy, "_NET_WM_STATE", False);
	fullScreen = XInternAtom (mDpy, "_NET_WM_STATE_FULLSCREEN", False);

	if (!mApp->mIsWindowed)
	{
		memset(&event, 0, sizeof(event));
		event.type = ClientMessage;
		event.xclient.window = mWindow;
		event.xclient.message_type = wmState;
		event.xclient.format = 32;
		event.xclient.data.l[0] = 1;
		event.xclient.data.l[1] = fullScreen;
		event.xclient.data.l[2] = 0;

		XSendEvent (mDpy, DefaultRootWindow (mDpy), False,
			    SubstructureNotifyMask, &event);
		XIfEvent (mDpy,	 &event,  WaitForSubstructureNotify, (char*)this);
	}
	XStoreName (mDpy, mWindow, mApp->mTitle.c_str ());
	XSync(mDpy, False);

	mScreenImage = static_cast<GLImage*>(CreateImage (mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mWidth;
	mPresentationRect.mHeight = mHeight;

	InitGL ();

	mInitCount++;
	mInitialized = true;

	return 0;
close_window:
	XDestroyWindow (mDpy, mWindow);
	mWindow = None;
close_dpy:
	XCloseDisplay (mDpy);
	mDpy = NULL;
fail:
	return -1;
}

void GLXInterface::Cleanup ()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLInterface::Cleanup ();

	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = NULL;

	if (mDpy)
		glXMakeCurrent (mDpy, None, NULL);

	if (mContext)
		glXDestroyContext (mDpy, mContext);
	mContext = NULL;

	if (mWindow)
		XDestroyWindow (mDpy, mWindow);
	mWindow = None;

	if (mDpy)
		XCloseDisplay (mDpy);
	mDpy = NULL;
}

bool GLXInterface::CanReinit(void)
{
	return mInitialized;
}

bool GLXInterface::Reinit (void)
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

	wmState = XInternAtom (mDpy, "_NET_WM_STATE", False);
	fullScreen = XInternAtom (mDpy, "_NET_WM_STATE_FULLSCREEN", False);

	memset(&event, 0, sizeof(event));
	event.type = ClientMessage;
	event.xclient.window = mWindow;
	event.xclient.message_type = wmState;
	event.xclient.format = 32;
	event.xclient.data.l[0] = mApp->mIsWindowed ? 0 : 1;
	event.xclient.data.l[1] = fullScreen;
	event.xclient.data.l[2] = 0;

	XSendEvent (mDpy, DefaultRootWindow (mDpy), False,
		    SubstructureNotifyMask, &event);
	XIfEvent (mDpy,	 &event,  WaitForSubstructureNotify, (char*)this);

	delete mScreenImage;
	mScreenImage = static_cast<GLImage*>(CreateImage (mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mWidth;
	mPresentationRect.mHeight = mHeight;

	return GLInterface::Reinit();
}

void GLXInterface::RemapMouse(int& theX, int& theY)
{
	theX *= (float)mWidth / mWindowWidth;
	theY *= (float)mHeight / mWindowHeight;
}

Image* GLXInterface::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

bool GLXInterface::HasEvent()
{
	if (!mDpy)
		return false;

	if (XPending (mDpy) > 0)
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

bool GLXInterface::GetEvent(struct Event &event)
{
	if (!mDpy)
		return false;

	if (XPending (mDpy) == 0)
		return false;

	XEvent xevent;
	XNextEvent (mDpy, &xevent);

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
		if (xevent.xmap.window == mWindow)
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

void GLXInterface::SwapBuffers()
{
	if (mDpy && mWindow)
		glXSwapBuffers (mDpy, mWindow);
}

class GLXVideoDriver: public VideoDriver {
public:
	GLXVideoDriver ()
	: VideoDriver("GLX", 10)
        {
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new GLXInterface (theApp);
	}
};

static GLXVideoDriver aGLXVideoDriver;
VideoDriverRegistor aGLXVideoDriverRegistor (&aGLXVideoDriver);
VideoDriver* GetGLXVideoDriver()
{
	return &aGLXVideoDriver;
}

