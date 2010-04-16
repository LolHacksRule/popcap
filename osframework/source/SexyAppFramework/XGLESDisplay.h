#ifndef __GLXINTERFACE_H__
#define __GLXINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLDisplay.h"
#include "Rect.h"
#include "Ratio.h"
#include "SexyMatrix.h"

#include <GLES/egl.h>
#include <GLES/gl.h>

#include <X11/Xlib.h>

namespace Sexy
{

class SexyAppBase;
class Image;
class MemoryImage;

class XGLESDisplay : public GLDisplay
{
public:
	XGLESDisplay(SexyAppBase* theApp);
	virtual ~XGLESDisplay();

	virtual int			        Init();
	virtual bool                            CanReinit();
	virtual bool                            Reinit();
	virtual void			        Cleanup();

        virtual void                            SwapBuffers();

	virtual void			        RemapMouse(int& theX, int& theY);

        virtual Image*                          CreateImage(SexyAppBase * theApp,
                                                            int width, int height);
        virtual bool                            HasEvent();
        virtual bool                            GetEvent(struct Event &event);

private:
        Display*                                mNativeDpy;
        Window                                  mNativeWindow;

	int                                     mScreen;

	EGLint					mEGLMajor;
	EGLint					mEGLMinor;

	EGLDisplay				mDpy;
	EGLConfig				mConfig;
	EGLSurface				mSurface;
	EGLContext				mContext;

	Atom                                    mWMDeleteMessage;

	static Bool                             WaitForSubstructureNotify(Display *d,
									  XEvent *e, char* arg);
};

}

#endif //__GLINTERFACE_H__

