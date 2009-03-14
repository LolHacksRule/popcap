#include "DFBInterface.h"
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

DFBInterface::DFBInterface(SexyAppBase* theApp)
{
	mApp = theApp;
	mDFB = NULL;
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
}

DFBInterface::~DFBInterface()
{
	Cleanup();

	if (mDFB)
		mDFB->Release(mDFB);
}

Image* DFBInterface::GetScreenImage()
{
	return mScreenImage;
}

int DFBInterface::Init(void)
{
	Cleanup();

	DFBResult ret;
	AutoCrit anAutoCrit(mCritSect);
	mInitialized = false;

	if (!mDFB) {
		DirectFBInit(NULL, NULL);
		ret = DirectFBCreate(&mDFB);
		DBG_ASSERT(ret == DFB_OK);
		//mDFB->SetCooperativeLevel(mDFB, DFSCL_FULLSCREEN);
		//mDFB->SetCooperativeLevel(mDFB, DFSCL_EXCLUSIVE);
	}

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;

	ret = mDFB->GetDisplayLayer (mDFB, DLID_PRIMARY, &mLayer);
	DBG_ASSERT (ret == DFB_OK);

	DFBDisplayLayerConfig layer_config;
	mLayer->GetConfiguration (mLayer, &layer_config);

	DFBDisplayLayerDescription layer_desc;
	mLayer->GetDescription (mLayer, &layer_desc);
	printf ("Display layer name: %s\n", layer_desc.name);
	printf ("\t width = %d\n", layer_config.width);
	printf ("\t height = %d\n", layer_config.height);

#if 0
	layer_config.flags = (DFBDisplayLayerConfigFlags)
		(DLCONF_BUFFERMODE | DLCONF_OPTIONS);
	layer_config.buffermode = DLBM_BACKVIDEO;
	layer_config.options = DLOP_ALPHACHANNEL;

	ret = mLayer->SetConfiguration (mLayer, &layer_config);
	/* DBG_ASSERT (ret == DFB_OK); */
#endif

	ret = mLayer->SetCooperativeLevel (mLayer, DLSCL_ADMINISTRATIVE);
	DBG_ASSERT (ret == DFB_OK);

	IDirectFBSurface * surface = 0;
	bool perferWindow = true;

#if defined(SEXY_INTEL_CANMORE) || defined(SEXY_INTEL_OLO) || defined(SEXY_ST_SH4)
	perferwindow = false;
	if (getenv ("SEXY_DFB_PERFER_WINDOW"))
		perferWindow = true;
#endif
	if (getenv ("SEXY_DFB_NO_WINDOW"))
		perferWindow = false;

	if (perferWindow)
	{
		DFBWindowDescription window_desc;

		window_desc.flags =
			(DFBWindowDescriptionFlags)(DWDESC_CAPS | DWDESC_WIDTH |  DWDESC_HEIGHT |
						    DWDESC_POSX | DWDESC_POSY);
		window_desc.caps = (DFBWindowCapabilities)(DWCAPS_ALPHACHANNEL | DWCAPS_DOUBLEBUFFER |
							   DWCAPS_NODECORATION);
		if (mApp->mIsWindowed)
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

		ret = mLayer->CreateWindow (mLayer, &window_desc, &mWindow);
		DBG_ASSERT (ret == DFB_OK);
		mWindow->SetOpacity (mWindow, 255);
		mWindow->RaiseToTop (mWindow);

		ret = mWindow->GetSurface (mWindow, &surface);
		DBG_ASSERT (ret == DFB_OK);
	}
	else
	{
		mLayer->EnableCursor (mLayer, DFB_FALSE);
		mSoftCursor = true;
	}

	DFBSurfaceDescription surface_desc;
	int width, height;

#if 0
	surface_desc.flags = DSDESC_CAPS;
	surface_desc.caps =
		(DFBSurfaceCapabilities)(DSCAPS_PRIMARY | DSCAPS_VIDEOONLY | DSCAPS_DOUBLE);
	ret = mDFB->CreateSurface(mDFB, &surface_desc, &surface);
	if (ret != DFB_OK) {
		surface_desc.caps =
			(DFBSurfaceCapabilities)(surface_desc.caps  & ~DSCAPS_DOUBLE);
		ret = mDFB->CreateSurface(mDFB, &surface_desc, &surface);
		DBG_ASSERT (ret == DFB_OK);
	}
#else
	if (!surface)
		ret = mLayer->GetSurface (mLayer, &surface);
	DBG_ASSERT(surface != NULL);
#endif
	surface->GetSize (surface, &width, &height);
	surface->Clear (surface, 0, 0, 0, 0);
	surface->Flip (surface, 0,
		       (DFBSurfaceFlipFlags)(DSFLIP_BLIT));
	surface->Clear (surface, 0, 0, 0, 0);
	surface->Flip (surface, 0,
		       (DFBSurfaceFlipFlags)(DSFLIP_BLIT));

	mPrimarySurface = surface;
	mWidth = width;
	mHeight = height;
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mDisplayWidth;
	mPresentationRect.mHeight = mDisplayHeight;

	mScreenImage = new DFBImage(surface, this);
	mScreenImage->mFlags = (ImageFlags)IMAGE_FLAGS_DOUBLE_BUFFER;

#if 0
	DFBInputDeviceID id = (DFBInputDeviceID)0xffffffff;
	ret = mDFB->GetInputDevice (mDFB, id, &mInput);
	DBG_ASSERT (ret == DFB_OK);
	ret = mInput->CreateEventBuffer (mInput, &mBuffer);
	DBG_ASSERT (ret == DFB_OK);
#endif
	if (mWindow)
		ret = mWindow->CreateEventBuffer (mWindow, &mBuffer);
	else
		ret = mDFB->CreateInputEventBuffer (mDFB, DICAPS_ALL, DFB_TRUE,
						    &mBuffer);
	DBG_ASSERT (ret == DFB_OK);

	mInitCount++;
	mInitialized = true;

	return 0;
}

