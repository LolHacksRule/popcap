#include "DFBDisplay.h"
#include "DFBImage.h"
#include "SexyAppBase.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"

#include <cstdio>

using namespace Sexy;

DFBDisplay::DFBDisplay(SexyAppBase* theApp)
{
	mApp = theApp;
	mDFB = NULL;
	mPrimarySurface = NULL;
	mBackSurface = NULL;
	mScreenImage = NULL;
	mInitialized = false;
	mVideoOnlyDraw = false;

	mInRedraw = false;
	mInitCount = 0;

	mInput = NULL;
	mBuffer = NULL;
	mMouseX = 0;
	mMouseY = 0;
	mCursorX = 0;
	mCursorY = 0;
	mCursorOldX = 0;
	mCursorOldY = 0;
	mCursorHotX = 0;
	mCursorHotY = 0;
	mLayer = NULL;
	mWindow = NULL;
	mCursorImage = NULL;
	mSoftCursor = false;
	mDFBCursorImage = NULL;
}

DFBDisplay::~DFBDisplay()
{
	Cleanup();

	if (mDFB)
		mDFB->Release(mDFB);
}

Image* DFBDisplay::GetScreenImage()
{
	return mScreenImage;
}

int DFBDisplay::Init(void)
{
	Cleanup();

	DFBResult ret;
	mInitialized = false;

	if (!mDFB)
	{
		DirectFBInit(NULL, NULL);
		ret = DirectFBCreate(&mDFB);
		if (ret != DFB_OK)
			return -1;
		//mDFB->SetCooperativeLevel(mDFB, DFSCL_FULLSCREEN);
		//mDFB->SetCooperativeLevel(mDFB, DFSCL_EXCLUSIVE);
	}

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;

	ret = mDFB->GetDisplayLayer (mDFB, DLID_PRIMARY, &mLayer);
	if (ret != DFB_OK)
		return -1;

	DFBDisplayLayerConfig layer_config;
	mLayer->GetConfiguration (mLayer, &layer_config);

	DFBDisplayLayerDescription layer_desc;
	mLayer->GetDescription (mLayer, &layer_desc);
	printf ("DFB: Display layer: %s[%dx%d]\n",
		layer_desc.name, layer_config.width, layer_config.height);

	ret = mLayer->SetCooperativeLevel (mLayer, DLSCL_ADMINISTRATIVE);

	IDirectFBSurface * surface = 0;
	bool preferWindow = true;

#if defined(SEXY_INTEL_CANMORE) || defined(SEXY_INTEL_OLO) || defined(SEXY_ST_SH4)
	preferWindow = false;
#endif
	preferWindow = GetEnvOption ("SEXY_DFB_PREFER_WINDOW", preferWindow);

	mIsWindowed = mApp->mIsWindowed;
	if (preferWindow)
	{
		printf("DFB: Creating an IDirectFBWindow...\n");

		DFBWindowDescription window_desc;

		window_desc.flags =
			(DFBWindowDescriptionFlags)(DWDESC_CAPS | DWDESC_WIDTH |  DWDESC_HEIGHT |
						    DWDESC_POSX | DWDESC_POSY);
		window_desc.caps = (DFBWindowCapabilities)(DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER |
							   DWCAPS_NODECORATION);
		if (mIsWindowed)
		{
			window_desc.width = mWidth;
			window_desc.height = mHeight;
			window_desc.posx = (layer_config.width - mWidth) / 2;
			window_desc.posy = (layer_config.height - mHeight) / 2;
		}
		else
		{
			window_desc.width = layer_config.width;
			window_desc.height = layer_config.height;
			window_desc.posx = 0;
			window_desc.posy = 0;
		}
		printf("DFB: Window geometry: %dx%d@%dx%d\n",
		       window_desc.width, window_desc.height,
		       window_desc.posx, window_desc.posy);
		ret = mLayer->CreateWindow (mLayer, &window_desc, &mWindow);
		if (ret != DFB_OK)
			return -1;
		mWindow->SetOpacity (mWindow, 255);
		mWindow->RaiseToTop (mWindow);

		ret = mWindow->GetSurface (mWindow, &surface);
		if (ret != DFB_OK)
			return -1;
	}
	else
	{
		mLayer->EnableCursor (mLayer, DFB_FALSE);
		mSoftCursor = true;
	}

	if (!surface)
		ret = mLayer->GetSurface (mLayer, &surface);

	if (!surface)
		return -1;

	int width, height;

	surface->GetSize (surface, &width, &height);

	IDirectFBSurface *backSurface = 0;
	DFBSurfacePixelFormat format;
	if (surface->GetPixelFormat(surface, &format) != DFB_OK ||
	    format != DSPF_ARGB || width != mWidth || height != mHeight)
	{
		DFBSurfaceDescription desc;

		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH |
							  DSDESC_HEIGHT |
							  DSDESC_PIXELFORMAT);
		desc.width = mWidth;
		desc.height = mHeight;
		desc.pixelformat = DSPF_ARGB;
		ret = mDFB->CreateSurface(mDFB, &desc, &backSurface);
		if (ret != DFB_OK)
			return -1;
		printf("DFB: Created a back surface(%dx%d).\n", mWidth, mHeight);
	}

	for (int i = 0; i < 3; i++)
	{
		surface->Clear (surface, 0, 0, 0, 0);
		surface->Flip (surface, 0,
			       (DFBSurfaceFlipFlags)(DSFLIP_BLIT));
	}

	IDirectFBSurface *drawSurface = backSurface ? backSurface : surface;

	int drawWidth, drawHeight;
	drawSurface->GetSize (drawSurface, &drawWidth, &drawHeight);

	mPrimarySurface = surface;
	mBackSurface = backSurface;
	mWidth = drawWidth;
	mHeight = drawHeight;
	mDisplayWidth = drawWidth;
	mDisplayHeight = drawHeight;
	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mDisplayWidth;
	mPresentationRect.mHeight = mDisplayHeight;

	mViewport.x = 0;
	mViewport.y = 0;
	mViewport.w = width;
	mViewport.h = height;

	drawSurface->AddRef(drawSurface);
	mScreenImage = new DFBImage(drawSurface, this);
	mScreenImage->mFlags = (ImageFlags)IMAGE_FLAGS_DOUBLE_BUFFER;

	if (mWindow)
		ret = mWindow->CreateEventBuffer (mWindow, &mBuffer);
	else
		ret = mDFB->CreateInputEventBuffer (mDFB, DICAPS_ALL, DFB_TRUE,
						    &mBuffer);

	mInitCount++;
	mInitialized = true;

	return 0;
}

