#ifndef __AGLINTERFACE_H__
#define __AGLINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLDisplay.h"
#include "Rect.h"
#include "Ratio.h"

#include <UIKit/UIKit.h>
#include <QuartzCore/QuartzCore.h>

#include <OpenGLES/EAGL.h>
#include <OpenGLES/EAGLDrawable.h>

#include "EAGLView.h"

namespace Sexy
{

class SexyAppBase;
class Image;
class MemoryImage;

class EAGLDisplay : public GLDisplay
{
public:
	EAGLDisplay(SexyAppBase* theApp);
	virtual ~EAGLDisplay();

	virtual int			        Init();
	virtual void			        Cleanup();

        virtual void                            SwapBuffers();

	virtual void			        RemapMouse(int& theX, int& theY);

        virtual Image*                          CreateImage(SexyAppBase * theApp,
                                                            int width, int height);
        virtual bool                            HasEvent();
        virtual bool                            GetEvent(struct Event &event);

private:
	UIWindow*                               mWindow;
	UIView*                                 mView;
	EAGLContext*                            mContext;
        int                                     mEAGLMajor;
        int                                     mEAGLMinor;

	std::map<int, int>                      mKeyMap;

private:
	void                                    InitKeyMap();
	int                                     KeyCodeFromNSKeyCode(int NSKeyCode);

};

}

#endif //__GLINTERFACE_H__

