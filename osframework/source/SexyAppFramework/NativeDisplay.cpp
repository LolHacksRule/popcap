#include "NativeDisplay.h"
#include "SexyAppBase.h"
#include "MemoryImage.h"
#include "Font.h"
#include "AutoCrit.h"

#ifdef SEXY_FREETYPE_FONT
#include "FreeTypeFont.h"
#endif

#include <stdlib.h>
#include <ctype.h>

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NativeDisplay::NativeDisplay()
{
	mPreserveBits = false;
	mRGBBits = 0;

	mRedMask = 0;
	mGreenMask = 0;
	mBlueMask = 0;

	mRedBits = 0;
	mGreenBits = 0;
	mBlueBits = 0;

	mRedShift = 0;
	mGreenShift = 0;
	mBlueShift = 0;

	mCursorX = 0;
	mCursorY = 0;

	mCurTexMemSpace = 0;
	mMaxTexMemSpace = 0;
	if (getenv("SEXY_MAX_TEX_MEM_SPACE"))
		mMaxTexMemSpace = atoi(getenv("SEXY_MAX_TEX_MEM_SPACE"));
	mTraceTexMemAlloc = getenv("SEXY_TRACE_TEX_MEM_ALLOC") != 0;

	mApp = 0;
	mMainThread = Thread::Self();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NativeDisplay::~NativeDisplay()
{
}

RenderSystem* NativeDisplay::GetRenderSystem()
{
	return 0;
}

bool NativeDisplay::Is3DAccelerated()
{
    return false;
}

bool NativeDisplay::Is3DAccelerationSupported()
{
    return false;
}

bool NativeDisplay::Is3DAccelerationRecommended()
{
    return false;
}

int NativeDisplay::Init()
{
	return -1;
}

bool NativeDisplay::CanReinit()
{
	return false;
}

bool NativeDisplay::Reinit()
{
	return false;
}

Font* NativeDisplay::CreateFont(SexyAppBase * theApp,
				 const std::string theFace,
				 int thePointSize,
				 bool bold,
				 bool italics,
				 bool underline)
{
#ifdef SEXY_FREETYPE_FONT
	return new FreeTypeFont(theFace, thePointSize * 96 / 72.0f, bold,
				italics, underline);
#endif
	return 0;
}

Image* NativeDisplay::CreateImage(SexyAppBase * theApp,
				  int width, int height)
{
	return 0;
}

bool  NativeDisplay::EnableCursor(bool enable)
{
	return false;
}

bool  NativeDisplay::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	return false;
}

void NativeDisplay::SetCursorPos(int theCursorX, int theCursorY)
{
}

bool  NativeDisplay::HasEvent()
{
	return false;
}

bool NativeDisplay::GetEvent(struct Event & event)
{
	event.type = EVENT_NONE;
	return false;
}

bool NativeDisplay::GetInputInfo(InputInfo &anInfo)
{
	return false;
}

bool NativeDisplay::ShowKeyboard(Widget* theWidget,
				 KeyboardMode mode,
				 const std::string &title,
				 const std::string &hint,
				 const std::string &initial)
{
	return false;
}

void NativeDisplay::HideKeyboard()
{
}

bool NativeDisplay::UpdateCursor(int theCursorX, int theCursorY)
{
	return false;
}

bool NativeDisplay::DrawCursor(Sexy::Graphics* g)
{
	return false;
}

bool NativeDisplay::CreateImageData(MemoryImage * theMemoryImage)
{
	return false;
}

void NativeDisplay::RemoveImageData(MemoryImage * theMemoryImage)
{
}

class DelayedCreateImageDataWork: public DelayedWork
{
public:
	DelayedCreateImageDataWork(NativeDisplay *theDisplay,
				   MemoryImage *theMemoryImage,
				   bool thePurgeBits) :
		mImage(theMemoryImage),
		mDisplay(theDisplay),
		mPurgeBits(thePurgeBits)
	{
        }

public:
	virtual void Work()
	{
		if (mDisplay->CreateImageData(mImage) && mPurgeBits)
			mImage->PurgeBits();
	}

private:
	MemoryImage* mImage;
	NativeDisplay* mDisplay;
	bool mPurgeBits;
};

