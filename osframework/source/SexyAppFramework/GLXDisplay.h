#ifndef __GLXINTERFACE_H__
#define __GLXINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLDisplay.h"
#include "Rect.h"
#include "Ratio.h"
#include "SexyMatrix.h"

#include <GL/glx.h>
#include <X11/Xlib.h>

namespace Sexy
{

class SexyAppBase;
class Image;
class MemoryImage;

class GLXDisplay : public GLDisplay
{
public:
	GLXDisplay(SexyAppBase* theApp);
	virtual ~GLXDisplay();

	virtual int			        Init();
	virtual bool                            CanReinit();
	virtual bool                            Reinit();
	virtual void			        Cleanup();
	virtual bool                            CanWindowed();

        virtual void                            SwapBuffers();

	virtual void			        RemapMouse(int& theX, int& theY);

        virtual Image*                          CreateImage(SexyAppBase * theApp,
                                                            int width, int height);
        virtual bool                            HasEvent();
        virtual bool                            GetEvent(struct Event &event);
	virtual bool                            GetInputInfo(InputInfo &anInfo);

private:
        Display*                                mDpy;
        Window                                  mWindow;
        GLXContext                              mContext;
        int                                     mGLXMajor;
        int                                     mGLXMinor;

	int                                     mScreen;

	Atom                                    mWMDeleteMessage;

	bool                                    mIsWindowed;
	static Bool                             WaitForSubstructureNotify(Display *d,
									  XEvent *e, char* arg);
};

}

#endif //__GLINTERFACE_H__

