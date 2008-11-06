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
	mDpy = NULL;
	mWindow = NULL;
	mContext = EGL_NO_CONTEXT;
	mSurface = EGL_NO_SURFACE;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mCursorHotX = 0;
	mCursorHotY = 0;
	mCursorX = 0;
	mCursorY = 0;
	mCursorOldX = 0;
	mCursorOldY = 0;
	mCursorEnabled = false;
	mCursorDrawn = false;
}

CEGLESInterface::~CEGLESInterface ()
{
	Cleanup();
}

#ifdef SEXY_INTEL_CANMORE
#include <libgdl.h>

static void init_gdl_plane (void)
{
	gdl_pixel_format_t pix_fmt = GDL_PF_ARGB_32;
	gdl_color_space_t color_space = GDL_COLOR_SPACE_RGB;
	gdl_display_info_t  display;
	gdl_rectangle_t src_rect;
	gdl_rectangle_t dst_rect;

	gdl_init (0);

	gdl_get_display_info (GDL_DISPLAY_ID_0, &display);
	dst_rect.origin.x = 0;
	dst_rect.origin.y = 0;
	dst_rect.width = display.tvmode.width;
	dst_rect.height = display.tvmode.height;

	src_rect.origin.x = 0;
	src_rect.origin.y = 0;
	src_rect.width = display.tvmode.width;
	src_rect.height = display.tvmode.height;

	gdl_plane_config_begin (GDL_PLANE_ID_UPP_C);
	gdl_plane_set_attr (GDL_PLANE_SRC_COLOR_SPACE, &color_space);
	gdl_plane_set_attr (GDL_PLANE_PIXEL_FORMAT, &pix_fmt);
	gdl_plane_set_attr (GDL_PLANE_DST_RECT, &dst_rect);
	gdl_plane_set_attr (GDL_PLANE_SRC_RECT, &src_rect);
	gdl_plane_config_end (GDL_FALSE);
}
#endif

int CEGLESInterface::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLInterface::Init();

	EGLBoolean ret;

#ifdef SEXY_INTEL_CANMORE
	init_gdl_plane ();

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

	mWidth = width;
	mHeight = height;
	mApp->mWidth = width;
	mApp->mHeight = height;

#if 0
	EGLint SwapBehavior;
	eglQuerySurface (mDpy, mSurface, EGL_SWAP_BEHAVIOR, &SwapBehavior);
	printf ("swap behavior: %s\n",
		SwapBehavior == EGL_BUFFER_PRESERVED ? "preserved" : "destroyed");

	eglSurfaceAttrib (mDpy, mSurface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
#endif

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

#if defined(EGL1_1) || defined(EGL1_2)
	glSwapInterval (mDpy, 1);
#endif

	mCursorDrawn = false;
	glGenTextures (1, &mOldCursorTex);

	static unsigned char bits[64 * 64 * 4] = { 0 };

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture (GL_TEXTURE_2D, mOldCursorTex);
	glTexImage2D (GL_TEXTURE_2D,
		      0,
		      GL_RGBA,
		      64, 64,
		      0,
		      GL_RGBA,
		      GL_UNSIGNED_BYTE,
		      bits);

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
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLInterface::Cleanup ();

	mCursorImage = 0;
	glDeleteTextures (1, &mOldCursorTex);

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

	mCursorX = 0;
	mCursorY = 0;
	mCursorOldX = 0;
	mCursorOldY = 0;
}

void CEGLESInterface::RemapMouse(int& theX, int& theY)
{
}

bool CEGLESInterface::EnableCursor(bool enable)
{
	mCursorEnabled = enable;
	return true;
}

bool CEGLESInterface::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	GLImage * aGLImage = dynamic_cast<GLImage*>(theImage);
	mCursorImage = aGLImage;
	mCursorHotX = theHotX;
	mCursorHotY = theHotY;
	return false;
}

void CEGLESInterface::SetCursorPos(int theCursorX, int theCursorY)
{
        mCursorOldX = mCursorX;
        mCursorOldY = mCursorY;
	mCursorX = theCursorX;
	mCursorY = theCursorY;
}

bool CEGLESInterface::UpdateCursor(int theCursorX, int theCursorY)
{
	SetCursorPos (theCursorX, theCursorY);
	if (mCursorImage &&
	    (mCursorOldX != mCursorX ||
	     mCursorOldY != mCursorY))
		return true;
	return false;
}

bool CEGLESInterface::DrawCursor(Graphics* g)
{
	if (!mCursorImage)
		return false;

	if (0)
	{
		glBindTexture (GL_TEXTURE_2D, mOldCursorTex);
		glCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, mCursorX, mCursorY, 64, 64);
	}

	g->DrawImage (mCursorImage,
		      mCursorX - mCursorHotX,
		      mCursorY - mCursorHotY);

	mCursorDrawnX = mCursorX;
	mCursorDrawnY = mCursorY;
	return true;
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
	{
		eglSwapBuffers (mDpy, mSurface);

		if (0 && mCursorDrawn)
		{
			glColor4ub (255, 255, 255, 255);

			glBindTexture (GL_TEXTURE_2D, mOldCursorTex);

			GLfloat verts[4 * 2];
			verts[0] = 0;  verts[1] = 0;
			verts[2] = 0;  verts[3] = 64;
			verts[4] = 64; verts[5] = 0;
			verts[6] = 64; verts[7] = 64;

			GLfloat coords[4 * 2];
			coords[0] = 0.0f; coords[1] = 0.0f;
			coords[2] = 0.0f; coords[3] = 1.0f;
			coords[4] = 1.0f; coords[5] = 0.0f;
			coords[6] = 1.0f; coords[7] = 1.0f;

			glEnableClientState (GL_VERTEX_ARRAY);
			glEnableClientState (GL_TEXTURE_COORD_ARRAY);

			glVertexPointer (2, GL_FLOAT, 0, verts);
			glTexCoordPointer (2, GL_FLOAT, 0, coords);
			glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

			glDisableClientState (GL_VERTEX_ARRAY);
			glDisableClientState (GL_TEXTURE_COORD_ARRAY);

			mCursorDrawn = false;
		}
	}
}

class CEGLESVideoDriver: public VideoDriver {
public:
	CEGLESVideoDriver ()
	 : VideoDriver("CEGLESInterface", 10)
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
