#ifndef __AGLINTERFACE_H__
#define __AGLINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLInterface.h"
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

class AGLInterface : public GLInterface
{
public:
	AGLInterface(SexyAppBase* theApp);
	virtual ~AGLInterface();

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
	NSWindow*                               mWindow;
        CGLContextObj                           mCGLContext;
	NSOpenGLContext*                        mContext;
        int                                     mAGLMajor;
        int                                     mAGLMinor;

	std::map<int, int>                      mKeyMap;
};

}

#endif //__GLINTERFACE_H__

