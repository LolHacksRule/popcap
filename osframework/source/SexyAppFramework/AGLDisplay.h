#ifndef __AGLINTERFACE_H__
#define __AGLINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLDisplay.h"
#include "Rect.h"
#include "Ratio.h"

#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#include <Cocoa/Cocoa.h>

namespace Sexy
{

class SexyAppBase;
class Image;
class MemoryImage;

class AGLDisplay : public GLDisplay
{
public:
	AGLDisplay(SexyAppBase* theApp);
	virtual ~AGLDisplay();

	virtual int			        Init();
	virtual void			        Cleanup();
	virtual bool                            CanWindowed();

        virtual void                            SwapBuffers();

	virtual void			        RemapMouse(int& theX, int& theY);

        virtual Image*                          CreateImage(SexyAppBase * theApp,
                                                            int width, int height);
        virtual bool                            HasEvent();
        virtual bool                            GetEvent(struct Event &event);

private:
	NSWindow*                               mWindow;
        CGLContextObj                           mCGLContext;
	NSOpenGLContext*                        mContext;
        int                                     mAGLMajor;
        int                                     mAGLMinor;
	bool                                    mScreenCapture;

	std::map<int, int>                      mKeyMap;

	bool                                    mCursorHide;

private:
	void                                    InitKeyMap();
	int                                     KeyCodeFromNSKeyCode(int NSKeyCode);

};

}

#endif //__GLINTERFACE_H__

