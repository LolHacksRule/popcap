#ifndef __GLXINTERFACE_H__
#define __GLXINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLInterface.h"
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

class GLXInterface : public GLInterface
{
public:
	GLXInterface(SexyAppBase* theApp);
	virtual ~GLXInterface();

	virtual int			        Init();
	virtual void			        Cleanup();

        virtual void                            SwapBuffers();

	virtual void			        RemapMouse(int& theX, int& theY);

        virtual Image*                          CreateImage(SexyAppBase * theApp,
                                                            int width, int height);
        virtual bool                            HasEvent();
        virtual bool                            GetEvent(struct Event &event);

private:
        Display*                                mDpy;
        Window                                  mWindow;
        GLXContext                              mContext;
        int                                     mGLXMajor;
        int                                     mGLXMinor;

	int                                     mScreen;

	Atom                                    mWMDeleteMessage;

	static Bool                             WaitForSubstructureNotify(Display *d,
									  XEvent *e, char* arg);
};

}

#endif //__GLINTERFACE_H__

