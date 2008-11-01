#ifndef __WGLINTERFACE_H__
#define __WGLINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLInterface.h"
#include "Rect.h"
#include "Ratio.h"

#include <GL/gl.h>

namespace Sexy
{

class SexyAppBase;
class Image;
class MemoryImage;

class WGLInterface : public GLInterface
{
public:
	WGLInterface(SexyAppBase* theApp);
	virtual ~WGLInterface();

	virtual int				Init();
	virtual void				Cleanup();

	virtual void				SwapBuffers();

	virtual void				RemapMouse(int& theX, int& theY);
	virtual bool				EnableCursor(bool enable);
	virtual bool				SetCursorImage(Image* theImage, int theHotX = 0, int theHotY = 0);
	virtual void				SetCursorPos(int theCursorX, int theCursorY);

	virtual Image*				CreateImage(SexyAppBase * theApp,
							    int width, int height);
	virtual bool				HasEvent();
	virtual bool				GetEvent(struct Event &event);

private:
	typedef std::list<Event> EventQueue;
	HDC					mHDC;
	HWND					mWindow;
	HGLRC					mContext;
	int					mGLXMajor;
	int					mGLXMinor;

	EventQueue				mEventQueue;

	static LONG WINAPI			WndProc (HWND	 hWnd,
							 UINT	 uMsg,
							 WPARAM	 wParam,
							 LPARAM	 lParam);

	void					PumpMsg();
};

}

#endif //__GLINTERFACE_H__

