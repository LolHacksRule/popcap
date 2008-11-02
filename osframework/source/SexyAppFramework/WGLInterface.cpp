#include "SexyAppBase.h"
#include "WGLInterface.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"

#include "windowsx.h"

//#include <GL/glu.h>

#include <cstdio>

using namespace Sexy;

WGLInterface::WGLInterface (SexyAppBase* theApp)
	: GLInterface (theApp)
{
	mWindow = 0;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
}

WGLInterface::~WGLInterface ()
{
	Cleanup();
}

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(l) ((l) & 0xffff)
#define GET_Y_LPARAM(l) ((l) >> 16)
#endif

LONG WINAPI WGLInterface::WndProc (HWND	   hWnd,
				   UINT	   uMsg,
				   WPARAM  wParam,
				   LPARAM  lParam)
{
	LONG ret;

	WGLInterface * interface;

	interface = (WGLInterface*)GetWindowLongPtr (hWnd, GWLP_USERDATA);
	if (!interface)
		return DefWindowProc (hWnd, uMsg, wParam, lParam);

	ret = 1;
	Event event;
	event.type = EVENT_NONE;
	switch (uMsg)
	{
	case WM_CLOSE:
		event.type = EVENT_QUIT;
		break;
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
		break;
	case WM_LBUTTONDOWN:
		event.type = EVENT_MOUSE_BUTTON_PRESS;
		event.button = 1;
		event.x = GET_X_LPARAM (lParam);
		event.y = GET_Y_LPARAM (lParam);
		break;
	case WM_LBUTTONUP:
		event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.button = 1;
		event.x = GET_X_LPARAM (lParam);
		event.y = GET_Y_LPARAM (lParam);
		break;
	case WM_MBUTTONDOWN:
		event.type = EVENT_MOUSE_BUTTON_PRESS;
		event.button = 3;
		event.x = GET_X_LPARAM (lParam);
		event.y = GET_Y_LPARAM (lParam);
		break;
	case WM_MBUTTONUP:
		event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.button = 3;
		event.x = GET_X_LPARAM (lParam);
		event.y = GET_Y_LPARAM (lParam);
		break;
	case WM_RBUTTONDOWN:
		event.type = EVENT_MOUSE_BUTTON_PRESS;
		event.button = 2;
		event.x = GET_X_LPARAM (lParam);
		event.y = GET_Y_LPARAM (lParam);
		break;
	case WM_RBUTTONUP:
		event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.button = 2;
		event.x = GET_X_LPARAM (lParam);
		event.y = GET_Y_LPARAM (lParam);
		break;
	case WM_MOUSEMOVE:
		event.type = EVENT_MOUSE_MOTION;
		event.x = GET_X_LPARAM (lParam);
		event.y = GET_Y_LPARAM (lParam);
		break;
	default:
		ret = DefWindowProc (hWnd, uMsg, wParam, lParam);
		break;
	}

	if (event.type != EVENT_NONE)
		interface->mEventQueue.push_back (event);

	return ret;
}