void DFBDisplay::SetVideoOnlyDraw(bool videoOnlyDraw)
{
	mVideoOnlyDraw = videoOnlyDraw;
}

void DFBDisplay::UnmapMouse(int& theX, int& theY)
{
	if (mInitialized && mBackSurface)
	{
		theX = theX * mViewport.w / mWidth + mViewport.x;
		theY = theY * mViewport.h / mHeight + mViewport.y;
	}
}

void DFBDisplay::RemapMouse(int& theX, int& theY)
{
	if (mInitialized && mBackSurface)
	{
		theX = (theX - mViewport.x) * mWidth / mViewport.w;
		theY = (theY - mViewport.y) * mHeight / mViewport.h;
	}
}

ulong DFBDisplay::GetColorRef(ulong theRGB)
{
	return theRGB;
}

void DFBDisplay::AddImage(Image* theImage)
{
	mImageSet.insert((DFBImage*)theImage);
}

void DFBDisplay::RemoveImage(Image* theImage)
{
	DFBImageSet::iterator anItr = mImageSet.find((DFBImage*)theImage);
	if (anItr != mImageSet.end())
		mImageSet.erase(anItr);
}

void DFBDisplay::Cleanup()
{
	FlushWork();
	mInitialized = false;

	if (mInput)
		mInput->Release(mInput);

	if (mBuffer)
		mBuffer->Release(mBuffer);

	if (mDFBCursorImage)
		mDFBCursorImage->Release(mDFBCursorImage);
	mDFBCursorImage = 0;
	mCursorImage = NULL;

	if (mScreenImage != NULL)
	{
		delete mScreenImage;
		mScreenImage = NULL;
	}

	if (mBackSurface)
		mBackSurface->Release (mBackSurface);
	mBackSurface = NULL;

	if (mWindow)
		mWindow->Release (mWindow);
	mWindow = NULL;

	if (mLayer)
		mLayer->Release (mLayer);
	mLayer = NULL;

	if (mPrimarySurface)
		mPrimarySurface->Release(mPrimarySurface);
	mPrimarySurface = NULL;

	mCursorX = 0;
	mCursorY = 0;
	mCursorOldX = 0;
	mCursorOldY = 0;
	mCursorHotX = 0;
	mCursorHotY = 0;
	mSoftCursor = false;
}

