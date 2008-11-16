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

int GLXInterface::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLInterface::Init();

	mDpy = XOpenDisplay (NULL);
	if (!mDpy)
		goto fail;

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
	mWindow = XCreateWindow (mDpy, RootWindow (mDpy, visualInfo->screen), 0, 0, mWidth,
				 mHeight, 0, visualInfo->depth, InputOutput, visualInfo->visual,
				 CWBorderPixel | CWColormap | CWEventMask, &swa);

	/* create an OpenGL rendering context */
	mContext = glXCreateContext (mDpy, visualInfo,  None, GL_TRUE);
	if (!mContext)
		goto close_window;
	glXMakeCurrent (mDpy, mWindow, mContext);

	XMapWindow (mDpy, mWindow);

	XEvent event;
	XIfEvent (mDpy,  &event,  WaitForMapNotify, (char*)mWindow);

	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	InitGL ();

	mScreenImage->mFlags =
		(ImageFlags)(IMAGE_FLAGS_DOUBLE_BUFFER |
			     IMAGE_FLAGS_FLIP_AS_COPY);

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
}

bool GLXInterface::EnableCursor(bool enable)
{
	return false;
}

bool GLXInterface::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	return false;
}

void GLXInterface::SetCursorPos(int theCursorX, int theCursorY)
{
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
		switch (key) {
		case XK_Left:
			break;
		case XK_Right:
			break;
		case XK_Up:
			break;
		case XK_Down:
			break;
		case XK_Escape:
			break;
		}
		break;
	case KeyRelease:
	{
		XKeyEvent * ke = (XKeyEvent *)&xevent;

                XLookupString ((XKeyEvent *)&xevent, NULL, 0, &key, NULL);
		switch (key) {
		case XK_Left:
			break;
		case XK_Right:
			break;
		case XK_Up:
			break;
		case XK_Down:
			break;
		case XK_Escape:
			if ((ke->state & (ControlMask | ShiftMask)) == (ControlMask | ShiftMask))
				event.type = EVENT_QUIT;
			break;
		}
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
				event.button = 1;
				break;
			case Button2:
				event.button = 3;
				break;
			case Button3:
				event.button = 2;
				break;
			default:
				break;
			}
			event.x = be->x;
			event.y = be->y;
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
				event.button = 1;
				break;
			case Button2:
				event.button = 3;
				break;
			case Button3:
				event.button = 2;
				break;
			default:
				break;
			}
			event.x = be->x;
			event.y = be->y;
		}
		break;
	}
	case MotionNotify:
	{
		XMotionEvent * me = (XMotionEvent*)&xevent;
		event.type = EVENT_MOUSE_MOTION;
		event.flags = EVENT_FLAGS_AXIS;
		event.x = me->x;
		event.y = me->y;
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
		event.active = true;
		break;
	case LeaveNotify:
		event.type = EVENT_ACTIVE;
		event.active = false;
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

