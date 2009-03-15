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

	mScreen = DefaultScreen (mDpy);
	mDesktopWidth = DisplayWidth (mDpy, mScreen);
	mDesktopHeight = DisplayHeight (mDpy, mScreen);
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mWindowWidth = mWidth;
	mWindowHeight = mHeight;
	if (!glXQueryExtension (mDpy, NULL, NULL))
		goto close_dpy;

	static int attributes[] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};
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

	/* create an OpenGL rendering context */
	mContext = glXCreateContext (mDpy, visualInfo,  None, GL_TRUE);
	if (!mContext)
		goto close_window;
	glXMakeCurrent (mDpy, mWindow, mContext);

	XMapWindow (mDpy, mWindow);

	XEvent event;
	XIfEvent (mDpy,  &event,  WaitForMapNotify, (char*)mWindow);

	XSync (mDpy, FALSE);

	XIfEvent (mDpy,  &event,  WaitForSubstructureNotify, (char*)this);

	while (XPending (mDpy))
		XNextEvent (mDpy, &event);

	Atom wm_state, fullscreen;

	wm_state = XInternAtom(mDpy, "_NET_WM_STATE", False);
	fullscreen = XInternAtom(mDpy, "_NET_WM_STATE_FULLSCREEN", False);

	if (!mApp->mIsWindowed) {
		memset(&event, 0, sizeof(event));
		event.type = ClientMessage;
		event.xclient.window = mWindow;
		event.xclient.message_type = wm_state;
		event.xclient.format = 32;
		event.xclient.data.l[0] = 1;
		event.xclient.data.l[1] = fullscreen;
		event.xclient.data.l[2] = 0;

		XSendEvent (mDpy, DefaultRootWindow(mDpy), False,
			    SubstructureNotifyMask, &event);
		XIfEvent (mDpy,  &event,  WaitForSubstructureNotify, (char*)this);
	}
	XStoreName (mDpy, mWindow, mApp->mTitle.c_str ());

	mScreenImage = static_cast<GLImage*>(CreateImage (mApp, mWidth, mHeight));

	mScreenImage->mFlags = (ImageFlags)(IMAGE_FLAGS_DOUBLE_BUFFER);

	if (mWidth != mWindowWidth || mHeight != mWindowHeight) {
		mTrans[0].LoadIdentity ();
		mTrans[0].Translate (0, 0);
		mTrans[0].Scale (((float)mWindowWidth) / mWidth,
				 ((float)mWindowHeight) / mHeight);

		mTrans[1].LoadIdentity ();
		mTrans[1].Scale (mWidth / ((float)mWindowWidth),
				 mHeight / ((float)mWindowHeight));
		mTrans[1].Translate (0, 0);
	} else {
		mTrans[0].LoadIdentity ();
		mTrans[1].LoadIdentity ();
	}

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

void GLXInterface::RemapMouse(int& theX, int& theY)
{
	theX *= (float)mWidth / mDisplayWidth;
	theY *= (float)mHeight / mDisplayHeight;
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
	{ 0, 0 }
    };
    int i;

    for (i = 0; keymap[i].keySym; i++)
	if (keymap[i].keySym == keysym)
	    return keymap[i].keyCode;

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

	//printf ("XEvent type %d\n", xevent.type);
        switch (xevent.type) {
        case KeyPress:
                XLookupString ((XKeyEvent *)&xevent, NULL, 0, &key, NULL);
		event.type = EVENT_KEY_DOWN;
		event.u.key.keyCode = XKsymToKeyCode (key);
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

			SexyVector2 v(be->x, be->y);
			v = mTrans[1] * v;

			event.u.mouse.x = v.x;
			event.u.mouse.y = v.y;
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

			SexyVector2 v(be->x, be->y);
			v = mTrans[1] * v;

			event.u.mouse.x = v.x;
			event.u.mouse.y = v.y;
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

		SexyVector2 v(me->x, me->y);
		v = mTrans[1] * v;

		event.u.mouse.x = v.x;
		event.u.mouse.y = v.y;
		break;
	}
	case Expose:
		event.type = EVENT_EXPOSE;
		Redraw (NULL);
		break;
        case ConfigureNotify:
		/* resize (xevent.xconfigure.width, xevent.xconfigure.height); */
		break;
	case EnterNotify:
		event.type = EVENT_ACTIVE;
		event.u.active.active = true;
		break;
	case LeaveNotify:
		event.type = EVENT_ACTIVE;
		event.u.active.active = false;
		break;
	default:
		break;
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