void DFBInterface::SetVideoOnlyDraw(bool videoOnlyDraw)
{
	AutoCrit anAutoCrit(mCritSect);

	mVideoOnlyDraw = videoOnlyDraw;
}

void DFBInterface::RemapMouse(int& theX, int& theY)
{
}

ulong DFBInterface::GetColorRef(ulong theRGB)
{
	return theRGB;
}

void DFBInterface::AddImage(Image* theImage)
{
	mImageSet.insert((DFBImage*)theImage);
}

void DFBInterface::RemoveImage(Image* theImage)
{
	DFBImageSet::iterator anItr = mImageSet.find((DFBImage*)theImage);
	if (anItr != mImageSet.end())
		mImageSet.erase(anItr);
}

void DFBInterface::Remove3DData(MemoryImage* theImage) // for 3d texture cleanup
{
}

void DFBInterface::Cleanup()
{
	AutoCrit anAutoCrit(mCritSect);

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

	if (mPrimarySurface)
		mPrimarySurface = NULL;

	if (mWindow)
		mWindow->Release (mWindow);
	mWindow = NULL;

	if (mLayer)
		mLayer->Release (mLayer);
	mLayer = NULL;

	mCursorX = 0;
	mCursorY = 0;
	mCursorOldX = 0;
	mCursorOldY = 0;
	mCursorHotX = 0;
	mCursorHotY = 0;
	mSoftCursor = false;
}

bool DFBInterface::Redraw(Rect* theClipRect)
{
	AutoCrit anAutoCrit(mCritSect);

	if (!mInitialized)
		return false;

	if (mPrimarySurface)
		mPrimarySurface->Flip (mPrimarySurface, 0,
				       (DFBSurfaceFlipFlags)(DSFLIP_BLIT));

	return true;
}

bool DFBInterface::EnableCursor(bool enable)
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

bool DFBInterface::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	AutoCrit anAutoCrit(mCritSect);

	if (mDFBCursorImage)
		mDFBCursorImage->Release(mDFBCursorImage);
	mDFBCursorImage = NULL;
	mCursorImage = NULL;

	DFBImage * anImage = dynamic_cast<DFBImage*>(theImage);
	if (anImage)
		mDFBCursorImage = anImage->EnsureSurface();
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

void DFBInterface::SetCursorPos(int theCursorX, int theCursorY)
{
	AutoCrit anAutoCrit(mCritSect);

	if (mCursorX == theCursorX && mCursorY == theCursorY)
		return;

	mCursorX = theCursorX;
	mCursorY = theCursorY;
	if (!mSoftCursor && mLayer && true)
	{
		if (mWindow)
		{
			int x, y;

			mWindow->GetPosition (mWindow, &x, &y);
			theCursorX += x;
			theCursorY += y;
		}
		mLayer->WarpCursor (mLayer, theCursorX, theCursorY);
	}
}

