#include "SexyAppBase.h"
#include "WGLDisplay.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"
#include "SexyMatrix.h"

#include "windowsx.h"

//#include <GL/glu.h>

#include <cstdio>

using namespace Sexy;

WGLDisplay::WGLDisplay (SexyAppBase* theApp)
	: GLDisplay (theApp)
{
	mWindow = 0;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mSysCursor = TRUE;
	mBlankCursor = 0;
	mHDC = 0;
	mContext = 0;
}

WGLDisplay::~WGLDisplay ()
{
	Cleanup();
}

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(l) ((l) & 0xffff)
#define GET_Y_LPARAM(l) ((l) >> 16)
#endif

static int WinKeyToKeyCode(WPARAM winKey)
{
    static const struct {
	WPARAM winKey;
	int    keyCode;
    } keymap[] = {
	{ VK_LEFT, KEYCODE_LEFT },
	{ VK_RIGHT, KEYCODE_RIGHT },
	{ VK_UP, KEYCODE_UP },
	{ VK_DOWN, KEYCODE_DOWN },
	{ VK_RETURN, KEYCODE_RETURN },
	{ VK_ESCAPE, KEYCODE_ESCAPE },
	{ VK_BACK, KEYCODE_BACK },
	{ VK_TAB, KEYCODE_TAB },
	{ VK_SPACE, KEYCODE_SPACE },
	{ 0, 0 }
    };
    int i;

    for (i = 0; keymap[i].winKey; i++)
	if (keymap[i].winKey == winKey)
	    return keymap[i].keyCode;

    if (isalnum (winKey))
	    return winKey;

    return 0;
}

LONG WINAPI WGLDisplay::WndProc (HWND	   hWnd,
				   UINT	   uMsg,
				   WPARAM  wParam,
				   LPARAM  lParam)
{
	LONG ret;

	WGLDisplay * interface;

	interface = (WGLDisplay*)GetWindowLongPtr (hWnd, GWLP_USERDATA);
	if (!interface)
		return DefWindowProc (hWnd, uMsg, wParam, lParam);

	ret = 1;
	Event event;
	event.type = EVENT_NONE;
	event.flags = 0;
	switch (uMsg)
	{
	case WM_CLOSE:
		event.type = EVENT_QUIT;
		break;
	case WM_KEYDOWN:
		event.type = EVENT_KEY_DOWN;
		event.flags = EVENT_FLAGS_KEY_CODE;
		event.u.key.keyCode = WinKeyToKeyCode (wParam);
		event.u.key.keyChar = 0;
		break;
	case WM_CHAR:
		event.type = EVENT_KEY_DOWN;
		event.flags = EVENT_FLAGS_KEY_CODE | EVENT_FLAGS_KEY_CHAR;
		event.u.key.keyChar = wParam;
		event.u.key.keyCode = 0;
		break;
	case WM_KEYUP:
		event.type = EVENT_KEY_UP;
		event.flags = EVENT_FLAGS_KEY_CODE;
		event.u.key.keyCode = WinKeyToKeyCode (wParam);
		break;
	case WM_LBUTTONDOWN:
		event.type = EVENT_MOUSE_BUTTON_PRESS;
		event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
		event.u.mouse.button = 1;
		event.u.mouse.x = GET_X_LPARAM (lParam);
		event.u.mouse.y = GET_Y_LPARAM (lParam);
		interface->RemapMouse(event.u.mouse.x, event.u.mouse.y);
		break;
	case WM_LBUTTONUP:
		event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
		event.u.mouse.button = 1;
		event.u.mouse.x = GET_X_LPARAM (lParam);
		event.u.mouse.y = GET_Y_LPARAM (lParam);
		interface->RemapMouse(event.u.mouse.x, event.u.mouse.y);
		break;
	case WM_MBUTTONDOWN:
		event.type = EVENT_MOUSE_BUTTON_PRESS;
		event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
		event.u.mouse.button = 3;
		event.u.mouse.x = GET_X_LPARAM (lParam);
		event.u.mouse.y = GET_Y_LPARAM (lParam);
		interface->RemapMouse(event.u.mouse.x, event.u.mouse.y);

		break;
	case WM_MBUTTONUP:
		event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
		event.u.mouse.button = 3;
		event.u.mouse.x = GET_X_LPARAM (lParam);
		event.u.mouse.y = GET_Y_LPARAM (lParam);
		interface->RemapMouse(event.u.mouse.x, event.u.mouse.y);
		break;
	case WM_RBUTTONDOWN:
		event.type = EVENT_MOUSE_BUTTON_PRESS;
		event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
		event.u.mouse.button = 2;
		event.u.mouse.x = GET_X_LPARAM (lParam);
		event.u.mouse.y = GET_Y_LPARAM (lParam);
		interface->RemapMouse(event.u.mouse.x, event.u.mouse.y);
		break;
	case WM_RBUTTONUP:
		event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
		event.u.mouse.button = 2;
		event.u.mouse.x = GET_X_LPARAM (lParam);
		event.u.mouse.y = GET_Y_LPARAM (lParam);
		interface->RemapMouse(event.u.mouse.x, event.u.mouse.y);
		break;
	case WM_MOUSEMOVE:
		if (interface->mCursorImage != interface->mBlankCursor && interface->mSysCursor)
		{
			ShowCursor (FALSE);
			interface->mSysCursor = false;
		}

		event.type = EVENT_MOUSE_MOTION;
		event.flags = EVENT_FLAGS_AXIS;
		event.u.mouse.x = GET_X_LPARAM (lParam);
		event.u.mouse.y = GET_Y_LPARAM (lParam);
		interface->RemapMouse(event.u.mouse.x, event.u.mouse.y);
		break;
	case WM_NCMOUSEMOVE:
		if (!interface->mSysCursor)
		{
			ShowCursor (TRUE);
			interface->mSysCursor = true;
		}

		ret = DefWindowProc (hWnd, uMsg, wParam, lParam);
		break;
	case WM_KILLFOCUS:
		event.type = EVENT_ACTIVE;
		event.u.active.active = false;
		break;
	case WM_SETFOCUS:
		event.type = EVENT_ACTIVE;
		event.u.active.active = true;
		break;
	default:
		ret = DefWindowProc (hWnd, uMsg, wParam, lParam);
		break;
	}

	if (event.type != EVENT_NONE)
		interface->mEventQueue.push_back (event);

	return ret;
}