void NativeDisplay::EnsureImageData(MemoryImage *theMemoryImage,
				    bool         thePurgeBits,
				    bool         theForce)
{
	if (!theMemoryImage)
		return;

	if (!theForce && !CanReinit())
		thePurgeBits = false;

	if (mMainThread != Thread::Self())
	{
		PushWork(new DelayedCreateImageDataWork(this, theMemoryImage,
							thePurgeBits));
	}
	else
	{
		if (CreateImageData(theMemoryImage) && thePurgeBits)
			theMemoryImage->PurgeBits();
	}
}

bool NativeDisplay::CanFullscreen()
{
    return true;
}

bool NativeDisplay::CanWindowed()
{
    return false;
}

void NativeDisplay::PushWork(DelayedWork* theWork)
{
	AutoCrit anAutoCrit(mWorkQueuCritSect);
	mWorkQueue.push_back(theWork);
}

bool NativeDisplay::IsWorkPending(void)
{
	AutoCrit anAutoCrit(mWorkQueuCritSect);
	return mWorkQueue.empty();
}

DelayedWork* NativeDisplay::PopWork(void)
{
	AutoCrit anAutoCrit(mWorkQueuCritSect);
	std::list<DelayedWork*>::iterator it = mWorkQueue.begin();
	if (it == mWorkQueue.end())
		return 0;

	DelayedWork* aWork = *it;
	mWorkQueue.pop_front();
	return aWork;
}

void NativeDisplay::FlushWork()
{
	DelayedWork* aWork;

	for (aWork = PopWork(); aWork; aWork = PopWork())
	{
		aWork->Work();
		delete aWork;
	}
}

void NativeDisplay::AllocTexMemSpace(DWORD theTexMemSize)
{
	AutoCrit aAutoCrit(mTexMemSpaceCritSect);
	mCurTexMemSpace += theTexMemSize;

	if (mTraceTexMemAlloc)
		printf ("Alloc: MaxTexMemSpace: %u CurTexMemSpace: %u -> %u\n",
			mMaxTexMemSpace, mCurTexMemSpace - theTexMemSize,
			mCurTexMemSpace);
}

void NativeDisplay::FreeTexMemSpace(DWORD theTexMemSize)
{
	if (!theTexMemSize)
		return;

	AutoCrit aAutoCrit(mTexMemSpaceCritSect);
	if (mCurTexMemSpace >= theTexMemSize)
		mCurTexMemSpace -= theTexMemSize;
	else
		mCurTexMemSpace = 0;

	if (mTraceTexMemAlloc)
		printf ("Free: MaxTexMemSpace: %u CurTexMemSpace: %u -> %u\n",
			mMaxTexMemSpace, mCurTexMemSpace + theTexMemSize,
			mCurTexMemSpace);
}

bool NativeDisplay::EnsureTexMemSpace(DWORD theTexMemSize)
{
	if (!mMaxTexMemSpace)
		return true;
	if (!mApp)
		return false;

	if (mTraceTexMemAlloc)
		printf ("MaxTexMemSpace: %u CurTexMemSpace: %u required: %u\n",
			mMaxTexMemSpace, mCurTexMemSpace, theTexMemSize);

	if (mCurTexMemSpace < mMaxTexMemSpace &&
	    mCurTexMemSpace + theTexMemSize < mMaxTexMemSpace)
		return true;

	if (mTraceTexMemAlloc)
		printf ("Try to evict some textures to free space.\n");

	mApp->Evict3DImageData(theTexMemSize);

	if (mTraceTexMemAlloc)
		printf ("MaxTexMemSpace: %u CurTexMemSpace: %u required: %u\n",
			mMaxTexMemSpace, mCurTexMemSpace, theTexMemSize);

	return true;
}

void NativeDisplay::Update()
{
	FlushWork();
}