bool DFBInterface::UpdateCursor(int theCursorX, int theCursorY)
{
	SetCursorPos (theCursorX, theCursorY);
	if (mSoftCursor && mCursorImage &&
	    (mCursorOldX != mCursorX || mCursorOldY != mCursorY))
		return true;
	return false;
}

bool DFBInterface::DrawCursor(Graphics* g)
{
	if (!mCursorImage)
		return false;

	if (mSoftCursor && mApp->mScreenBounds.Contains (mCursorX, mCursorY))
		g->DrawImage (mCursorImage,
			      mCursorX - mCursorHotX,
			      mCursorY - mCursorHotY);

	mCursorOldX = mCursorX;
	mCursorOldY = mCursorY;
	return true;
}

IDirectFBSurface* DFBInterface::CreateDFBSurface(int width, int height)
{
	IDirectFBSurface * aSurface;

	if (!mInitialized)
	{
		return 0;
	}
	else if (width && height)
	{
		AutoCrit anAutoCrit(mCritSect);

		DFBSurfaceDescription desc;

		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH |
							  DSDESC_HEIGHT |
							  DSDESC_PIXELFORMAT);
		desc.width = width;
		desc.height = height;
		desc.pixelformat = DSPF_ARGB;
		if (mDFB->CreateSurface(mDFB, &desc, &aSurface))
			return 0;
	} else {
		aSurface = 0;
	}
	return aSurface;
}

Image* DFBInterface::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	IDirectFBSurface * aSurface;

	if (!mInitialized)
	{
		return 0;
	}
	else if (width && height)
	{
		AutoCrit anAutoCrit(mCritSect);

		DFBSurfaceDescription desc;

		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH |
							  DSDESC_HEIGHT |
							  DSDESC_PIXELFORMAT);
		desc.width = width;
		desc.height = height;
		desc.pixelformat = DSPF_ARGB;
		if (mDFB->CreateSurface(mDFB, &desc, &aSurface))
			return 0;
	} else {
		aSurface = 0;
	}
	return new DFBImage(aSurface, this);
}

bool DFBInterface::HasEvent()
{
	if (!mBuffer)
		return false;

	return	mBuffer->HasEvent(mBuffer) == DFB_OK;
}

