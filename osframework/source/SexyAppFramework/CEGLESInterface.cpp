#include "SexyAppBase.h"
#include "CEGLESInterface.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"

#include <cstdio>

using namespace Sexy;

CEGLESInterface::CEGLESInterface (SexyAppBase* theApp)
	: GLInterface (theApp)
{
	mOverScan = 0.9;
	mDpy = NULL;
	mWindow = NULL;
	mContext = EGL_NO_CONTEXT;
	mSurface = EGL_NO_SURFACE;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
}

CEGLESInterface::~CEGLESInterface ()
{
	Cleanup();
}

static void init_gdl_plane ()
{
#ifdef SEXY_INTEL_CANMORE
	gdl_pixel_format_t pix_fmt = GDL_PF_ARGB_32;
	gdl_color_space_t color_space = GDL_COLOR_SPACE_RGB;
	gdl_display_info_t  display;
	gdl_rectangle_t src_rect;
	gdl_rectangle_t dst_rect;
	gdl_boolean_t upscale;

	gdl_init (0);

	gdl_get_display_info (GDL_DISPLAY_ID_0, &display);
	printf ("Display size: %dx%d(%dx%d)\n", width, height,
		display.tvmode.width, display.tvmode.height);

	dst_rect.origin.x = 0;
	dst_rect.origin.y = 0;
	dst_rect.width = display.tvmode.width;
	dst_rect.height = display.tvmode.height;

	src_rect.origin.x = 0;
	src_rect.origin.y = 0;
	src_rect.width = display.tvmode.width;
	src_rect.height = display.tvmode.height;

	upscale = GDL_TRUE;

	gdl_plane_config_begin (GDL_PLANE_ID_UPP_C);
	gdl_plane_set_attr (GDL_PLANE_SRC_COLOR_SPACE, &color_space);
	gdl_plane_set_attr (GDL_PLANE_PIXEL_FORMAT, &pix_fmt);
	gdl_plane_set_attr (GDL_PLANE_DST_RECT, &dst_rect);
	gdl_plane_set_attr (GDL_PLANE_SRC_RECT, &src_rect);
	gdl_plane_set_attr (GDL_PLANE_UPSCALE, &upscale);
	gdl_plane_config_end (GDL_FALSE);
#elif defined(SEXY_INTEL_OLO)
#endif
}

int CEGLESInterface::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLInterface::Init();

	EGLBoolean ret;

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	if (mApp->mIsWindowed)
		mOverScan = 1.0f;
	else
		mOverScan = 0.9f;

	init_gdl_plane ();