int WGLDisplay::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLDisplay::Init();

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mAspect.Set(mWidth, mHeight);
	mDesktopWidth = GetSystemMetrics (SM_CXSCREEN);
	mDesktopHeight = GetSystemMetrics (SM_CYSCREEN);
	mDesktopAspect.Set (mDesktopWidth, mDesktopHeight);
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mDisplayAspect = mAspect;
	mPresentationRect = Rect (0, 0, mWidth, mHeight);
	mApp->mScreenBounds = mPresentationRect;

	HINSTANCE  hInstance = GetModuleHandle (NULL);
	WNDCLASS   wndclass;

	/* Register the frame class */
	wndclass.style	       = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = (WNDPROC)WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon	       = LoadIcon (hInstance, "IDI_MAIN_ICON");
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject (BLACK_BRUSH);
	wndclass.lpszMenuName  = "SexyGL";
	wndclass.lpszClassName = "SexyGL";

	if (!RegisterClass (&wndclass) )
		goto fail;

	/* Create the frame */
	static const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	mWindow = CreateWindow ("SexyGL",
				mApp->mTitle.c_str(),
				style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
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

	RECT rect;
	if (mApp->mIsWindowed) {
		WINDOWINFO wininfo;
		GetWindowInfo (mWindow, &wininfo);

		RECT rect;
		rect.left = 0;
		rect.right = mApp->mWidth;
		rect.top = 0;
		rect.bottom = mApp->mHeight;
		AdjustWindowRect (&rect, wininfo.dwStyle, FALSE);
		MoveWindow (mWindow, wininfo.rcWindow.left, wininfo.rcWindow.top,
			    rect.right - rect.left, rect.bottom - rect.top, FALSE);
	} else {
		WINDOWPLACEMENT placement;
		int style;

		placement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement (mWindow, &placement);

		style = WS_CLIPCHILDREN | WS_VISIBLE;
		SetWindowLong (mWindow, GWL_STYLE, style);

		SetForegroundWindow (mWindow);
		ShowWindow (mWindow, SW_SHOW);

		placement.showCmd = SW_SHOWNORMAL;
		placement.rcNormalPosition.left = 0;
		placement.rcNormalPosition.right = mDesktopWidth;
		placement.rcNormalPosition.top = 0;
		placement.rcNormalPosition.bottom = mDesktopHeight;
		SetWindowPlacement(mWindow, &placement);
		SetWindowPos(mWindow, 0, 0, 0, 0, 0,
			     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	GetClientRect (mWindow, &rect);
	mWindowWidth = rect.right - rect.left;
	mWindowHeight = rect.bottom - rect.top;
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mDisplayWidth;
	mPresentationRect.mHeight = mDisplayHeight;

	mContext = wglCreateContext (mHDC);
	wglMakeCurrent (mHDC, mContext);

	ShowCursor (FALSE);
	mSysCursor = 0;

	mBlankCursor = CreateImage (mApp, 1, 1);

	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

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

void WGLDisplay::Cleanup ()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLDisplay::Cleanup ();

	if (!mSysCursor)
		ShowCursor (TRUE);
	mSysCursor = TRUE;

	if (mBlankCursor)
		delete mBlankCursor;
	mBlankCursor = 0;

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

	UnregisterClass ("SexyGL", GetModuleHandle (NULL));
}

void WGLDisplay::RemapMouse(int& theX, int& theY)
{
    theX = (float)theX * mDisplayWidth / mWindowWidth;
    theY = (float)theY * mDisplayHeight / mWindowHeight;
}

Image* WGLDisplay::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

void WGLDisplay::PumpMsg()
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

bool WGLDisplay::HasEvent()
{
	if (!mWindow)
		return false;
	PumpMsg ();
	if (mEventQueue.size () > 0)
		return true;
	return false;
}

bool WGLDisplay::GetEvent(struct Event &event)
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

void WGLDisplay::SwapBuffers()
{
	if (mHDC)
		::SwapBuffers (mHDC);
}

bool WGLDisplay::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	if (theImage == mApp->mArrowCursor)
	{
		if (mSysCursor)
			return true;

		ShowCursor (TRUE);
		mSysCursor = true;

		theImage = mBlankCursor;
		theHotX = 0;
		theHotY = 0;
	}
	else
	{
		if (mSysCursor)
			ShowCursor (FALSE);
		mSysCursor = false;
	}

	return GLDisplay::SetCursorImage(theImage, theHotX, theHotY);
}

class WGLVideoDriver: public VideoDriver {
public:
	WGLVideoDriver ()
		: VideoDriver("WGL", 10)
	{
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new WGLDisplay (theApp);
	}
};

static WGLVideoDriver aWGLVideoDriver;
VideoDriverRegistor aWGLVideoDriverRegistor (&aWGLVideoDriver);
VideoDriver* GetWGLVideoDriver()
{
	return &aWGLVideoDriver;
}