bool DFBDisplay::Redraw(Rect* theClipRect)
{
	if (!mInitialized)
		return false;

	SwapBuffers();
	return true;
}

void DFBDisplay::SwapBuffers()
{
	if (!mPrimarySurface)
		return;

	if (mBackSurface)
	{
		DFBSurfaceBlittingFlags flags = DSBLIT_NOFX;

		mPrimarySurface->SetBlittingFlags(mPrimarySurface, flags);
		mPrimarySurface->SetColor(mPrimarySurface, 0xff, 0xff, 0xff, 0xff);
		mPrimarySurface->StretchBlit(mPrimarySurface, mBackSurface, 0, &mViewport);
	}

	mPrimarySurface->Flip (mPrimarySurface, NULL, DSFLIP_WAIT);
}

bool DFBDisplay::EnableCursor(bool enable)
{
	mCursorEnabled = enable;
	if (mWindow)
	{
		mWindow->SetCursorShape (mWindow, NULL, 0, 0);
		return true;
	}
	else
	{
		if (!mSoftCursor && mLayer)
		{
			mLayer->EnableCursor (mLayer, enable);
			return true;
		}
	}

	return false;
}

bool DFBDisplay::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	if (mDFBCursorImage)
		mDFBCursorImage->Release(mDFBCursorImage);
	mDFBCursorImage = NULL;
	mCursorImage = NULL;

	MemoryImage * anImage = dynamic_cast<MemoryImage*>(theImage);
	if (anImage)
		mDFBCursorImage = DFBImage::EnsureSrcSurface(this, theImage);
	if (mDFBCursorImage)
		mDFBCursorImage->AddRef(mDFBCursorImage);
	mCursorImage = anImage;
	mCursorHotX = theHotX;
	mCursorHotY = theHotY;

	if (mWindow)
		mWindow->SetCursorShape (mWindow, mDFBCursorImage,
					 mCursorHotX, mCursorHotY);

	return true;
}

void DFBDisplay::SetCursorPos(int theCursorX, int theCursorY)
{
	if (mCursorX == theCursorX && mCursorY == theCursorY)
		return;

	mCursorX = theCursorX;
	mCursorY = theCursorY;
	if (!mSoftCursor && mLayer)
	{
		if (mWindow)
		{
			int x, y;

			mWindow->GetPosition (mWindow, &x, &y);
			theCursorX += x;
			theCursorY += y;
		}

		// convert back to window coords
		UnmapMouse(theCursorX, theCursorY);
		mLayer->WarpCursor (mLayer, theCursorX, theCursorY);
	}
}

bool DFBDisplay::UpdateCursor(int theCursorX, int theCursorY)
{
	SetCursorPos (theCursorX, theCursorY);
	if (mSoftCursor && mCursorImage &&
	    (mCursorOldX != mCursorX || mCursorOldY != mCursorY))
		return true;
	return false;
}

bool DFBDisplay::DrawCursor(Graphics* g)
{
	if (!mCursorImage)
		return false;

	if (mSoftCursor)
		g->DrawImage (mCursorImage,
			      mCursorX - mCursorHotX,
			      mCursorY - mCursorHotY);

	mCursorOldX = mCursorX;
	mCursorOldY = mCursorY;
	return true;
}

IDirectFBSurface* DFBDisplay::CreateDFBSurface(int width, int height)
{
	IDirectFBSurface * aSurface;

	if (!mDFB)
		return 0;

	if (!width || !height)
		return 0;

	DFBSurfaceDescription desc;

	desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH |
						  DSDESC_HEIGHT |
						  DSDESC_PIXELFORMAT |
						  DSDESC_CAPS);
	desc.width = width;
	desc.height = height;
	desc.pixelformat = DSPF_ARGB;
	desc.caps = DSCAPS_PREMULTIPLIED;
	if (mDFB->CreateSurface(mDFB, &desc, &aSurface))
		return 0;
	return aSurface;
}