bool DFBInterface::GetEvent(struct Event &event)
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
			event.x = mMouseX;
			event.y = mMouseY;
			event.type = EVENT_MOUSE_BUTTON_PRESS;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.button = 1;
				break;
			case DIBI_MIDDLE:
				event.button = 3;
				break;
			case DIBI_RIGHT:
				event.button = 2;
				break;
			default:
				break;
			}
			break;
		case DIET_BUTTONRELEASE:
			event.x = mMouseX;
			event.y = mMouseY;
			event.type = EVENT_MOUSE_BUTTON_RELEASE;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.button = 1;
				break;
			case DIBI_MIDDLE:
				event.button = 3;
				break;
			case DIBI_RIGHT:
				event.button = 2;
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
				event.x = mMouseX;
				event.y = mMouseY;
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
			if (id == DIKI_UP) {
				event.keyCode = (int)KEYCODE_UP;
			} else if(id ==	 DIKI_DOWN) {
				event.keyCode = (int)KEYCODE_DOWN;
			} else if (id ==  DIKI_LEFT) {
				event.keyCode = (int)KEYCODE_LEFT;
			} else if (id == DIKI_RIGHT) {
				event.keyCode = (int)KEYCODE_RIGHT;
			} else if (id == DIKI_ENTER) {
				event.keyCode = (int)KEYCODE_RETURN;
			} else if (id == DIKI_SPACE) {
				event.keyCode = (int)KEYCODE_SPACE;
			} else if (id == DIKI_BACKSPACE) {
				event.keyCode = (int)KEYCODE_BACK;
			} else if (id == DIKI_ESCAPE) {
				if ((e->modifiers & (DIMM_SHIFT | DIMM_CONTROL)) == (DIMM_SHIFT | DIMM_CONTROL))
					event.type = EVENT_QUIT;
				else
					event.keyCode = (int)KEYCODE_ESCAPE;
			} else if (id == DIKI_SHIFT_L || id == DIKI_SHIFT_R) {
				event.keyCode = (int)KEYCODE_SHIFT;
			} else if (id == DIKI_CONTROL_L || id == DIKI_CONTROL_R) {
				event.keyCode = (int)KEYCODE_CONTROL;
			} else if (id >= DIKI_A && id < DIKI_Z) {
				event.keyCode = (int)('a' + id - DIKI_A);
				event.keyChar = (int)sym;
			} else if (id >= DIKI_0 && id < DIKI_9) {
				event.keyCode = (int)('0' + id - DIKI_0);
				event.keyChar = (int)sym;
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
			event.x = mMouseX;
			event.y = mMouseY;

			if (mWindow)
			{
				int x, y;

				mWindow->GetPosition (mWindow, &x, &y);
				event.x -= x;
				event.y -= y;
			}

			event.type = EVENT_MOUSE_BUTTON_PRESS;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.button = 1;
				break;
			case DIBI_MIDDLE:
				event.button = 3;
				break;
			case DIBI_RIGHT:
				event.button = 2;
				break;
			default:
				break;
			}
			break;
		case DWET_BUTTONUP:
			event.x = mMouseX;
			event.y = mMouseY;

			if (mWindow)
			{
				int x, y;

				mWindow->GetPosition (mWindow, &x, &y);
				event.x -= x;
				event.y -= y;
			}

			event.type = EVENT_MOUSE_BUTTON_RELEASE;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			switch (e->button) {
			case DIBI_LEFT:
				event.button = 1;
				break;
			case DIBI_MIDDLE:
				event.button = 3;
				break;
			case DIBI_RIGHT:
				event.button = 2;
				break;
			default:
				break;
			}
			break;
		case DWET_MOTION:
			mMouseX = e->cx;
			mMouseY = e->cy;
			event.x = mMouseX;
			event.y = mMouseY;

			if (mWindow)
			{
				int x, y;

				mWindow->GetPosition (mWindow, &x, &y);
				event.x -= x;
				event.y -= y;
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
			if (id == DIKI_UP) {
				event.keyCode = (int)KEYCODE_UP;
			} else if(id ==	 DIKI_DOWN) {
				event.keyCode = (int)KEYCODE_DOWN;
			} else if (id ==  DIKI_LEFT) {
				event.keyCode = (int)KEYCODE_LEFT;
			} else if (id == DIKI_RIGHT) {
				event.keyCode = (int)KEYCODE_RIGHT;
			} else if (id == DIKI_ENTER) {
				event.keyCode = (int)KEYCODE_RETURN;
			} else if (id == DIKI_SPACE) {
				event.keyCode = (int)KEYCODE_SPACE;
			} else if (id == DIKI_BACKSPACE) {
				event.keyCode = (int)KEYCODE_BACK;
			} else if (id == DIKI_ESCAPE) {
				if ((e->modifiers & (DIMM_SHIFT | DIMM_CONTROL)) == (DIMM_SHIFT | DIMM_CONTROL))
					event.type = EVENT_QUIT;
				else
					event.keyCode = (int)KEYCODE_ESCAPE;
			} else if (id == DIKI_SHIFT_L || id == DIKI_SHIFT_R) {
				event.keyCode = (int)KEYCODE_SHIFT;
			} else if (id == DIKI_CONTROL_L || id == DIKI_CONTROL_R) {
				event.keyCode = (int)KEYCODE_CONTROL;
			} else if (id >= DIKI_A && id < DIKI_Z) {
				event.keyCode = (int)('a' + id - DIKI_A);
				event.keyChar = (int)sym;
			} else if (id >= DIKI_0 && id < DIKI_9) {
				event.keyCode = (int)('0' + id - DIKI_0);
				event.keyChar = (int)sym;
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
	return true;
}

class DFBVideoDriver: public VideoDriver {
public:
	DFBVideoDriver ()
	 : VideoDriver("DFBInterface", 0)
	{
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new DFBInterface (theApp);
	}
};

static DFBVideoDriver aDFBVideoDriver;
VideoDriverRegistor aDFBVideoDriverRegistor (&aDFBVideoDriver);
VideoDriver* GetDFBVideoDriver()
{
	return &aDFBVideoDriver;
}
