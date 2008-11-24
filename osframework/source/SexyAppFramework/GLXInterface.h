#ifndef __GLXINTERFACE_H__
#define __GLXINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLInterface.h"
#include "Rect.h"
#include "Ratio.h"

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
        virtual bool                            EnableCursor(bool enable);
	virtual bool				SetCursorImage(Image* theImage, int theHotX = 0, int theHotY = 0);
	virtual void				SetCursorPos(int theCursorX, int theCursorY);

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
};

}

#endif //__GLINTERFACE_H__