Image* DFBDisplay::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	IDirectFBSurface * aSurface;
	DFBImage * aDFBImage;

	if (!mInitialized)
	{
		return 0;
	}
	else if (width && height)
	{
		DFBSurfaceDescription desc;

		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH |
							  DSDESC_HEIGHT |
							  DSDESC_PIXELFORMAT);
		desc.width = width;
		desc.height = height;
		desc.pixelformat = DSPF_ARGB;
		if (mDFB->CreateSurface(mDFB, &desc, &aSurface))
			return 0;
		aDFBImage = new DFBImage(aSurface, this);
	} else {
		aDFBImage = new DFBImage(this);
	}

	if (!aDFBImage)
		return 0;

	aDFBImage->Create(width, height);

	return aDFBImage;
}

bool DFBDisplay::HasEvent()
{
	if (!mBuffer)
		return false;

	return	mBuffer->HasEvent(mBuffer) == DFB_OK;
}

bool DFBDisplay::GetEvent(struct Event &event)
{
	if (!HasEvent())
		return false;

	DFBEvent dfb_event;
	if (mBuffer->GetEvent(mBuffer, &dfb_event) != DFB_OK)
		return false;

	event.type = EVENT_NONE;
	event.flags = 0;
	//printf ("clazz %d\n", (int)dfb_event.clazz);
	switch (dfb_event.clazz) {
	case DFEC_INPUT:
		DFBInputEvent * e;

		e = &dfb_event.input;
		//printf ("type %d\n", (int)e->type);
		switch (e->type) {
		case DIET_BUTTONPRESS:
			event.u.mouse.x = mMouseX;
			event.u.mouse.y = mMouseY;
			event.type = EVENT_MOUSE_BUTTON_PRESS;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.u.mouse.button = 1;
				break;
			case DIBI_MIDDLE:
				event.u.mouse.button = 3;
				break;
			case DIBI_RIGHT:
				event.u.mouse.button = 2;
				break;
			default:
				break;
			}
			break;
		case DIET_BUTTONRELEASE:
			event.u.mouse.x = mMouseX;
			event.u.mouse.y = mMouseY;
			event.type = EVENT_MOUSE_BUTTON_RELEASE;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.u.mouse.button = 1;
				break;
			case DIBI_MIDDLE:
				event.u.mouse.button = 3;
				break;
			case DIBI_RIGHT:
				event.u.mouse.button = 2;
				break;
			default:
				break;
			}
			break;
		case DIET_AXISMOTION:
			switch (e->axis) {
			case DIAI_X:
				if (e->flags & DIEF_AXISABS)
					mMouseX = e->axisabs;
				else if (e->flags & DIEF_AXISREL)
					mMouseX += e->axisrel;
				break;
			case DIAI_Y:
				if (e->flags & DIEF_AXISABS)
					mMouseY = e->axisabs;
				else if (e->flags & DIEF_AXISREL)
					mMouseY += e->axisrel;
				break;
			default:
				break;
			}

			if (!(e->flags & DIEF_FOLLOW)) {
				event.u.mouse.x = mMouseX;
				event.u.mouse.y = mMouseY;
				event.type = EVENT_MOUSE_MOTION;
				event.flags = EVENT_FLAGS_AXIS;
			}
			break;
		case DIET_KEYPRESS:
		case DIET_KEYRELEASE:
		{
			const DFBInputDeviceKeyIdentifier id = e->key_id;
			const DFBInputDeviceKeySymbol sym = e->key_symbol;

			//printf ("id = %d symbol = %d\n", id, sym);
			if (e->type == DIET_KEYPRESS)
				event.type = EVENT_KEY_DOWN;
			else
				event.type = EVENT_KEY_UP;
			event.flags = EVENT_FLAGS_KEY_CODE;
			if (id == DIKI_UP) {
				event.u.key.keyCode = (int)KEYCODE_UP;
			} else if(id ==	 DIKI_DOWN) {
				event.u.key.keyCode = (int)KEYCODE_DOWN;
			} else if (id ==  DIKI_LEFT) {
				event.u.key.keyCode = (int)KEYCODE_LEFT;
			} else if (id == DIKI_RIGHT) {
				event.u.key.keyCode = (int)KEYCODE_RIGHT;
			} else if (id == DIKI_ENTER) {
				event.u.key.keyCode = (int)KEYCODE_RETURN;
			} else if (id == DIKI_SPACE) {
				event.u.key.keyCode = (int)KEYCODE_SPACE;
			} else if (id == DIKI_BACKSPACE) {
				event.u.key.keyCode = (int)KEYCODE_BACK;
			} else if (id == DIKI_ESCAPE) {
				if ((e->modifiers & (DIMM_SHIFT | DIMM_CONTROL)) == (DIMM_SHIFT | DIMM_CONTROL))
					event.type = EVENT_QUIT;
				else
					event.u.key.keyCode = (int)KEYCODE_ESCAPE;
			} else if (id == DIKI_SHIFT_L || id == DIKI_SHIFT_R) {
				event.u.key.keyCode = (int)KEYCODE_SHIFT;
			} else if (id == DIKI_CONTROL_L || id == DIKI_CONTROL_R) {
				event.u.key.keyCode = (int)KEYCODE_CONTROL;
			} else if (id >= DIKI_A && id < DIKI_Z) {
				event.flags |= EVENT_FLAGS_KEY_CHAR;
				event.u.key.keyCode = (int)('a' + id - DIKI_A);
				event.u.key.keyChar = (int)sym;
			} else if (id >= DIKI_0 && id < DIKI_9) {
				event.flags |= EVENT_FLAGS_KEY_CHAR;
				event.u.key.keyCode = (int)('0' + id - DIKI_0);
				event.u.key.keyChar = (int)sym;
			} else {
				event.type = EVENT_NONE;
			}
			break;
		}
		default:
			break;
		}
		break;
	case DFEC_WINDOW:
	{
		DFBWindowEvent * e;

		e = &dfb_event.window;
		event.type = EVENT_NONE;
		//printf ("type %d\n", (int)e->type);
		switch (e->type) {
		case DWET_BUTTONDOWN:
			event.u.mouse.x = mMouseX;
			event.u.mouse.y = mMouseY;

			if (mWindow)
			{
				int x, y;

				mWindow->GetPosition (mWindow, &x, &y);
				event.u.mouse.x -= x;
				event.u.mouse.y -= y;
			}

			event.type = EVENT_MOUSE_BUTTON_PRESS;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.u.mouse.button = 1;
				break;
			case DIBI_MIDDLE:
				event.u.mouse.button = 3;
				break;
			case DIBI_RIGHT:
				event.u.mouse.button = 2;
				break;
			default:
				break;
			}
			break;
		case DWET_BUTTONUP:
			event.u.mouse.x = mMouseX;
			event.u.mouse.y = mMouseY;

			if (mWindow)
			{
				int x, y;

				mWindow->GetPosition (mWindow, &x, &y);
				event.u.mouse.x -= x;
				event.u.mouse.y -= y;
			}

			event.type = EVENT_MOUSE_BUTTON_RELEASE;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.u.mouse.button = 1;
				break;
			case DIBI_MIDDLE:
				event.u.mouse.button = 3;
				break;
			case DIBI_RIGHT:
				event.u.mouse.button = 2;
				break;
			default:
				break;
			}
			break;
		case DWET_MOTION:
			mMouseX = e->cx;
			mMouseY = e->cy;
			event.u.mouse.x = mMouseX;
			event.u.mouse.y = mMouseY;

			if (mWindow)
			{
				int x, y;

				mWindow->GetPosition (mWindow, &x, &y);
				event.u.mouse.x -= x;
				event.u.mouse.y -= y;
			}
			event.type = EVENT_MOUSE_MOTION;
			event.flags = EVENT_FLAGS_AXIS;
			break;
		case DWET_KEYDOWN:
		case DWET_KEYUP:
		{
			const DFBInputDeviceKeyIdentifier id = e->key_id;
			const DFBInputDeviceKeySymbol sym = e->key_symbol;

			//printf ("id = %d symbol = %d\n", id, sym);
			if (e->type == DWET_KEYDOWN)
				event.type = EVENT_KEY_DOWN;
			else
				event.type = EVENT_KEY_UP;
			event.flags = EVENT_FLAGS_KEY_CODE;
			if (id == DIKI_UP) {
				event.u.key.keyCode = (int)KEYCODE_UP;
			} else if(id ==	 DIKI_DOWN) {
				event.u.key.keyCode = (int)KEYCODE_DOWN;
			} else if (id ==  DIKI_LEFT) {
				event.u.key.keyCode = (int)KEYCODE_LEFT;
			} else if (id == DIKI_RIGHT) {
				event.u.key.keyCode = (int)KEYCODE_RIGHT;
			} else if (id == DIKI_ENTER) {
				event.u.key.keyCode = (int)KEYCODE_RETURN;
			} else if (id == DIKI_SPACE) {
				event.u.key.keyCode = (int)KEYCODE_SPACE;
			} else if (id == DIKI_BACKSPACE) {
				event.u.key.keyCode = (int)KEYCODE_BACK;
			} else if (id == DIKI_ESCAPE) {
				if ((e->modifiers & (DIMM_SHIFT | DIMM_CONTROL)) == (DIMM_SHIFT | DIMM_CONTROL))
					event.type = EVENT_QUIT;
				else
					event.u.key.keyCode = (int)KEYCODE_ESCAPE;
			} else if (id == DIKI_SHIFT_L || id == DIKI_SHIFT_R) {
				event.u.key.keyCode = (int)KEYCODE_SHIFT;
			} else if (id == DIKI_CONTROL_L || id == DIKI_CONTROL_R) {
				event.u.key.keyCode = (int)KEYCODE_CONTROL;
			} else if (id >= DIKI_A && id < DIKI_Z) {
				event.flags |= EVENT_FLAGS_KEY_CHAR;
				event.u.key.keyCode = (int)('a' + id - DIKI_A);
				event.u.key.keyChar = (int)sym;
			} else if (id >= DIKI_0 && id < DIKI_9) {
				event.flags |= EVENT_FLAGS_KEY_CHAR;
				event.u.key.keyCode = (int)('0' + id - DIKI_0);
				event.u.key.keyChar = (int)sym;
			} else {
				event.type = EVENT_NONE;
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case DIET_UNKNOWN:
	default:
		event.type = EVENT_NONE;
		break;
	}

	if (event.type == EVENT_MOUSE_MOTION ||
	    event.type == EVENT_MOUSE_BUTTON_RELEASE ||
	    event.type == EVENT_MOUSE_BUTTON_PRESS)
		RemapMouse(event.u.mouse.x, event.u.mouse.y);

	return true;
}

