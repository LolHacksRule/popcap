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

	virtual bool                            ShowKeyBoard(Widget* theWidget);
	virtual void                            HideKeyBoard();

public:
	UIWindow*                               mWindow;
	UIView*                                 mView;
	UITextField*                            mTextField;
	bool                                    mKeyboardVisible;
	EAGLContext*                            mContext;
        int                                     mEAGLMajor;
        int                                     mEAGLMinor;
};

}

#endif //__GLINTERFACE_H__

