#ifndef __CEGLESINTERFACE_H__
#define __CEGLESINTERFACE_H__

#include "Common.h"
#include "CritSect.h"
#include "GLInterface.h"
#include "Rect.h"
#include "Ratio.h"

#include <GLES/egl.h>
#include <GLES/gl.h>

#include <libgdl.h>

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
	virtual bool				CanReinit();
	virtual bool				Reinit();
	virtual void				Cleanup();

	virtual void				SwapBuffers();

	virtual void				RemapMouse(int& theX, int& theY);

	virtual Image*				CreateImage(SexyAppBase * theApp,
							    int width, int height);
	virtual bool				HasEvent();
	virtual bool				GetEvent(struct Event &event);

private:
	EGLint					mEGLMajor;
	EGLint					mEGLMinor;

	EGLDisplay				mDpy;
	EGLConfig				mConfig;
	EGLSurface				mSurface;
	EGLContext				mContext;
	NativeWindowType			mWindow;
};

}

#endif //__GLINTERFACE_H__

