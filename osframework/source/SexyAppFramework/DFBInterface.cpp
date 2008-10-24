#include "DFBInterface.h"
#include "DFBImage.h"
#include "SexyAppBase.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"

#include <cstdio>

using namespace Sexy;

DFBInterface::DFBInterface(SexyAppBase* theApp)
{
	mApp = theApp;
	mDFB = NULL;
	mScreenImage = NULL;
	mRedAddTable = NULL;
	mGreenAddTable = NULL;
	mBlueAddTable = NULL;
	mInitialized = false;
	mVideoOnlyDraw = false;
	mScanLineFailCount = 0;

	//TODO: Standards, anyone?
	mNextCursorX = 0;
	mNextCursorY = 0;
	mCursorX = 0;
	mCursorY = 0;
	mInRedraw = false;
	//mCursorWidth = 54;
	//mCursorHeight = 54;
	mCursorWidth = 64;
	mCursorHeight = 64;
	mHasOldCursorArea = false;
	mNewCursorAreaImage = NULL;
	mOldCursorAreaImage = NULL;
	mInitCount = 0;
	mRefreshRate = 60;
	mMillisecondsPerFrame = 1000 / mRefreshRate;

	mInput = NULL;
	mBuffer = NULL;
	mMouseX = 0;
	mMouseY = 0;
	mLayer = NULL;
}

DFBInterface::~DFBInterface()
{
	delete [] mRedAddTable;
	delete [] mGreenAddTable;
	delete [] mBlueAddTable;

	Cleanup();

	if (mDFB)
		mDFB->Release(mDFB);
}

std::string DFBInterface::ResultToString(int theResult)
{
	return "RESULT_UNKNOWN";
}

Image* DFBInterface::GetScreenImage()
{
	return mScreenImage;
}

int DFBInterface::Init(void)
{
	Cleanup();

	AutoCrit anAutoCrit(mCritSect);
	mInitialized = false;

	if (!mDFB) {
		DirectFBInit(NULL, NULL);
		DirectFBCreate(&mDFB);
		//mDFB->SetCooperativeLevel(mDFB, DFSCL_FULLSCREEN);
	}

        mDFB->GetDisplayLayer (mDFB, DLID_PRIMARY, &mLayer);

	DFBDisplayLayerConfig layer_config;
	mLayer->GetConfiguration (mLayer, &layer_config);

	layer_config.flags = (DFBDisplayLayerConfigFlags)
		(DLCONF_BUFFERMODE | DLCONF_OPTIONS);
	layer_config.buffermode = DLBM_BACKVIDEO;
	layer_config.options = DLOP_ALPHACHANNEL;

	mLayer->SetConfiguration (mLayer, &layer_config);
	mLayer->SetCooperativeLevel (mLayer, DLSCL_ADMINISTRATIVE);
	mLayer->EnableCursor (mLayer, 1);

	IDirectFBSurface * surface;
	DFBSurfaceDescription surface_desc;
	int width, height;
	int ret;

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
	mLayer->GetSurface (mLayer, &surface);
#endif
	surface->GetSize (surface, &width, &height);
	surface->Clear (surface, 0, 0, 0, 0);
	mPrimarySurface = surface;
	mWidth = width;
	mHeight = height;

	mScreenImage = new DFBImage(surface, this);

#if 0
	DFBInputDeviceID id = (DFBInputDeviceID)0xffffffff;
	ret = mDFB->GetInputDevice (mDFB, id, &mInput);
	DBG_ASSERT (ret == DFB_OK);
	ret = mInput->CreateEventBuffer (mInput, &mBuffer);
	DBG_ASSERT (ret == DFB_OK);
#endif
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
	if (mInitialized)
	{
		theX = ( theX - mPresentationRect.mX ) * mWidth / mPresentationRect.mWidth;
		theY = ( theY - mPresentationRect.mY ) * mHeight / mPresentationRect.mHeight;
	}
}

ulong DFBInterface::GetColorRef(ulong theRGB)
{
	return theRGB;
}

void DFBInterface::AddImage(Image* theImage)
{
	//AutoCrit anAutoCrit(mCritSect);

	mImageSet.insert((DFBImage*)theImage);
}

void DFBInterface::RemoveImage(Image* theImage)
{
	//AutoCrit anAutoCrit(mCritSect);

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

	if (mOldCursorAreaImage != NULL)
	{
		delete mOldCursorAreaImage;
		mOldCursorAreaImage = NULL;
	}

	if (mNewCursorAreaImage != NULL)
	{
		delete mNewCursorAreaImage;
		mNewCursorAreaImage = NULL;
	}

	if (mScreenImage != NULL)
	{
		delete mScreenImage;
		mScreenImage = NULL;
	}

	if (mPrimarySurface)
		mPrimarySurface = NULL;

	if (mLayer)
		mLayer->Release(mLayer);
	mLayer = NULL;
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

bool DFBInterface::SetCursorImage(Image* theImage)
{
	AutoCrit anAutoCrit(mCritSect);

	if (mCursorImage != theImage)
	{
		// Wait until next Redraw or cursor move to draw new cursor
		mCursorImage = theImage;
		return true;
	}
	else
		return false;
}

void DFBInterface::SetCursorPos(int theCursorX, int theCursorY)
{
	mNextCursorX = theCursorX;
	mNextCursorY = theCursorY;

	if (mInRedraw)
		return;

	AutoCrit anAutoCrit(mCritSect);

	if (mHasOldCursorArea)
	{
		;
	}
	else
	{
		mCursorX = theCursorX;
		mCursorY = theCursorY;
	}
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

	return 	mBuffer->HasEvent(mBuffer) == DFB_OK;
}

bool DFBInterface::GetEvent(struct Event &event)
{
	if (!HasEvent())
		return false;

 	DFBEvent dfb_event;
	mBuffer->GetEvent(mBuffer, &dfb_event);

	switch (dfb_event.clazz) {
	case DFEC_INPUT:
		DFBInputEvent * e;

		e = &dfb_event.input;
		event.type = EVENT_NONE;
		switch (e->type) {
		case DIET_BUTTONPRESS:
			event.x = mMouseX;
			event.y = mMouseY;
			event.type = EVENT_MOUSE_BUTTON_PRESS;
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
			}
			break;
		case DIET_KEYPRESS:
		case DIET_KEYRELEASE:
		{
			const DFBInputDeviceKeyIdentifier id = e->key_id;
			const DFBInputDeviceKeySymbol sym = e->key_symbol;

			if (e->type == DIET_KEYPRESS)
				event.type = EVENT_KEY_DOWN;
			else
				event.type = EVENT_KEY_UP;
			if (id == DIKI_UP) {
				event.keyCode = (int)KEYCODE_UP;
			} else if(id ==  DIKI_DOWN) {
				event.keyCode = (int)KEYCODE_DOWN;
			} else if (id ==  DIKI_LEFT) {
				event.keyCode = (int)KEYCODE_LEFT;
			} else if (id == DIKI_RIGHT) {
				event.keyCode = (int)KEYCODE_RIGHT;
			} else if (id == DIKI_ENTER) {
				event.keyCode = (int)KEYCODE_ACCEPT;
			} else if (id == DIKI_SPACE) {
				event.keyCode = (int)KEYCODE_SPACE;
			} else if (id == DIKI_BACKSPACE) {
				event.keyCode = (int)KEYCODE_BACK;
			} else if (id == DIKI_ESCAPE) {
				event.keyCode = (int)KEYCODE_ESCAPE;
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
	case DIET_UNKNOWN:
	default:
		event.type = EVENT_NONE;
		break;
	}
	return true;
}