bool DFBDisplay::CreateImageData(MemoryImage *theImage)
{
	return DFBImage::EnsureSrcSurface(this, theImage) != 0;
}

void DFBDisplay::RemoveImageData(MemoryImage *theImage)
{
	DFBImage::RemoveImageData(theImage);
}

namespace Sexy {
class DelayedReleaseSurfaceWork: public DelayedWork
{
public:
	DelayedReleaseSurfaceWork(IDirectFBSurface* surface) : mSurface(surface) {}

public:
	virtual void Work()
	{
		mSurface->Release(mSurface);
	}

private:
	IDirectFBSurface* mSurface;
};
}

void DFBDisplay::DelayedReleaseSurface(IDirectFBSurface* surface)
{
	if (mMainThread != Thread::Self())
		PushWork(new DelayedReleaseSurfaceWork(surface));
	else
		surface->Release(surface);
}

bool DFBDisplay::IsMainThread(void)
{
	return mMainThread == Thread::Self();
}

class DFBVideoDriver: public VideoDriver {
public:
	DFBVideoDriver ()
	 : VideoDriver("DFBDisplay", 0)
	{
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new DFBDisplay (theApp);
	}
};

static DFBVideoDriver aDFBVideoDriver;
VideoDriverRegistor aDFBVideoDriverRegistor (&aDFBVideoDriver);
VideoDriver* GetDFBVideoDriver()
{
	return &aDFBVideoDriver;
}
