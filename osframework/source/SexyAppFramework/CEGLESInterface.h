#ifndef __CEGLESINTERFACE_H__
#define __CEGLESINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLInterface.h"
#include "Rect.h"
#include "Ratio.h"

#include <GLES/egl.h>
#include <GLES/gl.h>

namespace Sexy
{

class SexyAppBase;
class Image;
class MemoryImage;

class CEGLESInterface : public GLInterface
{
public:
	CEGLESInterface(SexyAppBase* theApp);
	virtual ~CEGLESInterface();

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

        virtual bool                            UpdateCursor(int theCursorX, int theCursorY);
        virtual bool                            DrawCursor(Graphics* g);

private:
	EGLint					mEGLMajor;
	EGLint					mEGLMinor;

	EGLDisplay				mDpy;
	EGLConfig				mConfig;
	EGLSurface				mSurface;
	EGLContext				mContext;
	NativeWindowType			mWindow;

        bool                                    mCursorEnabled;
        int                                     mCursorX;
        int                                     mCursorY;
        int                                     mCursorDrawnX;
        int                                     mCursorDrawnY;
        int                                     mCursorOldX;
        int                                     mCursorOldY;
        int                                     mCursorHotX;
        int                                     mCursorHotY;

        GLImage*                                mCursorImage;
        bool                                    mCursorDrawn;
        GLuint                                  mOldCursorTex;
};

}

#endif //__GLINTERFACE_H__