int WGLInterface::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLInterface::Init();

	HINSTANCE  hInstance = GetModuleHandle (NULL);
	WNDCLASS   wndclass;

	/* Register the frame class */
	wndclass.style	       = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = (WNDPROC)WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon	       = LoadIcon (hInstance, "SexyGL");
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject (BLACK_BRUSH);
	wndclass.lpszMenuName  = "SexyGL";
	wndclass.lpszClassName = "SexyGL";

	if (!RegisterClass (&wndclass) )
		goto fail;

	/* Create the frame */
	mWindow = CreateWindow ("SexyGL",
				"SexyGL",
				WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				mWidth,
				mHeight,
				NULL,
				NULL,
				hInstance,
				NULL);
	if (!mWindow)
		goto fail;

	mHDC = GetDC (mWindow);

	PIXELFORMATDESCRIPTOR pfd, *ppfd;
	int pixelformat;

	ppfd = &pfd;

	ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
	ppfd->nVersion = 1;
	ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	ppfd->dwLayerMask = PFD_MAIN_PLANE;
	ppfd->iPixelType = PFD_TYPE_RGBA;
	ppfd->cColorBits = 24;
	ppfd->cDepthBits = 0;
	ppfd->cAccumBits = 0;
	ppfd->cStencilBits = 0;

	pixelformat = ChoosePixelFormat (mHDC, ppfd);
	if (!pixelformat)
		goto close_window;
	if (SetPixelFormat (mHDC, pixelformat, ppfd) == FALSE)
		goto close_window;

	ShowWindow (mWindow, SW_SHOW);
	UpdateWindow (mWindow);

	SetWindowLongPtr (mWindow, GWLP_USERDATA, (LONG_PTR)this);

	WINDOWINFO wininfo;
	GetWindowInfo (mWindow, &wininfo);

	RECT rect;
	GetClientRect (mWindow, &rect);
	if (rect.right != mWidth || rect.bottom != mHeight)
	{
		MoveWindow (mWindow,
			    wininfo.rcWindow.left,
			    wininfo.rcWindow.top,
			    wininfo.rcWindow.right - rect.right + mWidth,
			    wininfo.rcWindow.bottom - rect.bottom + mHeight,
			    FALSE);
	}

	GetClientRect (mWindow, &rect);
	mWidth = rect.right;
	mHeight = rect.bottom;
	mApp->mWidth = mWidth;
	mApp->mHeight = mHeight;

	mContext = wglCreateContext (mHDC);
	wglMakeCurrent (mHDC, mContext);

	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	InitGL ();

	mInitCount++;
	mInitialized = true;

	return 0;
 close_window:
	DestroyWindow (mWindow);
	mWindow = 0;
 fail:
	return -1;
}

void WGLInterface::Cleanup ()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLInterface::Cleanup ();

	mEventQueue.clear ();
	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = NULL;

	if (mContext)
		wglMakeCurrent (NULL, NULL);
	if (mContext)
		wglDeleteContext (mContext);
	mContext = 0;

	if (mHDC)
		ReleaseDC (mWindow, mHDC);
	mHDC = 0;

	if (mWindow)
		DestroyWindow (mWindow);
	mWindow = 0;
}

void WGLInterface::RemapMouse(int& theX, int& theY)
{
}

bool WGLInterface::EnableCursor(bool enable)
{
	return false;
}

bool WGLInterface::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	return false;
}

void WGLInterface::SetCursorPos(int theCursorX, int theCursorY)
{
}

Image* WGLInterface::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

void WGLInterface::PumpMsg()
{
	MSG msg;
	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE)
	{
		if (GetMessage (&msg, NULL, 0, 0) )
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		} else {
			break;
		}
	}
}

bool WGLInterface::HasEvent()
{
	if (!mWindow)
		return false;
	PumpMsg ();
	if (mEventQueue.size () > 0)
		return true;
	return false;
}

bool WGLInterface::GetEvent(struct Event &event)
{
	if (!mWindow)
		return false;
	PumpMsg ();

	event.type = EVENT_NONE;
	if (mEventQueue.size () == 0)
		return false;

	event = mEventQueue.front ();
	mEventQueue.pop_front ();
	return true;
}

void WGLInterface::SwapBuffers()
{
	if (mHDC)
		::SwapBuffers (mHDC);
}

class WGLVideoDriver: public VideoDriver {
public:
	WGLVideoDriver ()
		: VideoDriver("WGLInterface", 10)
		{
		}

	NativeDisplay* Create (SexyAppBase * theApp)
		{
			return new WGLInterface (theApp);
		}
};

static WGLVideoDriver aWGLVideoDriver;
VideoDriverRegistor aWGLVideoDriverRegistor (&aWGLVideoDriver);
VideoDriver* GetWGLVideoDriver()
{
	return &aWGLVideoDriver;
}