#ifdef SEXY_INTEL_CANMORE
	mDpy = eglGetDisplay ((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
#else
	mDpy = eglGetDisplay (EGL_DEFAULT_DISPLAY);
#endif
	ret = eglInitialize (mDpy, &mEGLMajor, &mEGLMinor);
	if (ret != EGL_TRUE)
	{
		printf ( "eglInitialize failed!\n");
		goto fail;
	}

	printf ("OpenGL/ES %d.%d\n", mEGLMajor, mEGLMinor);


	static EGLint attributes[] =
	{
		EGL_BUFFER_SIZE,    24,//EGL_DONT_CARE,
		//EGL_RED_SIZE,	      8,
		//EGL_GREEN_SIZE,     8,
		//EGL_BLUE_SIZE,      8,
		EGL_DEPTH_SIZE,     32,
		EGL_NONE
	};

	EGLConfig configs[2];
	EGLint config_count;

	ret = eglGetConfigs (mDpy, configs, 2, &config_count);
	if (ret != EGL_TRUE)
	{
		printf ("eglGetConfigs failed.\n");
		goto terminate;
	}

	ret = eglChooseConfig (mDpy, attributes, configs,
			       2, &config_count);
	if (ret != EGL_TRUE)
	{
		printf ("eglChooseConfig failed.\n");
		goto terminate;
	}

#ifdef SEXY_INTEL_CANMORE
	mSurface = eglCreateWindowSurface (mDpy, configs[0],
					   (EGLSurface)GDL_PLANE_ID_UPP_C, NULL);
#else
	mSurface = eglCreateWindowSurface (mDpy, configs[0], NULL, NULL);
#endif
	if (mSurface == EGL_NO_SURFACE)
	{
		printf ( "eglCreateWindowSurface failed.\n");
		goto terminate;
	}

	EGLint width, height;
	if (!eglQuerySurface (mDpy, mSurface, EGL_WIDTH, &width) ||
	    !eglQuerySurface (mDpy, mSurface, EGL_HEIGHT, &height))
	{
		printf ("eglQuerySurface failed\n");
		goto destroy_surface;
	}

	printf ("surface size: %dx%d\n", width, height);
	mWindowWidth = width;
	mWindowHeight = height;
	if (mApp->mIsWindowed)
	{
		mWidth = width;
		mHeight = height;
		mDisplayWidth = width;
		mDisplayHeight = height;
	}
	else
	{
		mWidth = mApp->mWidth;
		mHeight = mApp->mHeight;
		mDisplayWidth = mApp->mWidth;
		mDisplayHeight = mApp->mHeight;
	}
	mPresentationRect = Rect(0, 0, mDisplayWidth, mDisplayHeight);

	mContext = eglCreateContext (mDpy, configs[0], EGL_NO_CONTEXT, NULL);
	if (mContext == EGL_NO_CONTEXT)
	{
		printf ("eglCreateContext failed.\n");
		goto destroy_surface;
	}

	ret = eglMakeCurrent (mDpy, mSurface, mSurface, mContext);
	if (ret != EGL_TRUE)
	{
		printf ("eglMakeCurrent failed");
		goto destroy_context;
	}

	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	InitGL ();

	mInitCount++;
	mInitialized = true;

	return 0;
 destroy_context:
	eglDestroyContext (mDpy, mContext);
	mContext = EGL_NO_CONTEXT;
 destroy_surface:
	eglDestroySurface (mDpy, mSurface);
	mSurface = EGL_NO_SURFACE;
 terminate:
	eglTerminate (mDpy);
	mDpy = NULL;
 fail:
	return -1;
}

void CEGLESInterface::Cleanup ()
{
	FlushWork();

	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLInterface::Cleanup ();

	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = NULL;

	if (mDpy)
		eglMakeCurrent (mDpy,
				EGL_NO_SURFACE, EGL_NO_SURFACE,
				EGL_NO_CONTEXT);

	if (mContext)
	    eglDestroyContext (mDpy, mContext);
	mContext = EGL_NO_CONTEXT;

	if (mSurface)
		eglDestroySurface (mDpy, mSurface);
	mSurface = EGL_NO_SURFACE;

	if (mDpy)
		eglTerminate (mDpy);
	mDpy = NULL;
}

bool CEGLESInterface::CanReinit (void)
{
	return mInitialized;
}

bool CEGLESInterface::Reinit (void)
{
	if (mApp->mIsWindowed)
		mOverScan = 1.0f;
	else
		mOverScan = 0.9f;

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mIs3D = mApp->mIs3D;

	if (mApp->mIsWindowed)
	{
		mWidth = mWindowWidth;
		mHeight = mWindowHeight;
		mDisplayWidth = mWindowWidth;
		mDisplayHeight = mWindowHeight;
	}
	else
	{
		mWidth = mApp->mWidth;
		mHeight = mApp->mHeight;
		mDisplayWidth = mApp->mWidth;
		mDisplayHeight = mApp->mHeight;
	}
	mPresentationRect = Rect(0, 0, mDisplayWidth, mDisplayHeight);

	delete mScreenImage;
	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	return GLInterface::Reinit();
}

void CEGLESInterface::RemapMouse(int& theX, int& theY)
{
}

Image* CEGLESInterface::CreateImage(SexyAppBase * theApp,
				     int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

bool CEGLESInterface::HasEvent()
{
	return false;
}

bool CEGLESInterface::GetEvent(struct Event &event)
{
	return false;
}

void CEGLESInterface::SwapBuffers()
{
	if (mSurface)
		eglSwapBuffers (mDpy, mSurface);
}

class CEGLESVideoDriver: public VideoDriver {
public:
	CEGLESVideoDriver ()
	 : VideoDriver("IntelCE", 10)
	{
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new CEGLESInterface (theApp);
	}
};

static CEGLESVideoDriver aCEGLESVideoDriver;
VideoDriverRegistor aCEGLESVideoDriverRegistor (&aCEGLESVideoDriver);
VideoDriver* GetCEGLESVideoDriver()
{
	return &aCEGLESVideoDriver;
}
