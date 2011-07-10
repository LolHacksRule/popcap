//#define SEXY_TRACING_ENABLED
//#define SEXY_PERF_ENABLED
//#define SEXY_MEMTRACE

#include "SexyAppBase.h"
#include "SEHCatcher.h"
#include "WidgetManager.h"
#include "Widget.h"
#include "Debug.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"
#include "SoundDriverFactory.h"
#include "SoundManager.h"
#include "SoundInstance.h"
#include "MusicInterface.h"
#include "DummySoundManager.h"
#include "MemoryImage.h"
#include "HTTPTransfer.h"
#include "Dialog.h"
#include "../ImageLib/ImageLib.h"
#include "Rect.h"
#include "PropertiesParser.h"
#include "PerfTimer.h"
#include "MTRand.h"
#include "ModVal.h"
#include "SexyDebug.h"
#include "SexyI18n.h"
#include "SexyUtf8.h"

#include <iostream>
#include <fstream>
#include <time.h>
#include <math.h>

#include "ResourceManager.h"
#include "InputManager.h"
#include "RegistryInterfaceFactory.h"
#include "AutoCrit.h"
#include "Debug.h"
#include "../PakLib/PakInterface.h"
#include <string>

#ifdef WIN32
#include <process.h>
#include <direct.h>
#include <regstr.h>
#include <shlobj.h>
#else
#include <sys/time.h>
#endif

#include "memmgr.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>

#define  LOG_TAG    "Sexy"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define printf LOGI
#endif

using namespace Sexy;

const int DEMO_FILE_ID = 0x42BEEF78;
const int DEMO_VERSION = 2;

SexyAppBase* Sexy::gSexyAppBase = NULL;

static bool gScreenSaverActive = false;

#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif

//HotSpot: 0 0
//Size: 32 32
static unsigned char gArrowCursorData[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xff,
	0xff, 0x8f, 0xff, 0xff, 0xff, 0x87, 0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0xff, 0x81, 0xff,
	0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0x83, 0xff, 0xff, 0xff, 0x93,
	0xff, 0xff, 0xff, 0xb9, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff,
	0xfc, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00,
	0x00, 0xe0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xfc, 0x00,
	0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0xff,
	0xc0, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xef, 0x00, 0x00, 0x00,
	0xcf, 0x00, 0x00, 0x00, 0x87, 0x80, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x03, 0xc0, 0x00,
	0x00, 0x03, 0xc0, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00
};

//HotSpot: 11 4
//Size: 32 32
static unsigned char gFingerCursorData[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xc3,
	0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff,
	0xc0, 0x07, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xfc, 0x40, 0x01, 0xff, 0xfc, 0x00, 0x01, 0xff,
	0xfc, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0x00, 0x01,
	0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xc0,
	0x03, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18,
	0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
	0x18, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x1b, 0x60, 0x00, 0x00, 0x1b, 0x68, 0x00,
	0x00, 0x1b, 0x6c, 0x00, 0x01, 0x9f, 0xec, 0x00, 0x01, 0xdf, 0xfc, 0x00, 0x00, 0xdf, 0xfc,
	0x00, 0x00, 0x5f, 0xfc, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x3f,
	0xf8, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00,
	0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00
};

//HotSpot: 15 10
//Size: 32 32
static unsigned char gDraggingCursorData[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0,
	0x01, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff,
	0xe0, 0x00, 0xff, 0xfe, 0x60, 0x00, 0xff, 0xfc, 0x20, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff,
	0xfe, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00,
	0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xf0,
	0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x80, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x0d, 0xb0, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00,
	0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00, 0x00, 0x0d, 0xb6, 0x00,
	0x01, 0x8d, 0xb6, 0x00, 0x01, 0xcf, 0xfe, 0x00, 0x00, 0xef, 0xfe, 0x00, 0x00, 0xff, 0xfe,
	0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x1f,
	0xfc, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00,
	0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00
};

//////////////////////////////////////////////////////////////////////////
static void SexyCrash(void)
{
	if (gSexyAppBase)
		gSexyAppBase->Cleanup(true);
}

//////////////////////////////////////////////////////////////////////////
SexyAppBase::SexyAppBase()
{
	gSexyAppBase = this;

	ImageLib::InitJPEG2000();

	DebugInit(FAULT_HANDLER);
	DebugSetFaultCallback(SexyCrash);

	mNotifyGameMessage = 0;

#ifdef _DEBUG
	mOnlyAllowOneCopyToRun = false;
#else
	mOnlyAllowOneCopyToRun = true;
#endif

	mChangeDirTo = GetResourcesFolder();
	mResourceManifest = "properties/resources.xml";

	mArgc = 0;
	mArgv = 0;
	mNoDefer = false;
	mFullScreenPageFlip = true; // should we page flip in fullscreen?
	mTimeLoaded = GetTickCount();
	mSEHOccured = false;
	mProdName = "Product";
	mTitle = "SexyApp";
	mShutdown = false;
	mExitToTop = false;
	mWidth = 640;
	mHeight = 480;
	mFullscreenBits = 16;
	mIsWindowed = false;
	mIsPhysWindowed = true;
	mFullScreenWindow = false;
	mPreferredX = -1;
	mPreferredY = -1;
	mIsScreenSaver = false;
	mAllowMonitorPowersave = true;
	mHWnd = NULL;
	mDDInterface = NULL;
	mMusicInterface = NULL;
	mInvisHWnd = NULL;
	mFrameTime = 10;
	mNonDrawCount = 0;
	mDrawCount = 0;
	mSleepCount = 0;
	mUpdateCount = 0;
	mUpdateAppState = 0;
	mUpdateAppDepth = 0;
	mPendingUpdatesAcc = 0.0;
	mUpdateFTimeAcc = 0.0;
	mAllowSleep = true;
	mHasPendingDraw = true;
	mIsDrawing = false;
	mLastDrawWasEmpty = false;
	mLastTimeCheck = 0;
	mUpdateMultiplier = 1;
	mPaused = false;
	mFastForwardToUpdateNum = 0;
	mFastForwardToMarker = false;
	mFastForwardStep = false;
	mSoundManager = NULL;
	mCursorNum = CURSOR_POINTER;
	mMouseIn = false;
	mRunning = false;
	mActive = true;
	mProcessInTimer = false;
	mMinimized = false;
	mPhysMinimized = false;
	mIsDisabled = false;
	mLoaded = false;
	mYieldMainThread = false;
	mLoadingFailed = false;
	mLoadingThreadStarted = false;
	mAutoStartLoadingThread = true;
	mLoadingThreadCompleted = false;
	mCursorThreadRunning = false;
	mNumLoadingThreadTasks = 0;
	mCompletedLoadingThreadTasks = 0;
	mLastDrawTick = GetTickCount();
	mNextDrawTick = GetTickCount();
	mSysCursor = true;
	mForceFullscreen = false;
	mForceWindowed = false;
	mHasFocus = true;
	mCustomCursorsEnabled = false;
	mCustomCursorDirty = false;
	mHandCursor = 0;
	mDraggingCursor = 0;
	mArrowCursor = 0;
	//mOverrideCursor = NULL;
	mIsOpeningURL = false;
	mInitialized = false;
	mLastShutdownWasGraceful = true;
	mReadFromRegistry = false;
	mCmdLineParsed = false;
	mSkipSignatureChecks = false;
	mCtrlDown = false;
	mAltDown = false;
	mStepMode = 0;
	mCleanupSharedImages = false;
	mStandardWordWrap = true;
	mbAllowExtendedChars = true;
	mEnableMaximizeButton = false;

	mMusicVolume = 0.85;
	mSfxVolume = 0.85;
	mDemoMusicVolume = mDemoSfxVolume = 0.0;
	mMuteCount = 0;
	mAutoMuteCount = 0;
	mDemoMute = false;
	mMuteOnLostFocus = true;
	mCurHandleNum = 0;
	mFPSTime = 0;
	mFPSStartTick = GetTickCount();
	mFPSFlipCount = 0;
	mFPSCount = 0;
	mFPSDirtyCount = 0;
	mShowFPS = false;
	mShowFPSMode = FPS_ShowFPS;
	mDrawTime = 0;
	mScreenBltTime = 0;
	mAlphaDisabled = false;
	mDebugKeysEnabled = false;
	//mOldWndProc = 0;
	mNoSoundNeeded = false;
	mWantFMod = false;

	mSyncRefreshRate = 100;
	mVSyncUpdates = false;
	mVSyncBroken = false;
	mVSyncBrokenCount = 0;
	mVSyncBrokenTestStartTick = 0;
	mVSyncBrokenTestUpdates = 0;
	mWaitForVSync = false;
	mSoftVSyncWait = false;
	mUserChanged3DSetting = false;
	mAutoEnable3D = true;
	mIs3D = true;
	mTest3D = false;
	mMinVidMemory3D = 6;
	mRecommendedVidMemory3D = 14;
	mRelaxUpdateBacklogCount = 0;
	mWidescreenAware = false;
	mEnableWindowAspect = false;
	mWindowAspect.Set(4, 3);
	mIsWideWindow = false;

	int i;

	mCursorEnabled = true;
	for (i = 0; i < NUM_CURSORS; i++)
		mCursorImages[i] = NULL;

	for (i = 0; i < 256; i++)
		mAdd8BitMaxTable[i] = i;

	for (i = 256; i < 512; i++)
		mAdd8BitMaxTable[i] = 255;

	// Set default strings.  Init could read in overrides from partner.xml
	SetString("DIALOG_BUTTON_OK",		WSTR("OK"));
	SetString("DIALOG_BUTTON_CANCEL",	WSTR("CANCEL"));

	SetString("UPDATE_CHECK_TITLE",		WSTR("Update Check"));
	SetString("UPDATE_CHECK_BODY",		WSTR("Checking if there are any updates available for this product ..."));

	SetString("UP_TO_DATE_TITLE",		WSTR("Up to Date"));
	SetString("UP_TO_DATE_BODY",		WSTR("There are no updates available for this product at this time."));
	SetString("NEW_VERSION_TITLE",		WSTR("New Version"));
	SetString("NEW_VERSION_BODY",		WSTR("There is an update available for this product.  Would you like to visit the web site to download it?"));


	mDemoPrefix = "sexyapp";
	mDemoFileName = mDemoPrefix + ".dmo";
	mPlayingDemoBuffer = false;
	mManualShutdown = false;
	mRecordingDemoBuffer = false;
	mLastDemoMouseX = 0;
	mLastDemoMouseY = 0;
	mLastDemoUpdateCnt = 0;
	mDemoNeedsCommand = true;
	mDemoLoadingComplete = false;
	mDemoLength = 0;
	mDemoCmdNum = 0;
	mDemoCmdOrder = -1; // Means we haven't processed any demo commands yet
	mDemoCmdBitPos = 0;
	mLastDrawnTime = 0;

	mWidgetManager = new WidgetManager(this);
	mResourceManager = new ResourceManager(this);
	mInputManager = new InputManager (this);
	mRegistryInterface = RegistryInterfaceFactory::GetInterface(this);

	mPrimaryThreadId = 0;
	mTabletPC = false;
}

void SexyAppBase::Cleanup(bool force)
{
	Shutdown();

	ProcessSafeDeleteList();

	DialogMap::iterator aDialogItr = mDialogMap.begin();
	while (aDialogItr != mDialogMap.end())
	{
		mWidgetManager->RemoveWidget(aDialogItr->second);
		delete aDialogItr->second;
		++aDialogItr;
	}
	mDialogMap.clear();
	mDialogList.clear();

	if (force)
		mWidgetManager->RemoveAllWidgets(true, true);
	delete mWidgetManager;
	mWidgetManager = 0;

	delete mResourceManager;
	mResourceManager = 0;

	delete mInputManager;
	mInputManager = 0;

	SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
	while (aSharedImageItr != mSharedImageMap.end())
	{
		SharedImage* aSharedImage = &aSharedImageItr->second;
		DBG_ASSERTE(aSharedImage->mRefCount == 0);
		delete aSharedImage->mImage;
		mSharedImageMap.erase(aSharedImageItr++);
	}
	DeleteExtraImageData();

	if (mDDInterface)
		mDDInterface->Update();

        delete mArrowCursor;
	mArrowCursor = 0;

	delete mHandCursor;
	mHandCursor = 0;

	delete mDraggingCursor;
	mDraggingCursor = 0;

	delete mMusicInterface;
	mMusicInterface = 0;

	delete mSoundManager;
	mSoundManager = 0;

	delete mDDInterface;
	mDDInterface = 0;

	WaitForLoadingThread();

	delete mRegistryInterface;
	mRegistryInterface = 0;

	WriteDemoBuffer();
}

SexyAppBase::~SexyAppBase()
{
	Cleanup();
	gSexyAppBase = NULL;
}

void SexyAppBase::ClearUpdateBacklog(bool relaxForASecond)
{
	mLastTimeCheck = GetTickCount();
	mUpdateFTimeAcc = 0.0;

	if (relaxForASecond)
		mRelaxUpdateBacklogCount = 1000;
}

bool SexyAppBase::IsScreenSaver()
{
	return mIsScreenSaver;
}

bool SexyAppBase::AppCanRestore()
{
	return !mIsDisabled;
}

bool SexyAppBase::ReadDemoBuffer(std::string &theError)
{
	return true;
}

void SexyAppBase::WriteDemoBuffer()
{
}

void SexyAppBase::DemoSyncBuffer(Buffer* theBuffer)
{
}

void SexyAppBase::DemoSyncString(std::string* theString)
{
}

void SexyAppBase::DemoSyncInt(int* theInt)
{
}

void SexyAppBase::DemoSyncBool(bool* theBool)
{
}

void SexyAppBase::DemoAssertStringEqual(const std::string& theString)
{
}

void SexyAppBase::DemoAddMarker(const std::string& theString)
{
}

#if 0
void SexyAppBase::DemoRegisterHandle(HANDLE theHandle)
{
}

void SexyAppBase::DemoWaitForHandle(HANDLE theHandle)
{
}

bool SexyAppBase::DemoCheckHandle(HANDLE theHandle)
{
}
#endif

void SexyAppBase::DemoAssertIntEqual(int theInt)
{
}

Dialog* SexyAppBase::NewDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode)
{
	Dialog* aDialog = new Dialog(NULL, NULL, theDialogId, isModal, theDialogHeader,	theDialogLines, theDialogFooter, theButtonMode);
	return aDialog;
}

Dialog* SexyAppBase::DoDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode)
{
	KillDialog(theDialogId);

	Dialog* aDialog = NewDialog(theDialogId, isModal, theDialogHeader, theDialogLines, theDialogFooter, theButtonMode);

	AddDialog(theDialogId, aDialog);

	return aDialog;
}


Dialog*	SexyAppBase::GetDialog(int theDialogId)
{
	DialogMap::iterator anItr = mDialogMap.find(theDialogId);

	if (anItr != mDialogMap.end())
		return anItr->second;

	return NULL;
}

bool SexyAppBase::KillDialog(int theDialogId, bool removeWidget, bool deleteWidget)
{
	DialogMap::iterator anItr = mDialogMap.find(theDialogId);

	if (anItr != mDialogMap.end())
	{
		Dialog* aDialog = anItr->second;

		// set the result to something else so DoMainLoop knows that the dialog is gone
		// in case nobody else sets mResult
		if (aDialog->mResult == -1)
			aDialog->mResult = 0;

		DialogList::iterator aListItr = std::find(mDialogList.begin(),mDialogList.end(),aDialog);
		if (aListItr != mDialogList.end())
			mDialogList.erase(aListItr);

		mDialogMap.erase(anItr);

		if (removeWidget || deleteWidget)
		mWidgetManager->RemoveWidget(aDialog);

		if (aDialog->IsModal())
		{
			ModalClose();
			mWidgetManager->RemoveBaseModal(aDialog);
		}

		if (deleteWidget)
		SafeDeleteWidget(aDialog);

		return true;
	}

	return false;
}

bool SexyAppBase::KillDialog(int theDialogId)
{
	return KillDialog(theDialogId,true,true);
}

bool SexyAppBase::KillDialog(Dialog* theDialog)
{
	return KillDialog(theDialog->mId);
}

int SexyAppBase::GetDialogCount()
{
	return mDialogMap.size();
}

void SexyAppBase::AddDialog(int theDialogId, Dialog* theDialog)
{
	KillDialog(theDialogId);

	if (theDialog->mWidth == 0)
	{
		// Set the dialog position ourselves
		int aWidth = mWidth/2;
		theDialog->Resize((mWidth - aWidth)/2, mHeight / 5, aWidth, theDialog->GetPreferredHeight(aWidth));
	}

	mDialogMap.insert(DialogMap::value_type(theDialogId, theDialog));
	mDialogList.push_back(theDialog);

	mWidgetManager->AddWidget(theDialog);
	if (theDialog->IsModal())
	{
		mWidgetManager->AddBaseModal(theDialog);
		ModalOpen();
	}
}

void SexyAppBase::AddDialog(Dialog* theDialog)
{
	AddDialog(theDialog->mId, theDialog);
}

void SexyAppBase::ModalOpen()
{
}

void SexyAppBase::ModalClose()
{
}

void SexyAppBase::DialogButtonPress(int theDialogId, int theButtonId)
{
	if (theButtonId == Dialog::ID_YES)
		ButtonPress(2000 + theDialogId);
	else if (theButtonId == Dialog::ID_NO)
		ButtonPress(3000 + theDialogId);
}

void SexyAppBase::DialogButtonDepress(int theDialogId, int theButtonId)
{
	if (theButtonId == Dialog::ID_YES)
		ButtonDepress(2000 + theDialogId);
	else if (theButtonId == Dialog::ID_NO)
		ButtonDepress(3000 + theDialogId);
}

void SexyAppBase::GotFocus()
{
}

void SexyAppBase::LostFocus()
{
}

void SexyAppBase::URLOpenFailed(const std::string& theURL)
{
	mIsOpeningURL = false;
}

void SexyAppBase::URLOpenSucceeded(const std::string& theURL)
{
	mIsOpeningURL = false;

	if (mShutdownOnURLOpen)
		Shutdown();
}

bool SexyAppBase::OpenURL(const std::string& theURL, bool shutdownOnOpen)
{
	std::cout<<"url:"<<theURL<<std::endl;
	return true;
}

std::string SexyAppBase::GetProductVersion(const std::string& thePath)
{
	// Get Product Version
	std::string aProductVersion;
	return aProductVersion;
}

void SexyAppBase::WaitForLoadingThread()
{
        if ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
	{
		mLoadingThread.Join();
		mLoadingThreadStarted = false;
	}
}

void SexyAppBase::SetCursorImage(int theCursorNum, Image* theImage)
{
	if ((theCursorNum >= 0) && (theCursorNum < NUM_CURSORS))
	{
		mCursorImages[theCursorNum] = theImage;
		EnforceCursor();
	}
}

void SexyAppBase::TakeScreenshot()
{
	ClearUpdateBacklog();
}

void SexyAppBase::DumpProgramInfo()
{
}

double SexyAppBase::GetLoadingThreadProgress()
{
	if (mLoaded)
		return 1.0;
	if (!mLoadingThreadStarted)
		return 0.0;
	if (mNumLoadingThreadTasks == 0)
		return 0.0;
	return std::min(mCompletedLoadingThreadTasks / (double) mNumLoadingThreadTasks, 1.0);
}

bool SexyAppBase::RegistryWrite(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength)
{
	return mRegistryInterface->Write(theValueName, theType, theValue, theLength);
}

bool SexyAppBase::RegistryWriteString(const std::string& theValueName, const std::string& theString)
{
	return mRegistryInterface->WriteString(theValueName, theString);
}

bool SexyAppBase::RegistryWriteInteger(const std::string& theValueName, int theValue)
{
	return mRegistryInterface->WriteInteger(theValueName, theValue);
}

bool SexyAppBase::RegistryWriteBoolean(const std::string& theValueName, bool theValue)
{
	return mRegistryInterface->WriteBoolean(theValueName, theValue);
}

bool SexyAppBase::RegistryWriteData(const std::string& theValueName, const uchar* theValue, ulong theLength)
{
	return mRegistryInterface->WriteData(theValueName, theValue, theLength);
}

void SexyAppBase::WriteToRegistry()
{
	RegistryWriteInteger("MusicVolume", (int) (mMusicVolume * 100));
	RegistryWriteInteger("SfxVolume", (int) (mSfxVolume * 100));
	RegistryWriteInteger("Muted", (mMuteCount - mAutoMuteCount > 0) ? 1 : 0);
	RegistryWriteInteger("ScreenMode", mIsWindowed ? 0 : 1);
	RegistryWriteInteger("Is3D", mIs3D ? 1 : 0);
	RegistryWriteInteger("PreferredX", mPreferredX);
	RegistryWriteInteger("PreferredY", mPreferredY);
	RegistryWriteInteger("CustomCursors", mCustomCursorsEnabled ? 1 : 0);
	RegistryWriteInteger("InProgress", 0);
	RegistryWriteBoolean("WaitForVSync", mWaitForVSync);

	mRegistryInterface->Flush ();
}

bool SexyAppBase::RegistryEraseKey(const SexyString& _theKeyName)
{
	return mRegistryInterface->EraseKey (_theKeyName);
}

void SexyAppBase::RegistryEraseValue(const SexyString& _theValueName)
{
	mRegistryInterface->EraseValue (_theValueName);
}

bool SexyAppBase::RegistryGetSubKeys(const std::string& theKeyName, StringVector* theSubKeys)
{
	return mRegistryInterface->GetSubKeys (theKeyName, theSubKeys);
}

bool SexyAppBase::RegistryRead(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength)
{
	return  mRegistryInterface->Read(theValueName, theType, theValue, theLength);
}

bool SexyAppBase::RegistryReadString(const std::string& theKey, std::string* theString)
{
	return  mRegistryInterface->ReadString(theKey, theString);
}

bool SexyAppBase::RegistryReadInteger(const std::string& theKey, int* theValue)
{
	return  mRegistryInterface->ReadInteger(theKey, theValue);
}

bool SexyAppBase::RegistryReadBoolean(const std::string& theKey, bool* theValue)
{
	return  mRegistryInterface->ReadBoolean(theKey, theValue);
}

bool SexyAppBase::RegistryReadData(const std::string& theKey, uchar* theValue, ulong* theLength)
{
	return  mRegistryInterface->ReadData(theKey, theValue, theLength);
}

void SexyAppBase::ReadFromRegistry()
{
        mReadFromRegistry = true;
        mRegKey = SexyStringToString(GetString("RegistryKey", StringToSexyString(mRegKey)));

	int anInt = 0;

	if (RegistryReadInteger("MusicVolume", &anInt))
		mMusicVolume = anInt / 100.0;

	if (RegistryReadInteger("SfxVolume", &anInt))
		mSfxVolume = anInt / 100.0;

	if (RegistryReadInteger("Muted", &anInt))
		mMuteCount = anInt;

	if (RegistryReadInteger("ScreenMode", &anInt))
		mIsWindowed = anInt == 0;

	if (RegistryReadInteger("Is3D", &anInt))
		mIs3D = anInt != 0;

	RegistryReadInteger("PreferredX", &mPreferredX);
	RegistryReadInteger("PreferredY", &mPreferredY);

	if (RegistryReadInteger("CustomCursors", &anInt))
		EnableCustomCursors(anInt != 0);
}

bool SexyAppBase::WriteBytesToFile(const std::string& theFileName, const void *theData, unsigned long theDataLen)
{
	FILE* aFP = fopen(theFileName.c_str(), "w+b");
	if (!aFP)
		return false;
	fwrite(theData, 1, theDataLen, aFP);
	fclose(aFP);

	return true;
}

bool SexyAppBase::WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer)
{
	return WriteBytesToFile(theFileName,theBuffer->GetDataPtr(),theBuffer->GetDataLen());
}


bool SexyAppBase::ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer, bool dontWriteToDemo)
{
	if ((mPlayingDemoBuffer) && (!dontWriteToDemo))
	{
		if (mManualShutdown)
			return false;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;

		DBG_ASSERTE(!mDemoIsShortCmd);
		DBG_ASSERTE(mDemoCmdNum == DEMO_FILE_READ);

		bool success = mDemoBuffer.ReadNumBits(1, false) != 0;
		if (!success)
			return false;

		uint32 aLen = mDemoBuffer.ReadLong();

		theBuffer->Clear();
		for (int i = 0; i < (int) aLen; i++)
			theBuffer->WriteByte(mDemoBuffer.ReadByte());

		return true;
	}
	else
	{
		PFILE* aFP = p_fopen(theFileName.c_str(), "rb");

		if (aFP == NULL)
		{
			if ((mRecordingDemoBuffer) && (!dontWriteToDemo))
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_FILE_READ, 5);
				mDemoBuffer.WriteNumBits(0, 1); // failure
			}

			return false;
		}

		p_fseek(aFP, 0, SEEK_END);
		int aFileSize = p_ftell(aFP);
		p_fseek(aFP, 0, SEEK_SET);

		uchar* aData = new uchar[aFileSize];

		p_fread(aData, 1, aFileSize, aFP);
		p_fclose(aFP);

		theBuffer->Clear();
		theBuffer->SetData(aData, aFileSize);

		if ((mRecordingDemoBuffer) && (!dontWriteToDemo))
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_FILE_READ, 5);
			mDemoBuffer.WriteNumBits(1, 1); // success
			mDemoBuffer.WriteLong(aFileSize);
			mDemoBuffer.WriteBytes(aData, aFileSize);
		}

		delete [] aData;

		return true;
	}
}

bool SexyAppBase::FileExists(const std::string& theFileName)
{
	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return true;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;

		DBG_ASSERTE(!mDemoIsShortCmd);
		DBG_ASSERTE(mDemoCmdNum == DEMO_FILE_EXISTS);

		bool success = mDemoBuffer.ReadNumBits(1, false) != 0;
		return success;
	}
	else
	{
		PFILE* aFP = p_fopen(theFileName.c_str(), "rb");

		if (mRecordingDemoBuffer)
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_FILE_EXISTS, 5);
			mDemoBuffer.WriteNumBits((aFP != NULL) ? 1 : 0, 1);
		}

		if (aFP == NULL)
			return false;

		p_fclose(aFP);
		return true;
	}
}

bool SexyAppBase::EraseFile(const std::string& theFileName)
{
	if (mPlayingDemoBuffer)
		return true;

	return unlink(theFileName.c_str()) != 0;
}

void SexyAppBase::SEHOccured()
{
	SetMusicVolume(0);
	mSEHOccured = true;
	EnforceCursor();
}

std::string SexyAppBase::GetGameSEHInfo()
{
	int aSecLoaded = (GetTickCount() - mTimeLoaded) / 1000;

	char aTimeStr[16];
	snprintf(aTimeStr, sizeof(aTimeStr), "%02d:%02d:%02d", (aSecLoaded/60/60), (aSecLoaded/60)%60, aSecLoaded%60);

	char aThreadIdStr[16];
	snprintf(aThreadIdStr, sizeof(aThreadIdStr), "%X", mPrimaryThreadId);

	std::string anInfoString =
		"Product: " + mProdName + "\r\n" +
		"Version: " + mProductVersion + "\r\n";

	anInfoString +=
		"Time Loaded: " + std::string(aTimeStr) + "\r\n"
		"Fullscreen: " + (mIsWindowed ? std::string("No") : std::string("Yes")) + "\r\n"
		"Primary ThreadId: " + aThreadIdStr + "\r\n";

	return anInfoString;
}

void SexyAppBase::GetSEHWebParams(DefinesMap* theDefinesMap)
{
}

void SexyAppBase::ShutdownHook()
{
}

void SexyAppBase::Shutdown()
{
	if (/*(mPrimaryThreadId != 0) && (GetCurrentThreadId() != mPrimaryThreadId) */ false)
	{
		mLoadingFailed = true;
	}
	else if (!mShutdown)
	{
		mExitToTop = true;
		mShutdown = true;
		ShutdownHook();

		if (mPlayingDemoBuffer)
		{
			//if the music/sfx volume is 0, then it means that in playback
			//someone pressed the "S" key to mute sounds (or that the
			//sound volume was set to 0 in the first place). Out of politeness,
			//return the system sound volume to what it last was in the game.
			SetMusicVolume(mDemoMusicVolume);
			SetSfxVolume(mDemoSfxVolume);
		}

		// Blah
		while (mCursorThreadRunning)
		{
			Sleep(10);
		}

		if (mMusicInterface != NULL)
			mMusicInterface->StopAllMusic();

		RestoreScreenResolution();

		if (mReadFromRegistry)
			WriteToRegistry();

		ImageLib::CloseJPEG2000();
	}
}

void SexyAppBase::RestoreScreenResolution()
{
}

void SexyAppBase::DoExit(int theCode)
{
	RestoreScreenResolution();
	exit(theCode);
}

void SexyAppBase::UpdateFrames()
{
	mUpdateCount++;

	if (!mMinimized)
	{
		if (mWidgetManager->UpdateFrame())
			++mFPSDirtyCount;
	}

	if (mMusicInterface)
		mMusicInterface->Update();
	CleanSharedImages();
}

void SexyAppBase::DoUpdateFramesF(float theFrac)
{
	if ((mVSyncUpdates) && (!mMinimized))
		mWidgetManager->UpdateFrameF(theFrac);
}

bool SexyAppBase::DoUpdateFrames()
{
	SEXY_AUTO_PERF("SexyAppBase::DoUpdateFrames");

	if (gScreenSaverActive)
		return false;

	if (mPlayingDemoBuffer)
	{
		if ((mLoadingThreadCompleted) && (!mLoaded) && (mDemoLoadingComplete))
		{
			mLoaded = true;
			mYieldMainThread = false;
			LoadingThreadCompleted();

			mLoadingThread.Join();
			mLoadingThreadStarted = false;
		}

		// Hrrm not sure why we check (mUpdateCount != mLastDemoUpdateCnt) here
		if ((mLoaded == mDemoLoadingComplete) && (mUpdateCount != mLastDemoUpdateCnt))
		{
			UpdateFrames();
			return true;
		}

		return false;
	}
	else
	{
		if ((mLoadingThreadCompleted) && (!mLoaded))
		{
			mLoaded = true;
			mYieldMainThread = false;
			LoadingThreadCompleted();

			mLoadingThread.Join();
			mLoadingThreadStarted = false;
		}

		UpdateFrames();
		return true;
	}
}

bool gIsFailing = false;

void SexyAppBase::Redraw(Rect* theClipRect)
{
	SEXY_AUTO_PERF("SexyAppBase::Redraw");

	// Do mIsDrawing check because we could enter here at a bad time if any windows messages
	//  are processed during WidgetManager->Draw
	if ((mIsDrawing) || (mShutdown))
		return;

	if (gScreenSaverActive)
		return;

	if (mDDInterface)
		mDDInterface->Redraw();
	mFPSFlipCount++;
}

///////////////////////////// FPS Stuff
//static PerfTimer gFPSTimer;
static int gFrameCount;
static int gFPSDisplay;
static bool gForceDisplay = false;
static void CalculateFPS()
{
	gFrameCount++;
}

///////////////////////////// FPS Stuff to draw mouse coords
static void FPSDrawCoords(int theX, int theY)
{
}

///////////////////////////// Demo TimeLeft Stuff
//static DDImage* gDemoTimeLeftImage = NULL;
static void CalculateDemoTimeLeft()
{
}

static void UpdateScreenSaverInfo(DWORD theTick)
{
	if (gSexyAppBase->IsScreenSaver() || !gSexyAppBase->mIsPhysWindowed)
		return;
}

bool SexyAppBase::DrawDirtyStuff()
{
	SEXY_AUTO_PERF("SexyAppBase::DrawDirtyStuff");
	MTAutoDisallowRand aDisallowRand;

	if (gIsFailing) // just try to reinit
	{
		//Redraw(NULL);
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;
		return false;
	}

	if (mShowFPS)
	{
		switch(mShowFPSMode)
		{
			case FPS_ShowFPS: CalculateFPS(); break;
			case FPS_ShowCoords:
				if (mWidgetManager!=NULL)
					FPSDrawCoords(mWidgetManager->mLastMouseX, mWidgetManager->mLastMouseY);
				break;
		}

		if (mPlayingDemoBuffer)
			CalculateDemoTimeLeft();
	}

	DWORD aStartTime = GetTickCount();

	// Update user input and screen saver info
	static DWORD aPeriodicTick = 0;
	if (aStartTime-aPeriodicTick > 1000)
	{
		aPeriodicTick = aStartTime;
		UpdateScreenSaverInfo(aStartTime);
	}

	if (gScreenSaverActive)
	{
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;
		return false;
	}

	mDDInterface->Update();

	mIsDrawing = true;
	mLastDrawnTime = Sexy::GetTickCount();
	bool drewScreen = mWidgetManager->DrawScreen();
	mIsDrawing = false;

	mDDInterface->Update();

	if ((drewScreen || (aStartTime - mLastDrawTick >= 1000) || (mCustomCursorDirty)) &&
	    ((int) (aStartTime - mNextDrawTick) >= 0))
	{
		mLastDrawWasEmpty = false;

		if (drewScreen)
			mDrawCount++;
		else
			mWidgetManager->MarkDirtyFull();

		DWORD aMidTime = GetTickCount();

		if (drewScreen)
		{
			mFPSCount++;
			mFPSTime += aMidTime - aStartTime;

			mDrawTime += aMidTime - aStartTime;
		}


#if 0
		if (mShowFPS)
		{
			Graphics g(mDDInterface->GetScreenImage());
			g.DrawImage(gFPSImage,mWidth-gFPSImage->GetWidth()-10,mHeight-gFPSImage->GetHeight()-10);

			if (mPlayingDemoBuffer)
				g.DrawImage(gDemoTimeLeftImage,mWidth-gDemoTimeLeftImage->GetWidth()-10,mHeight-gFPSImage->GetHeight()-gDemoTimeLeftImage->GetHeight()-15);
		}

		if (mWaitForVSync && mIsPhysWindowed && mSoftVSyncWait)
		{
			DWORD aTick = GetTickCount();
			if (aTick-mLastDrawTick < mDDInterface->mMillisecondsPerFrame)
				Sleep(mDDInterface->mMillisecondsPerFrame - (aTick-mLastDrawTick));
		}
#endif
		DWORD aPreScreenBltTime = GetTickCount();
		mLastDrawTick = aPreScreenBltTime;

		if (drewScreen)
		{
			Redraw(NULL);
		}
		else
		{
			Event evt;

			evt.type = EVENT_EXPOSE;
			evt.flags = 0;
			mInputManager->PushEvent(evt);
		}

		// This is our one UpdateFTimeAcc if we are vsynched
		UpdateFTimeAcc();

		DWORD aEndTime = GetTickCount();

		if (drewScreen)
			mScreenBltTime = aEndTime - aPreScreenBltTime;

		static DWORD aLastUpdateStats;
		if (aEndTime - aLastUpdateStats >= 500) // Show FPS about every 5 seconds
		{
			uint32 aTickNow = GetTickCount();
			aLastUpdateStats = aTickNow;

			//printf("FPSCount: %d FlipCount: %d\n", mFPSCount, mFPSFlipCount);
			//printf("FPSTime: %u elasped %u\n", mFPSTime, aTickNow - mFPSStartTick);
			PerformanceStats& stats = mPerformanceStats;

			stats.mTheoreticalFPS = mFPSCount * 1000.0f / mFPSTime;
			stats.mFPS = mFPSFlipCount * 1000.0f / std::max((aTickNow - mFPSStartTick), 1U);
			stats.mDirtyRate = mFPSDirtyCount * 1000 / std::max((aTickNow - mFPSStartTick), 1U);

			if (mFPSTime >= 2000)
			{
				printf("Theoretical FPS: %.2f\n", stats.mTheoreticalFPS);
				printf("Actual      FPS: %.2f\n", stats.mFPS);
				printf("Dirty Rate     : %d\n", stats.mDirtyRate);

				mFPSTime = 0;
				mFPSCount = 0;
				mFPSFlipCount = 0;
				mFPSStartTick = aTickNow;
				mFPSDirtyCount = 0;
			}
		}

		if ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
		{
			int aTotalTime = aEndTime - aStartTime;

			mNextDrawTick += 35 + std::max(aTotalTime, 15);

			if ((int) (aEndTime - mNextDrawTick) >= 0)
				mNextDrawTick = aEndTime;

			/* printf ("Next Draw Time: %d\n", mNextDrawTick); */
		}
		else
		{
			mNextDrawTick = aEndTime;
		}

		mHasPendingDraw = false;
		mCustomCursorDirty = false;

		return true;
	}
	else
	{
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;
		return false;
	}
}

void SexyAppBase::LogScreenSaverError(const std::string &theError)
{
}

void SexyAppBase::BeginPopup()
{
#if 0
	if (!mIsPhysWindowed)
	{
		if (mDDInterface && mDDInterface->mDD)
		{
			mDDInterface->mDD->FlipToGDISurface();
			mNoDefer = true;
		}
	}
#endif
}

void SexyAppBase::EndPopup()
{
	if (!mIsPhysWindowed)
		mNoDefer = false;

	ClearUpdateBacklog();
	ClearKeysDown();

	if (mWidgetManager->mDownButtons)
		mWidgetManager->DoMouseUps();

}

#if 0
int SexyAppBase::MsgBox(const std::string& theText, const std::string& theTitle, int theFlags)
{
//	if (mDDInterface && mDDInterface->mDD)
//		mDDInterface->mDD->FlipToGDISurface();
	if (IsScreenSaver())
	{
		LogScreenSaverError(theText);
		return IDOK;
	}

	BeginPopup();
	//int aResult = MessageBoxA(mHWnd, theText.c_str(), theTitle.c_str(), theFlags);
	EndPopup();

	return aResult;
}

int SexyAppBase::MsgBox(const Sexy::WString& theText, const Sexy::WString& theTitle, int theFlags)
{
//	if (mDDInterface && mDDInterface->mDD)
//		mDDInterface->mDD->FlipToGDISurface();
	if (IsScreenSaver())
	{
		LogScreenSaverError(WStringToString(theText));
		return IDOK;
	}

	BeginPopup();
	//int aResult = MessageBoxW(mHWnd, theText.c_str(), theTitle.c_str(), theFlags);
	EndPopup();

	return aResult;
}
#endif

void SexyAppBase::Popup(const std::string& theString)
{
	BeginPopup();
	if (!mShutdown)
		printf("Popup: %s\n", theString.c_str());
	EndPopup();
}

void SexyAppBase::Popup(const Sexy::WString& theString)
{
	BeginPopup();
	if (!mShutdown)
		printf("Popup: %s\n",
		       std::string(theString.begin(),
				   theString.end()).c_str());
	EndPopup();
}

void SexyAppBase::SafeDeleteWidget(Widget* theWidget)
{
	WidgetSafeDeleteInfo aWidgetSafeDeleteInfo;
	aWidgetSafeDeleteInfo.mUpdateAppDepth = mUpdateAppDepth;
	aWidgetSafeDeleteInfo.mWidget = theWidget;
	mSafeDeleteList.push_back(aWidgetSafeDeleteInfo);
}

static int DemoJumpToTime()
{
	return 0;
}

static void ToggleDemoSoundVolume()
{
	if (gSexyAppBase->GetMusicVolume() == 0.0)
		gSexyAppBase->SetMusicVolume(gSexyAppBase->mDemoMusicVolume);
	else
	{
		gSexyAppBase->mDemoMusicVolume = gSexyAppBase->mMusicVolume;
		gSexyAppBase->SetMusicVolume(0.0);
	}

	if (gSexyAppBase->GetSfxVolume() == 0.0)
		gSexyAppBase->SetSfxVolume(gSexyAppBase->mDemoSfxVolume);
	else
	{
		gSexyAppBase->mDemoSfxVolume = gSexyAppBase->mSfxVolume;
		gSexyAppBase->SetSfxVolume(0.0);
	}
}

void SexyAppBase::HandleNotifyGameMessage(int theType, int theParam)
{
}

void SexyAppBase::RehupFocus()
{
	bool wantHasFocus = mActive && !mMinimized;

	if (wantHasFocus != mHasFocus)
	{
		mHasFocus = wantHasFocus;

		if (mHasFocus)
		{
			if (mMuteOnLostFocus)
				Unmute(true);

			mWidgetManager->GotFocus();
			GotFocus();
		}
		else
		{
			if (mMuteOnLostFocus)
				Mute(true);

			mWidgetManager->LostFocus();
			LostFocus();

			mWidgetManager->DoMouseUps();
		}
	}
}

void SexyAppBase::ClearKeysDown()
{
	if (mWidgetManager != NULL) // fix stuck alt-key problem
	{
		for (int aKeyNum = 0; aKeyNum < 0xFF; aKeyNum++)
			mWidgetManager->mKeyDown[aKeyNum] = false;
	}
	mCtrlDown = false;
	mAltDown = false;
}

void SexyAppBase::WriteDemoTimingBlock()
{
	// Demo writing functions can only be called from the main thread and after SexyAppBase::Init
	//DBG_ASSERTE(GetCurrentThreadId() == mPrimaryThreadId);

	while (mUpdateCount - mLastDemoUpdateCnt > 15)
	{
		mDemoBuffer.WriteNumBits(15, 4);
		mLastDemoUpdateCnt += 15;

		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_IDLE, 5);
		mDemoCmdOrder++;
	}

	mDemoBuffer.WriteNumBits(mUpdateCount - mLastDemoUpdateCnt, 4);
	mLastDemoUpdateCnt = mUpdateCount;
	mDemoCmdOrder++;
}

int aNumBigMoveMessages = 0;
int aNumSmallMoveMessages = 0;
int aNumTimerMessages = 0;

bool SexyAppBase::PrepareDemoCommand(bool required)
{
	if (mDemoNeedsCommand)
	{
		mDemoCmdBitPos = mDemoBuffer.mReadBitPos;

		mLastDemoUpdateCnt += mDemoBuffer.ReadNumBits(4, false);

		mDemoIsShortCmd = mDemoBuffer.ReadNumBits(1, false) == 1;

		if (mDemoIsShortCmd)
			mDemoCmdNum = mDemoBuffer.ReadNumBits(1, false);
		else
			mDemoCmdNum = mDemoBuffer.ReadNumBits(5, false);

		mDemoNeedsCommand = false;

		mDemoCmdOrder++;
	}

	DBG_ASSERTE((mUpdateCount == mLastDemoUpdateCnt) || (!required));

	return mUpdateCount == mLastDemoUpdateCnt;
}

void SexyAppBase::ProcessDemo()
{
	if (mPlayingDemoBuffer)
	{
		// At end of demo buffer?  How dare you!
		DBG_ASSERTE(!mDemoBuffer.AtEnd());

		while ((!mShutdown) && (mUpdateCount == mLastDemoUpdateCnt) && (!mDemoBuffer.AtEnd()))
		{
			if (PrepareDemoCommand(false))
			{
				mDemoNeedsCommand = true;

				if (mDemoIsShortCmd)
				{
					switch (mDemoCmdNum)
					{
					case 0:
						{
							int aDeltaX = mDemoBuffer.ReadNumBits(6, true);
							int aDeltaY = mDemoBuffer.ReadNumBits(6, true);
							mLastDemoMouseX += aDeltaX;
							mLastDemoMouseY += aDeltaY;

							mWidgetManager->MouseMove(mLastDemoMouseX, mLastDemoMouseY);
						}
						break;
					case 1:
						{
							bool down = mDemoBuffer.ReadNumBits(1, false) != 0;
							int aBtnCount = mDemoBuffer.ReadNumBits(3, true);

							if (down)
								mWidgetManager->MouseDown(mLastDemoMouseX, mLastDemoMouseY, aBtnCount);
							else
								mWidgetManager->MouseUp(mLastDemoMouseX, mLastDemoMouseY, aBtnCount);
						}
						break;
					}
				}
				else
				{
					switch (mDemoCmdNum)
					{
					case DEMO_MOUSE_POSITION:
						{
							mLastDemoMouseX = mDemoBuffer.ReadNumBits(12, false);
							mLastDemoMouseY = mDemoBuffer.ReadNumBits(12, false);

							mWidgetManager->MouseMove(mLastDemoMouseX, mLastDemoMouseY);
						}
						break;
					case DEMO_ACTIVATE_APP:
						{
							mActive = mDemoBuffer.ReadNumBits(1, false) != 0;

							RehupFocus();

							if ((mActive) && (!mIsWindowed))
								mWidgetManager->MarkAllDirty();

							if ((mIsOpeningURL) && (!mActive))
								URLOpenSucceeded(mOpeningURL);
						}
						break;
					case DEMO_SIZE:
						{
							bool isMinimized = mDemoBuffer.ReadBoolean();

							if ((!mShutdown) && (isMinimized != mMinimized))
							{
								mMinimized = isMinimized;

								// We don't want any sounds (or music) playing while its minimized
								if (mMinimized)
									Mute(true);
								else
								{
									Unmute(true);
									mWidgetManager->MarkAllDirty();
								}
							}

							RehupFocus();
						}
						break;
					case DEMO_MOUSE_WHEEL:
						{
							int aScroll = mDemoBuffer.ReadNumBits(8, true);
							mWidgetManager->MouseWheel(aScroll);
						}
						break;
					case DEMO_KEY_DOWN:
						{
							KeyCode aKeyCode = (KeyCode) mDemoBuffer.ReadNumBits(8, false);
							mWidgetManager->KeyDown(aKeyCode);
						}
						break;
					case DEMO_KEY_UP:
						{
							KeyCode aKeyCode = (KeyCode) mDemoBuffer.ReadNumBits(8, false);
							mWidgetManager->KeyUp(aKeyCode);
						}
						break;
					case DEMO_KEY_CHAR:
						{
							int sizeMult = (int)mDemoBuffer.ReadNumBits(1, false) + 1; // will be 1 for single, 2 for double
							SexyChar aChar = (SexyChar) mDemoBuffer.ReadNumBits(8*sizeMult, false);
							mWidgetManager->KeyChar(aChar);
						}
						break;
					case DEMO_CLOSE:
						Shutdown();
						break;
					case DEMO_MOUSE_ENTER:
						mMouseIn = true;
						EnforceCursor();
						break;
					case DEMO_MOUSE_EXIT:
						mWidgetManager->MouseExit(mLastDemoMouseX, mLastDemoMouseY);
						mMouseIn = false;
						EnforceCursor();
						break;
					case DEMO_LOADING_COMPLETE:
						mDemoLoadingComplete = true;
						break;
					case DEMO_VIDEO_DATA:
						mIsWindowed = mDemoBuffer.ReadBoolean();
						mSyncRefreshRate = mDemoBuffer.ReadByte();
						break;
					case DEMO_IDLE:
						break;
					default:
						DBG_ASSERTE("Invalid Demo Command" == 0);
						break;
					}
				}
			}
		}
	}
}

void SexyAppBase::ShowMemoryUsage()
{
	mLastTime = GetTickCount();
}

bool SexyAppBase::IsAltKeyUsed(int wParam)
{
	int aChar = tolower(wParam);
	switch (aChar)
	{
		case 13: // alt-enter
		case 'r':
			return true;
		default:
			return false;
	}
}

bool SexyAppBase::DebugKeyDown(int theKey)
{
	return false;
}

bool SexyAppBase::DebugKeyDownAsync(int theKey, bool ctrlDown, bool altDown)
{
	return false;
}

void SexyAppBase::CloseRequestAsync()
{
}

// Why did I defer messages?  Oh, incase a dialog comes up such as a crash
//  it won't keep crashing and stuff
bool SexyAppBase::ProcessDeferredMessages(bool singleMessage)
{
	return false;
}

void SexyAppBase::Done3dTesting()
{
}

// return file name that you want to upload
std::string	SexyAppBase::NotifyCrashHook()
{
	return "";
}

MemoryImage* SexyAppBase::CreateCursorFromAndMask(unsigned char * data, unsigned char * mask,
						  int width, int height)
{
	uint32* bits;
	MemoryImage* anImage;

	DBG_ASSERT ((width % 8 == 0) && (height % 8 == 0));
	anImage = new MemoryImage(this);
	anImage->Create (width, height);

	bits = anImage->GetBits();
	if (!bits)
	{
		delete anImage;
		return NULL;
	}

	int i, j;
	for (i = 0; i < (height * width) / 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if ((*data) & (1 << (7 - j)))
				*bits = 0;
			else
				*bits = 0xffU << 24;
			if ((*mask) & (1 << (7 - j)))
				*bits |= 0xffU << 16 | 0xffU << 8 | 0xffU;
			bits++;
		}
		data++;
		mask++;
	}
	anImage->BitsChanged();

	return anImage;
}

void SexyAppBase::InitVideoDriver()
{
#ifdef _WIN32_WCE
	std::string driver("auto");
#else
	char* video_driver = getenv ("SEXY_VIDEO_DRIVER");
	std::string driver(video_driver ? video_driver : "auto");
#endif
	VideoDriver* aVideoDriver = dynamic_cast<VideoDriver*>
		(VideoDriverFactory::GetVideoDriverFactory ()->Find (driver));
	if (aVideoDriver == NULL && driver != "auto")
	{
		printf("Video driver \'%s\' isn't available.\n", driver.c_str());
		aVideoDriver = dynamic_cast<VideoDriver*>
			(VideoDriverFactory::GetVideoDriverFactory ()->Find ());
	}
	if (!aVideoDriver)
	{
		printf("Video driver isn't available.\n");
		DoExit (1);
	}
	DBG_ASSERT (aVideoDriver != NULL);
	mDDInterface = aVideoDriver->Create(this);
	if (!mDDInterface)
	{
		printf("Couldn't initialize video driver.\n");
		DoExit (1);
	}
	mDDInterface->mApp = this;

	if (mIs3D && !Is3DAccelerationSupported())
		mIs3D = false;
	if (mIsWindowed && !mDDInterface->CanWindowed())
		mIsWindowed = false;
}

void SexyAppBase::MakeWindow()
{
	bool fullInit = true;
	mWidgetManager->mImage = NULL;
	if (mDDInterface)
	{
		if (mDDInterface->CanReinit())
			fullInit = false;
		else
			DeleteExtraImageData();
	}
	else
	{
		InitVideoDriver();
	}
	InitDDInterface();
	mWidgetManager->mImage =
		dynamic_cast<MemoryImage*>(mDDInterface->GetScreenImage());

	if (fullInit)
		ReInitImages();

	mMouseX = 0;
	mMouseY = 0;
	if (!mHandCursor)
		mHandCursor =  CreateCursorFromAndMask(gFingerCursorData,
						       gFingerCursorData + sizeof(gFingerCursorData) / 2,
						       32, 32);
	mHandCursorHot = Point(11, 4);

	if (!mDraggingCursor)
		mDraggingCursor = CreateCursorFromAndMask(gDraggingCursorData,
							  gDraggingCursorData + sizeof(gDraggingCursorData) / 2,
							  32, 32);
	mDraggingCursorHot = Point(15, 10);

	if (!mArrowCursor)
		mArrowCursor = CreateCursorFromAndMask(gArrowCursorData,
						       gArrowCursorData + sizeof(gArrowCursorData) / 2,
						       32, 32);
	mArrowCursorHot = Point(0, 0);
	EnforceCursor();
}

void SexyAppBase::DeleteNativeImageData()
{
	AutoCrit anAutoCrit(mCritSect);
	MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
	while (anItr != mMemoryImageSet.end())
	{
		MemoryImage* aMemoryImage = *anItr;
		aMemoryImage->DeleteNativeData();
		++anItr;
	}
}

void SexyAppBase::DeleteExtraImageData()
{
	AutoCrit anAutoCrit(mDDInterface->mCritSect);
	MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
	while (anItr != mMemoryImageSet.end())
	{
		MemoryImage* aMemoryImage = *anItr;
		aMemoryImage->DeleteExtraBuffers();
		++anItr;
	}
}

void SexyAppBase::Evict3DImageData(DWORD theMemSize)
{
	static MTRand aRand;

	AutoCrit anAutoCrit(mCritSect);

	// be smart
	MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
	while (anItr != mMemoryImageSet.end() && theMemSize)
	{
		MemoryImage* aMemoryImage = *anItr;
		if (mDDInterface && aMemoryImage->mTexMemSize &&
		    (aMemoryImage->mBits || aMemoryImage->mColorIndices) &&
		    mLastDrawnTime - aMemoryImage->mDrawnTime > 10000)
		{
			if (aMemoryImage->mTexMemSize < theMemSize)
				theMemSize -= aMemoryImage->mTexMemSize;
			else
				theMemSize = 0;
			mDDInterface->RemoveImageData(aMemoryImage);
			assert (aMemoryImage->mTexMemSize == 0);
		}
		++anItr;
	}

	if (!theMemSize)
		return;

	// try hard
	anItr = mMemoryImageSet.begin();
	while (anItr != mMemoryImageSet.end() && theMemSize)
	{
		MemoryImage* aMemoryImage = *anItr;
		if (mDDInterface && aMemoryImage->mTexMemSize &&
		    (aMemoryImage->mBits || aMemoryImage->mColorIndices))
		{
			if (aMemoryImage->mTexMemSize < theMemSize)
				theMemSize -= aMemoryImage->mTexMemSize;
			else
				theMemSize = 0;
			mDDInterface->RemoveImageData(aMemoryImage);
			assert (aMemoryImage->mTexMemSize == 0);
		}
		++anItr;
	}

	if (theMemSize)
		abort ();
}

void SexyAppBase::ReInitImages()
{
	MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
	while (anItr != mMemoryImageSet.end())
	{
		MemoryImage* aMemoryImage = *anItr;
		aMemoryImage->ReAttach(mDDInterface);
		aMemoryImage->ReInit();
		++anItr;
	}
}


void SexyAppBase::LoadingThreadProc()
{
}

void SexyAppBase::LoadingThreadCompleted()
{
}

void SexyAppBase::LoadingThreadProcStub(void *theArg)
{
	SexyAppBase* aSexyApp = (SexyAppBase*) theArg;

	aSexyApp->LoadingThreadProc();

#if defined(SEXY_DEBUG) || defined(DEBUG)
	printf("Resource Loading Time: %u\n",
	       (GetTickCount() - aSexyApp->mTimeLoaded));
#endif

	aSexyApp->mLoadingThreadCompleted = true;
}

void SexyAppBase::StartLoadingThread()
{
	if (!mLoadingThreadStarted)
	{
		mYieldMainThread = true;
		mLoadingThreadStarted = true;
		mLoadingThread = Thread::Create(LoadingThreadProcStub, this);
	}
}
void SexyAppBase::CursorThreadProc()
{
}

void SexyAppBase::CursorThreadProcStub(void *theArg)
{
}

void SexyAppBase::StartCursorThread()
{
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed, bool is3d, bool force)
{
	if (mForceFullscreen || (mDDInterface && !mDDInterface->CanWindowed()))
		wantWindowed = false;

	// Set 3d acceleration preference
	Set3DAcclerated(is3d, false);
	if (mIsWindowed == wantWindowed && mIs3D == Is3DAccelerated() && !force)
		return;

	// Always make the app windowed when playing demos, in order to
	//  make it easier to track down bugs.  We place this after the
	//  sanity check just so things get re-initialized and stuff
	//if (mPlayingDemoBuffer)
	//	wantWindowed = true;
	mIsWindowed = wantWindowed;

	MakeWindow();

	// We need to do this check to allow IE to get focus instead of
	//  stealing it away for ourselves
	mLastTime = GetTickCount();
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed)
{
	SwitchScreenMode(wantWindowed, Is3DAccelerated());
}

void SexyAppBase::SwitchScreenMode()
{
	SwitchScreenMode(mIsWindowed, Is3DAccelerated(), true);
}

void SexyAppBase::SetAlphaDisabled(bool isDisabled)
{
	if (mAlphaDisabled != isDisabled)
	{
		mAlphaDisabled = isDisabled;
		//mDDInterface->SetVideoOnlyDraw(mAlphaDisabled);
		//mWidgetManager->mImage = mDDInterface->GetScreenImage();
		mWidgetManager->MarkAllDirty();
	}
}

void SexyAppBase::EnforceCursor()
{
	if (!mDDInterface)
		return;

	bool wantSysCursor = true;

	if (!mMouseIn)
	{
		mDDInterface->SetCursorImage(mArrowCursor,
					     mArrowCursorHot.mX,
					     mArrowCursorHot.mY);
		mDDInterface->EnableCursor(true);

		if (mDDInterface->SetCursorImage(NULL))
			mCustomCursorDirty = true;
	}
	else if (mCursorEnabled)
	{
		if ((mCursorImages[mCursorNum] == NULL) ||
                    ((!mCustomCursorsEnabled) && (mCursorNum != CURSOR_CUSTOM)))
		{

			switch(mCursorNum) {
			case CURSOR_POINTER:
				mDDInterface->SetCursorImage(mArrowCursor,
							     mArrowCursorHot.mX,
							     mArrowCursorHot.mY);
				mDDInterface->EnableCursor(true);
				break;
			case CURSOR_HAND:
				mDDInterface->SetCursorImage(mHandCursor,
							     mHandCursorHot.mX,
							     mHandCursorHot.mY);
				mDDInterface->EnableCursor(true);
				break;
			case CURSOR_DRAGGING:
				mDDInterface->SetCursorImage(mDraggingCursor,
							     mDraggingCursorHot.mX,
							     mDraggingCursorHot.mY);
				mDDInterface->EnableCursor(true);
				break;
			case CURSOR_NONE:
				mDDInterface->SetCursorImage(NULL);
				mDDInterface->EnableCursor(false);
				break;
			}
			mCustomCursorDirty = true;
		}
		else
		{
			int hotX, hotY;

			hotX = mCursorImages[mCursorNum]->GetWidth () / 2;
			hotY = mCursorImages[mCursorNum]->GetHeight () / 2;
			if (mDDInterface->SetCursorImage(mCursorImages[mCursorNum],
							 hotX, hotY))
				mCustomCursorDirty = true;

			mDDInterface->EnableCursor(true);
			wantSysCursor = false;
		}
	}
	else
	{
		mDDInterface->EnableCursor(false);
	}

	if (wantSysCursor != mSysCursor)
		mSysCursor = wantSysCursor;
}

void SexyAppBase::ProcessSafeDeleteList()
{
	MTAutoDisallowRand aDisallowRand;

	WidgetSafeDeleteList::iterator anItr = mSafeDeleteList.begin();
	while (anItr != mSafeDeleteList.end())
	{
		WidgetSafeDeleteInfo* aWidgetSafeDeleteInfo = &(*anItr);
		if (mUpdateAppDepth <= aWidgetSafeDeleteInfo->mUpdateAppDepth)
		{
			delete aWidgetSafeDeleteInfo->mWidget;
			anItr = mSafeDeleteList.erase(anItr);
		}
		else
			++anItr;
	}
}

void SexyAppBase::UpdateFTimeAcc()
{
	DWORD aCurTime = GetTickCount();

	if (mLastTimeCheck != 0)
	{
		int aDeltaTime = aCurTime - mLastTimeCheck;

		mUpdateFTimeAcc = std::min(mUpdateFTimeAcc + aDeltaTime, 200.0);

		if (mRelaxUpdateBacklogCount > 0)
			mRelaxUpdateBacklogCount = std::max(mRelaxUpdateBacklogCount - aDeltaTime, 0);
	}

	mLastTimeCheck = aCurTime;
}

//int aNumCalls = 0;
//DWORD aLastCheck = 0;

bool SexyAppBase::Process(bool allowSleep)
{
	/*DWORD aTimeNow = GetTickCount();
	if (aTimeNow - aLastCheck >= 10000)
	{
		OutputDebugString(StrFormat("FUpdates: %d\n", aNumCalls).c_str());
		aLastCheck = aTimeNow;
		aNumCalls = 0;
	}*/

	if (mLoadingFailed)
		Shutdown();

	bool isVSynched = (!mPlayingDemoBuffer) && (mVSyncUpdates) && (!mLastDrawWasEmpty) && (!mVSyncBroken) &&
		((!mIsPhysWindowed) || (mIsPhysWindowed && mWaitForVSync && !mSoftVSyncWait));
	double aFrameFTime;
	double anUpdatesPerUpdateF;

	if (mVSyncUpdates)
	{
		aFrameFTime = (1000.0 / mSyncRefreshRate) / mUpdateMultiplier;
		anUpdatesPerUpdateF = (float) (1000.0 / (mFrameTime * mSyncRefreshRate));
	}
	else
	{
		aFrameFTime = mFrameTime / mUpdateMultiplier;
		anUpdatesPerUpdateF = 1.0;
	}

	// Do we need to fast forward?
	if (mPlayingDemoBuffer)
	{
		if (mUpdateCount < mFastForwardToUpdateNum || mFastForwardToMarker)
		{
			if (!mDemoMute && !mFastForwardStep)
			{
				mDemoMute = true;
				Mute(true);
			}

			static DWORD aTick = GetTickCount();
			while (mUpdateCount < mFastForwardToUpdateNum || mFastForwardToMarker)
			{
				ClearUpdateBacklog();
				int aLastUpdateCount = mUpdateCount;

				// Actual updating code below
				//////////////////////////////////////////////////////////////////////////

				bool hadRealUpdate = DoUpdateFrames();

				if (hadRealUpdate)
				{
					mPendingUpdatesAcc += anUpdatesPerUpdateF;
					mPendingUpdatesAcc -= 1.0;
					ProcessSafeDeleteList();

					// Process any extra updates
					while (mPendingUpdatesAcc >= 1.0)
					{
						// These should just be IDLE commands we have to clear out
						ProcessDemo();

						bool hasRealUpdate = DoUpdateFrames();
						DBG_ASSERTE(hasRealUpdate);

						if (!hasRealUpdate)
							break;

						ProcessSafeDeleteList();
						mPendingUpdatesAcc -= 1.0;
					}

					DoUpdateFramesF((float) anUpdatesPerUpdateF);
					ProcessSafeDeleteList();
				}

				//////////////////////////////////////////////////////////////////////////

				// If the update count doesn't change, its because we are
				//  playing back a demo and need to read more
				if (aLastUpdateCount == mUpdateCount)
					return true;

				DWORD aNewTick = GetTickCount();
				if (aNewTick - aTick >= 1000 || mFastForwardStep) // let the app draw some
				{
					mFastForwardStep = false;
					aTick = GetTickCount();
					DrawDirtyStuff();
					return true;
				}
			}
		}

		if (mDemoMute)
		{
			mDemoMute = false;
			if (mSoundManager)
				mSoundManager->StopAllSounds();
			Unmute(true);
		}
	}

	// Make sure we're not paused
	if ((!mPaused) && (mUpdateMultiplier > 0))
	{
		uint32 aStartTime = GetTickCount();

		uint32 aCurTime = aStartTime;
		int aCumSleepTime = 0;

		// When we are VSynching, only calculate this FTimeAcc right after drawing

		if (!isVSynched)
			UpdateFTimeAcc();

		// mNonDrawCount is used to make sure we draw the screen at least
		// 10 times per second, even if it means we have to slow down
		// the updates to make it draw 10 times per second in "game time"

		bool didUpdate = false;

		if (mUpdateAppState == UPDATESTATE_PROCESS_1)
		{
			if ((++mNonDrawCount < (int) ceil(10*mUpdateMultiplier)) || (!mLoaded))
			{
				bool doUpdate = false;

				if (isVSynched)
				{
					// Synch'ed to vertical refresh, so update as soon as possible after draw
					doUpdate = (!mHasPendingDraw) || (mUpdateFTimeAcc >= (int) (aFrameFTime * 0.75));
				}
				else if (mUpdateFTimeAcc >= aFrameFTime)
				{
					doUpdate = true;
				}

				if (doUpdate)
				{
					// Do VSyncBroken test.  This test fails if we're in fullscreen and
					// "don't vsync" has been forced in Advanced settings up Display Properties
					if ((!mPlayingDemoBuffer) && (mUpdateMultiplier == 1.0))
					{
						mVSyncBrokenTestUpdates++;
						if (mVSyncBrokenTestUpdates >= (DWORD) ((1000+mFrameTime-1)/mFrameTime))
						{
							// It has to be running 33% fast to be "broken" (25% = 1/0.800)
							if (aStartTime - mVSyncBrokenTestStartTick <= 800)
							{
								// The test has to fail 3 times in a row before we decide that
								//  vsync is broken overall
								mVSyncBrokenCount++;
								if (mVSyncBrokenCount >= 3)
									mVSyncBroken = true;
							}
							else
								mVSyncBrokenCount = 0;

							mVSyncBrokenTestStartTick = aStartTime;
							mVSyncBrokenTestUpdates = 0;
						}
					}

					bool hadRealUpdate = DoUpdateFrames();
					if (hadRealUpdate)
						mUpdateAppState = UPDATESTATE_PROCESS_2;

					mHasPendingDraw = true;
					didUpdate = true;
				}
			}
		}
		else if (mUpdateAppState == UPDATESTATE_PROCESS_2)
		{
			mUpdateAppState = UPDATESTATE_PROCESS_DONE;

			mPendingUpdatesAcc += anUpdatesPerUpdateF;
			mPendingUpdatesAcc -= 1.0;
			ProcessSafeDeleteList();

			// Process any extra updates
			while (mPendingUpdatesAcc >= 1.0)
			{
				// These should just be IDLE commands we have to clear out
				ProcessDemo();

				++mNonDrawCount;
				bool hasRealUpdate = DoUpdateFrames();
				DBG_ASSERTE(hasRealUpdate);

				if (!hasRealUpdate)
					break;

				ProcessSafeDeleteList();
				mPendingUpdatesAcc -= 1.0;
			}

			//aNumCalls++;
			DoUpdateFramesF((float) anUpdatesPerUpdateF);
			ProcessSafeDeleteList();

			// Don't let mUpdateFTimeAcc dip below 0
			//  Subtract an extra 0.2ms, because sometimes refresh rates have some
			//  fractional component that gets truncated, and it's better to take off
			//  too much to keep our timing tending toward occuring right after
			//  redraws
			if (isVSynched)
				mUpdateFTimeAcc = std::max(mUpdateFTimeAcc - aFrameFTime - 0.2f, 0.0);
			else
				mUpdateFTimeAcc -= aFrameFTime;

			if (mRelaxUpdateBacklogCount > 0)
				mUpdateFTimeAcc = 0;

			didUpdate = true;
		}

		if (!didUpdate)
		{
			mUpdateAppState = UPDATESTATE_PROCESS_DONE;

			mNonDrawCount = 0;

			if (mHasPendingDraw)
			{
				DrawDirtyStuff();
			}
			else
			{
				// Let us take into account the time it took to draw dirty stuff
				int aTimeToNextFrame = (int) (aFrameFTime - mUpdateFTimeAcc);
				if (aTimeToNextFrame > 0)
				{
					if (!allowSleep)
						return false;

					// Wait till next processing cycle
					++mSleepCount;
					Sleep(aTimeToNextFrame);

					aCumSleepTime += aTimeToNextFrame;
				}
			}
		}

		if (mYieldMainThread)
		{
			// This is to make sure that the title screen doesn't take up any more than
			// 1/3 of the processor time

			uint32 anEndTime = GetTickCount();
			int anElapsedTime = (anEndTime - aStartTime) - aCumSleepTime;
			int aLoadingYieldSleepTime = std::min(250, (anElapsedTime * 2) - aCumSleepTime);

			if (aLoadingYieldSleepTime > 0)
			{
				if (!allowSleep)
					return false;

				Sleep(aLoadingYieldSleepTime);
			}
		}
	}

	ProcessSafeDeleteList();
	return true;
}

/*void SexyAppBase::DoMainLoop()
{
	Dialog* aDialog = NULL;
	if (theModalDialogId != -1)
	{
		aDialog = GetDialog(theModalDialogId);
		DBG_ASSERTE(aDialog != NULL);
		if (aDialog == NULL)
			return;
	}

	while (!mShutdown)
	{
		MSG msg;
		while ((PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) && (!mShutdown))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ProcessDemo();
		ProcessDeferredMessages();

		if ((aDialog != NULL) && (aDialog->mResult != -1))
			return;

		if (!mShutdown)
		{
			//++aCount;
			Process();
		}
	}
}*/

void SexyAppBase::DoMainLoop()
{
	while (!mShutdown)
	{
		if (mExitToTop)
			mExitToTop = false;
		UpdateApp();
	}
}

bool SexyAppBase::ProcessMessage(Event & event)
{
	if (event.flags & EVENT_FLAGS_INCOMPLETE)
	{
		if (mAccuEvents.size() && event.type == mAccuEvents[0].type)
		{
			mAccuEvents.push_back(event);
		}
		else if (event.type != EVENT_NONE)
		{
			mAccuEvents.push_back(event);
		}
		return true;
	}

	switch(event.type) {
	case EVENT_NONE:
		break;

	case EVENT_QUIT:
		Shutdown();
		break;

	case EVENT_MOUSE_BUTTON_PRESS:
		mAccuEvents.clear();

		mLastUserInputTick = mLastTimerTime;
		mMouseX = event.u.mouse.x;
		mMouseY = event.u.mouse.y;
		mWidgetManager->RemapMouse(mMouseX, mMouseY);

		if (event.u.mouse.button == 1)
			mWidgetManager->MouseDown(mMouseX, mMouseY, 1);
		else if (event.u.mouse.button == 2)
			mWidgetManager->MouseDown(mMouseX, mMouseY, -1);
		else if (event.u.mouse.button == 3)
			mWidgetManager->MouseDown(mMouseX, mMouseY, 3);
		break;

	case EVENT_MOUSE_BUTTON_RELEASE:
		mAccuEvents.clear();

		mMouseX = event.u.mouse.x;
		mMouseY = event.u.mouse.y;
		mWidgetManager->RemapMouse(mMouseX, mMouseY);

		if (event.u.mouse.button == 1)
			mWidgetManager->MouseUp(mMouseX, mMouseY, 1);
		else if (event.u.mouse.button == 2)
			mWidgetManager->MouseUp(mMouseX, mMouseY, -1);
		else if (event.u.mouse.button == 3)
			mWidgetManager->MouseUp(mMouseX, mMouseY, 3);
		break;

	case EVENT_KEY_DOWN:
		mAccuEvents.clear();

		mLastUserInputTick = mLastTimerTime;
		if (event.u.key.keyCode)
			mWidgetManager->KeyDown((KeyCode)event.u.key.keyCode);
		if (event.flags & EVENT_FLAGS_KEY_CHAR && event.u.key.keyChar)
			mWidgetManager->KeyChar((SexyChar)event.u.key.keyChar);

		break;

	case EVENT_KEY_UP:
		mAccuEvents.clear();

		mWidgetManager->KeyUp((KeyCode)event.u.key.keyCode);
		break;

	case EVENT_MOUSE_MOTION:
		mAccuEvents.clear();

		mLastUserInputTick = mLastTimerTime;

		if (!mHasFocus)
		{
			mActive = true;
			RehupFocus();
		}

		mMouseX = event.u.mouse.x;
		mMouseY = event.u.mouse.y;
		mWidgetManager->RemapMouse(mMouseX, mMouseY);

		mWidgetManager->MouseMove(mMouseX, mMouseY);

		if (!mMouseIn)
		{
			mMouseIn = true;
			EnforceCursor();
		}
		break;

	case EVENT_ACTIVE:
		mAccuEvents.clear();

		mActive = event.u.active.active;
		RehupFocus();
		break;

	case EVENT_EXPOSE:
		mAccuEvents.clear();

		mWidgetManager->MarkDirtyFull();
		break;

	case EVENT_TOUCH:
		mLastUserInputTick = mLastTimerTime;
		mAccuEvents.push_back(event);
		if (event.u.touch.state == TOUCH_DOWN)
			mWidgetManager->TouchDown(mAccuEvents);
		else if (event.u.touch.state == TOUCH_MOVE)
			mWidgetManager->TouchMove(mAccuEvents);
		else if (event.u.touch.state == TOUCH_UP)
			mWidgetManager->TouchUp(mAccuEvents);
		else if (event.u.touch.state == TOUCH_CANCEL)
			mWidgetManager->TouchCancel(mAccuEvents);
		mAccuEvents.clear();
		break;

	case EVENT_MINIMIZED:
	{
		bool isMinimized = event.u.minimized.minimized;

		if (!mShutdown && isMinimized != mMinimized)
		{
			mMinimized = isMinimized;

			// We don't want any sounds (or music) playing while its minimized
			if (mMinimized)
			{
				Mute(true);
			}
			else
			{
				Unmute(true);
				mWidgetManager->MarkAllDirty();
			}
		}

		RehupFocus();
		break;
	}
	default:
		mWidgetManager->UserEvent(event);
		break;
	}

	return true;
}

bool SexyAppBase::UpdateAppStep(bool* updated)
{
	if (updated != NULL)
		*updated = false;

	if (mExitToTop)
		return false;

	if (mUpdateAppState == UPDATESTATE_PROCESS_DONE)
		mUpdateAppState = UPDATESTATE_MESSAGES;

	mUpdateAppDepth++;

	// We update in two stages to avoid doing a Process if our loop termination
	//  condition has already been met by processing windows messages
	if (mUpdateAppState == UPDATESTATE_MESSAGES)
	{
		Sexy::Event event;

		mInputManager->Update ();
		while (mInputManager->PopEvent(event))
			ProcessMessage (event);
		ProcessDemo();
		if (!ProcessDeferredMessages(true))
		{
			mUpdateAppState = UPDATESTATE_PROCESS_1;
		}
	}
	else
	{
		// Process changes state by itself
		if (mStepMode)
		{
			if (mStepMode==2)
			{
				Sleep(mFrameTime);
				mUpdateAppState = UPDATESTATE_PROCESS_DONE; // skip actual update until next step
			}
			else
			{
				mStepMode = 2;
				DoUpdateFrames();
				DoUpdateFramesF(1.0f);
				DrawDirtyStuff();
			}
		}
		else
		{
			int anOldUpdateCnt = mUpdateCount;
			Process(mAllowSleep);
			if (updated != NULL)
				*updated = mUpdateCount != anOldUpdateCnt;
		}
	}

	mUpdateAppDepth--;

	return true;
}

bool SexyAppBase::UpdateApp()
{
	bool updated;
	for (;;)
	{
		if (!UpdateAppStep(&updated))
			return false;
		if (updated)
			return true;
	}
}

int SexyAppBase::InitDDInterface()
{
	if (!mDDInterface)
		return -1;

	mInputManager->Cleanup ();

	bool aCanReinit = mDDInterface->CanReinit();

	PreDDInterfaceInitHook();
	if (!aCanReinit)
		DeleteNativeImageData();

	if (aCanReinit)
	{
		if (!mDDInterface->Reinit())
		{
			printf("Reinitializing video driver failed.\n");
			DoExit (1);
		}
	}
	else if (mDDInterface->Init())
	{
		printf("Initializing video driver failed.\n");
		DoExit (1);
	}

	DemoSyncRefreshRate();

	mScreenBounds.mX = (mWidth - mDDInterface->mWidth) / 2;
	mScreenBounds.mY = (mHeight - mDDInterface->mHeight) / 2;
	mScreenBounds.mWidth = mDDInterface->mWidth;
	mScreenBounds.mHeight = mDDInterface->mHeight;
	mWidgetManager->Resize(mScreenBounds, mDDInterface->mPresentationRect);

	PostDDInterfaceInitHook();

	mInputManager->Init ();
	return 0;
}

void SexyAppBase::PreTerminate()
{
}

void SexyAppBase::Startup()
{
	if (mShutdown)
		return;

	StartCursorThread();

	if (mAutoStartLoadingThread)
		StartLoadingThread();

	DWORD aStartTime = GetTickCount();

	mRunning = true;
	mLastTime = aStartTime;
	mLastUserInputTick = aStartTime;
	mLastTimerTime = aStartTime;
}

void SexyAppBase::Start()
{
	if (mShutdown)
		return;

	Startup();
	DoMainLoop();
	Terminate();
}

void SexyAppBase::Terminate()
{
	ProcessSafeDeleteList();

	mRunning = false;

	WaitForLoadingThread();

#if 0
	char aString[256];
	sprintf(aString, "Seconds       = %g\r\n", (GetTickCount() - aStartTime) / 1000.0);
	OutputDebugStringA(aString);
	//sprintf(aString, "Count         = %d\r\n", aCount);
	//OutputDebugString(aString);
	sprintf(aString, "Sleep Count   = %d\r\n", mSleepCount);
	OutputDebugStringA(aString);
	sprintf(aString, "Update Count  = %d\r\n", mUpdateCount);
	OutputDebugStringA(aString);
	sprintf(aString, "Draw Count    = %d\r\n", mDrawCount);
	OutputDebugStringA(aString);
	sprintf(aString, "Draw Time     = %d\r\n", mDrawTime);
	OutputDebugStringA(aString);
	sprintf(aString, "Screen Blt    = %d\r\n", mScreenBltTime);
	OutputDebugStringA(aString);
	if (mDrawTime+mScreenBltTime > 0)
	{
		sprintf(aString, "Avg FPS       = %d\r\n", (mDrawCount*1000)/(mDrawTime+mScreenBltTime));
		OutputDebugStringA(aString);
	}

	timeEndPeriod(1);
#endif

	PreTerminate();

	WriteToRegistry();
}

bool SexyAppBase::CheckSignature(const Buffer& theBuffer, const std::string& theFileName)
{
	// Add your own signature checking code here
	return false;
}

bool SexyAppBase::LoadProperties(const std::string& theFileName, bool required, bool checkSig)
{
	Buffer aBuffer;
	if (!ReadBufferFromFile(theFileName, &aBuffer))
	{
		if (!required)
			return true;
		else
		{
			Popup(GetString("UNABLE_OPEN_PROPERTIES", "Unable to open properties file ") + StringToSexyString(theFileName));
			return false;
		}
	}
	if (checkSig)
	{
		if (!CheckSignature(aBuffer, theFileName))
		{
			Popup(GetString("PROPERTIES_SIG_FAILED", "Signature check failed on ") + StringToSexyString(theFileName + "'"));
			return false;
		}
	}

	PropertiesParser aPropertiesParser(this);

	// Load required language-file properties
	if (!aPropertiesParser.ParsePropertiesBuffer(aBuffer))
	{
		Popup(aPropertiesParser.GetErrorText());
		return false;
	}
	else
		return true;
}

bool SexyAppBase::LoadProperties()
{
	// Load required language-file properties
	return LoadProperties("properties/default.xml", true, false);
}

void SexyAppBase::LoadResourceManifest()
{
	if (!mResourceManager->ParseResourcesFile(mResourceManifest))
		ShowResourceError(true);
}

void SexyAppBase::ShowResourceError(bool doExit)
{
	Popup(mResourceManager->GetErrorText());
	if (doExit)
		DoExit(0);
}

bool SexyAppBase::GetBoolean(const std::string& theId)
{
	StringBoolMap::iterator anItr = mBoolProperties.find(theId);
	DBG_ASSERTE(anItr != mBoolProperties.end());

	if (anItr != mBoolProperties.end())
		return anItr->second;
	else
		return false;
}

bool SexyAppBase::GetBoolean(const std::string& theId, bool theDefault)
{
	StringBoolMap::iterator anItr = mBoolProperties.find(theId);

	if (anItr != mBoolProperties.end())
		return anItr->second;
	else
		return theDefault;
}

int SexyAppBase::GetInteger(const std::string& theId)
{
	StringIntMap::iterator anItr = mIntProperties.find(theId);
	DBG_ASSERTE(anItr != mIntProperties.end());

	if (anItr != mIntProperties.end())
		return anItr->second;
	else
		return false;
}

int SexyAppBase::GetInteger(const std::string& theId, int theDefault)
{
	StringIntMap::iterator anItr = mIntProperties.find(theId);

	if (anItr != mIntProperties.end())
		return anItr->second;
	else
		return theDefault;
}

double SexyAppBase::GetDouble(const std::string& theId)
{
	StringDoubleMap::iterator anItr = mDoubleProperties.find(theId);
	DBG_ASSERTE(anItr != mDoubleProperties.end());

	if (anItr != mDoubleProperties.end())
		return anItr->second;
	else
		return false;
}

double SexyAppBase::GetDouble(const std::string& theId, double theDefault)
{
	StringDoubleMap::iterator anItr = mDoubleProperties.find(theId);

	if (anItr != mDoubleProperties.end())
		return anItr->second;
	else
		return theDefault;
}

SexyString SexyAppBase::GetString(const std::string& theId)
{
	StringWStringMap::iterator anItr = mStringProperties.find(theId);
	DBG_ASSERTE(anItr != mStringProperties.end());

	if (anItr != mStringProperties.end())
		return std::string(anItr->second.begin(), anItr->second.end());
	else
		return "";
}

SexyString SexyAppBase::GetString(const std::string& theId, const SexyString& theDefault)
{
	StringWStringMap::iterator anItr = mStringProperties.find(theId);

	if (anItr != mStringProperties.end())
		return std::string(anItr->second.begin(), anItr->second.end());
	else
		return theDefault;
}


Sexy::WString SexyAppBase::GetWString(const std::string& theId)
{
	StringWStringMap::iterator anItr = mStringProperties.find(theId);
	DBG_ASSERTE(anItr != mStringProperties.end());

	if (anItr != mStringProperties.end())
		return anItr->second;
	else
		return Sexy::WString();
}

Sexy::WString SexyAppBase::GetWString(const std::string& theId, const Sexy::WString& theDefault)
{
	StringWStringMap::iterator anItr = mStringProperties.find(theId);

	if (anItr != mStringProperties.end())
		return anItr->second;
	else
		return theDefault;
}

StringVector SexyAppBase::GetStringVector(const std::string& theId)
{
	StringStringVectorMap::iterator anItr = mStringVectorProperties.find(theId);
	DBG_ASSERTE(anItr != mStringVectorProperties.end());

	if (anItr != mStringVectorProperties.end())
		return anItr->second;
	else
		return StringVector();
}

void SexyAppBase::SetString(const std::string& theId, const Sexy::WString& theValue)
{
	std::pair<StringWStringMap::iterator, bool> aPair = mStringProperties.insert(StringWStringMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}


void SexyAppBase::SetBoolean(const std::string& theId, bool theValue)
{
	std::pair<StringBoolMap::iterator, bool> aPair = mBoolProperties.insert(StringBoolMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

void SexyAppBase::SetInteger(const std::string& theId, int theValue)
{
	std::pair<StringIntMap::iterator, bool> aPair = mIntProperties.insert(StringIntMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

void SexyAppBase::SetDouble(const std::string& theId, double theValue)
{
	std::pair<StringDoubleMap::iterator, bool> aPair = mDoubleProperties.insert(StringDoubleMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

char* SexyAppBase::GetCmdLine()
{
#if defined(WIN32)
	mCmdLine = std::string(GetCommandLineA());
#elif defined(__linux__)
	FILE *fp = fopen ("/proc/self/cmdline", "r");
	char buffer[65536];
	size_t size;

	size = fread(buffer, 1, 63335, fp);
	for (int i = 0; i < size - 1; i++)
		if (buffer[i] == '\0')
			buffer[i] = ' ';
	buffer[63335] = '\0';
	fclose(fp);
	mCmdLine = std::string(buffer);
#else
	mCmdLine = "";
#endif

	//printf ("CmdLine: %s\n", mCmdLine.c_str());
	return (char*)mCmdLine.c_str();
}

void SexyAppBase::DoParseCmdLine()
{
	if (mArgc > 1 && mArgv)
	{
		ParseCmdLine(mArgc, mArgv);
	}
	else
	{
		char* aCmdLine = GetCmdLine();

		char* aCmdLinePtr = aCmdLine;
		if (aCmdLinePtr[0] == '"')
		{
			aCmdLinePtr = strchr(aCmdLinePtr + 1, '"');
			if (aCmdLinePtr != NULL)
				aCmdLinePtr++;
		}

		if (aCmdLinePtr != NULL)
		{
			aCmdLinePtr = strchr(aCmdLinePtr, ' ');
			if (aCmdLinePtr != NULL)
				ParseCmdLine(aCmdLinePtr+1);
		}
	}

	mCmdLineParsed = true;
}

void SexyAppBase::ParseCmdLine(int argc, char **argv)
{
	std::string aCurParamName;
	std::string aCurParamValue;

	for (int i = 1; i < argc; i++)
	{
		bool onValue = false;

		if (argv[i][0] == '-')
		{
			int j = 0;

			aCurParamName = "-";
			for (j = 1; argv[i][j]; j++)
			{
				char c = argv[i][j];
				if (c == '=')
					onValue = true;
				else if (onValue)
					aCurParamValue += c;

				else
					aCurParamName += c;
			}
		}
		if (!aCurParamName.empty() && (onValue || i < argc - 1))
		{
			if (!onValue && i < argc - 1)
				aCurParamValue = argv[++i];
			HandleCmdLineParam(aCurParamName, aCurParamValue);
		}
		aCurParamName.clear();
		aCurParamValue.clear();
	}
}

void SexyAppBase::ParseCmdLine(const std::string& theCmdLine)
{
	// Command line example:  -play -demofile="game demo.dmo"
	// Results in HandleCmdLineParam("-play", ""); HandleCmdLineParam("-demofile", "game demo.dmo");
	std::string aCurParamName;
	std::string aCurParamValue;

	int aSpacePos = 0;
	bool inQuote = false;
	bool onValue = false;

	for (int i = 0; i < (int) theCmdLine.length(); i++)
	{
		char c = theCmdLine[i];
		bool atEnd = false;

		if (c == '"')
		{
			inQuote = !inQuote;

			if (!inQuote)
				atEnd = true;
		}
		else if ((c == ' ') && (!inQuote))
			atEnd = true;
		else if (c == '=')
			onValue = true;
		else if (onValue)
			aCurParamValue += c;
		else
			aCurParamName += c;

		if (i == theCmdLine.length() - 1)
			atEnd = true;

		if (atEnd && !aCurParamName.empty())
		{
			HandleCmdLineParam(aCurParamName, aCurParamValue);
			aCurParamName = "";
			aCurParamValue = "";
			onValue = false;
		}
	}
}

static int GetMaxDemoFileNum(const std::string& theDemoPrefix, int theMaxToKeep, bool doErase)
{
	return 0;
}

void SexyAppBase::HandleCmdLineParam(const std::string& theParamName, const std::string& theParamValue)
{
}

void SexyAppBase::PreDisplayHook()
{
}

void SexyAppBase::PreDDInterfaceInitHook()
{
}

void SexyAppBase::PostDDInterfaceInitHook()
{
}

bool SexyAppBase::ChangeDirHook(const char *theIntendedPath)
{
	return false;
}

MusicInterface* SexyAppBase::CreateMusicInterface(HWND theWindow)
{
	return new MusicInterface;
}

void SexyAppBase::InitPropertiesHook()
{
}

void SexyAppBase::InitHook()
{
}

void SexyAppBase::InitSoundManager()
{
	if (mSoundManager == NULL)
	{
#ifdef _WIN32_WCE
		char* sound_driver = NULL;
#else
		char* sound_driver = getenv ("SEXY_SOUND_DRIVER");
#endif
		std::string driver(sound_driver ? sound_driver : "auto");
		SoundDriver* aSoundDriver = dynamic_cast<SoundDriver*>
			(SoundDriverFactory::GetSoundDriverFactory ()->Find (driver));
	        while (aSoundDriver)
		{
			mSoundManager = aSoundDriver->Create (this);
			if (mSoundManager)
			{
				mMusicInterface = aSoundDriver->CreateMusicInterface (this);
				break;
			}
			aSoundDriver = dynamic_cast<SoundDriver*>
				(SoundDriverFactory::GetSoundDriverFactory ()->FindNext (aSoundDriver));
		}
	}
	if (mSoundManager == NULL)
		mSoundManager = new DummySoundManager();
	if (mMusicInterface == NULL)
		mMusicInterface = new MusicInterface();
}

void SexyAppBase::SetCmdline(int argc, char **argv)
{
	mArgc = argc;
	mArgv = argv;
}

void SexyAppBase::Init()
{
	if (mShutdown)
		return;

	InitPropertiesHook();
	ReadFromRegistry();

	if (!mCmdLineParsed)
		DoParseCmdLine();

	if (IsScreenSaver())
		mOnlyAllowOneCopyToRun = false;

	// Change directory
	if (!ChangeDirHook(mChangeDirTo.c_str()))
		chdir(mChangeDirTo.c_str());

        //gPakInterface->AddPakFile("main.pak");
	p_addResource("main.pak", "zip");
	p_addResource("main.pak", "poppak");

	mRandSeed = GetTickCount();
	SRand(mRandSeed);

	// Set up demo recording stuff
	if (mPlayingDemoBuffer)
	{
		std::string anError;
		if (!ReadDemoBuffer(anError))
		{
			mPlayingDemoBuffer = false;
			Popup(anError);
			DoExit(0);
		}
	}


	srand(GetTickCount());

	// Let app do something before showing window, or switching to fullscreen mode
	// NOTE: Moved call to PreDisplayHook above mIsWindowed and GetSystemsMetrics
	// checks because the checks below use values that could change in PreDisplayHook.
	// PreDisplayHook must call mWidgetManager->Resize if it changes mWidth or mHeight.
	PreDisplayHook();

	mWidgetManager->Resize(Rect(0, 0, mWidth, mHeight), Rect(0, 0, mWidth, mHeight));
	MakeWindow();

	InitSoundManager();
	SetSfxVolume(mSfxVolume);
 	SetMusicVolume(mMusicVolume);

	InitHook();

	mInitialized = true;
}

void SexyAppBase::HandleGameAlreadyRunning()
{
	if(mOnlyAllowOneCopyToRun)
		DoExit(0);
}

void SexyAppBase::CopyToClipboard(const std::string& theString)
{
	if (mPlayingDemoBuffer)
		return;
}

std::string	SexyAppBase::GetClipboard()
{
	std::string			aString;
	return aString;
}

void SexyAppBase::SetCursor(int theCursorNum)
{
	mCursorNum = theCursorNum;
	EnforceCursor();
}

int SexyAppBase::GetCursor()
{
	return mCursorNum;
}

void SexyAppBase::EnableCustomCursors(bool enabled)
{
	mCustomCursorsEnabled = enabled;
	EnforceCursor();
}

Sexy::Image* SexyAppBase::CreateImage(int theWidth, int theHeight)
{
	Image* anImage = mDDInterface->CreateImage(this, theWidth, theHeight);
	return anImage;
}

Sexy::Image* SexyAppBase::GetImage(const std::string& theFileName, bool commitBits)
{
	ImageLib::Image* aLoadedImage = ImageLib::GetImage(theFileName, true);

	if (aLoadedImage == NULL)
		return NULL;

	MemoryImage* anImage = new MemoryImage(this);
	anImage->Create(aLoadedImage->GetWidth(), aLoadedImage->GetHeight());
	if (anImage->TakeBits(aLoadedImage->GetBits(), aLoadedImage->GetWidth(),
			      aLoadedImage->GetHeight(), commitBits))
		aLoadedImage->mBits = 0;
	anImage->mFilePath = theFileName;
	delete aLoadedImage;
	return anImage;
}

Sexy::Image* SexyAppBase::CreateCrossfadeImage(Sexy::Image* theImage1, const Rect& theRect1,
					       Sexy::Image* theImage2, const Rect& theRect2,
					       double theFadeFactor)
{
	MemoryImage* aMemoryImage1 = dynamic_cast<MemoryImage*>(theImage1);
	MemoryImage* aMemoryImage2 = dynamic_cast<MemoryImage*>(theImage2);

	if ((aMemoryImage1 == NULL) || (aMemoryImage2 == NULL))
		return NULL;

	if ((theRect1.mX < 0) || (theRect1.mY < 0) ||
		(theRect1.mX + theRect1.mWidth > theImage1->GetWidth()) ||
		(theRect1.mY + theRect1.mHeight > theImage1->GetHeight()))
	{
		DBG_ASSERTE("Crossfade Rect1 out of bounds");
		return NULL;
	}

	if ((theRect2.mX < 0) || (theRect2.mY < 0) ||
		(theRect2.mX + theRect2.mWidth > theImage2->GetWidth()) ||
		(theRect2.mY + theRect2.mHeight > theImage2->GetHeight()))
	{
		DBG_ASSERTE("Crossfade Rect2 out of bounds");
		return NULL;
	}

	int aWidth = theRect1.mWidth;
	int aHeight = theRect1.mHeight;

	MemoryImage * anImage = new MemoryImage(this);
	anImage->Create(aWidth, aHeight);

	uint32* aDestBits = anImage->GetBits();
	uint32* aSrcBits1 = aMemoryImage1->GetBits();
	uint32* aSrcBits2 = aMemoryImage2->GetBits();

	int aSrc1Width = aMemoryImage1->GetWidth();
	int aSrc2Width = aMemoryImage2->GetWidth();
	uint32 aMult = (int) (theFadeFactor*256);
	uint32 aOMM = (256 - aMult);

	for (int y = 0; y < aHeight; y++)
	{
		uint32* s1 = &aSrcBits1[(y+theRect1.mY)*aSrc1Width+theRect1.mX];
		uint32* s2 = &aSrcBits2[(y+theRect2.mY)*aSrc2Width+theRect2.mX];
		uint32* d = &aDestBits[y*aWidth];

		for (int x = 0; x < aWidth; x++)
		{
			uint32 p1 = *s1++;
			uint32 p2 = *s2++;

			//p1 = 0;
			//p2 = 0xFFFFFFFF;

			*d++ =
				((((p1 & 0x000000FF)*aOMM + (p2 & 0x000000FF)*aMult)>>8) & 0x000000FF) |
				((((p1 & 0x0000FF00)*aOMM + (p2 & 0x0000FF00)*aMult)>>8) & 0x0000FF00) |
				((((p1 & 0x00FF0000)*aOMM + (p2 & 0x00FF0000)*aMult)>>8) & 0x00FF0000) |
				((((p1 >> 24)*aOMM + (p2 >> 24)*aMult)<<16) & 0xFF000000);
		}
	}

	anImage->BitsChanged();

	return anImage;
}

void SexyAppBase::ColorizeImage(Image* theImage, const Color& theColor)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	if (aSrcMemoryImage == NULL)
		return;

	uint32* aBits;
	int aNumColors;

	if (aSrcMemoryImage->mColorTable == NULL)
	{
		aBits = aSrcMemoryImage->GetBits();
		aNumColors = theImage->GetWidth()*theImage->GetHeight();
	}
	else
	{
		aBits = aSrcMemoryImage->mColorTable;
		aNumColors = 256;
	}

	if ((theColor.mAlpha <= 255) && (theColor.mRed <= 255) &&
		(theColor.mGreen <= 255) && (theColor.mBlue <= 255))
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32 aColor = aBits[i];

			aBits[i] =
				((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
				((((aColor & 0x00FF0000) * theColor.mRed) >> 8) & 0x00FF0000) |
				((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00)|
				((((aColor & 0x000000FF) * theColor.mBlue) >> 8) & 0x000000FF);
		}
	}
	else
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32 aColor = aBits[i];

			int aAlpha = ((aColor >> 24) * theColor.mAlpha) / 255;
			int aRed = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
			int aGreen = (((aColor >> 8) & 0xFF) * theColor.mGreen) / 255;
			int aBlue = ((aColor & 0xFF) * theColor.mBlue) / 255;

			if (aAlpha > 255)
				aAlpha = 255;
			if (aRed > 255)
				aRed = 255;
			if (aGreen > 255)
				aGreen = 255;
			if (aBlue > 255)
				aBlue = 255;

			aBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
		}
	}

	aSrcMemoryImage->BitsChanged();
}

Image* SexyAppBase::CreateColorizedImage(Image* theImage, const Color& theColor)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	if (aSrcMemoryImage == NULL)
		return NULL;

	MemoryImage* anImage = new MemoryImage(this);
	anImage->Create(theImage->GetWidth(), theImage->GetHeight());

	uint32* aSrcBits;
	uint32* aDestBits;
	int aNumColors;

	if (aSrcMemoryImage->mColorTable == NULL)
	{
		aSrcBits = aSrcMemoryImage->GetBits();
		aDestBits = anImage->GetBits();
		aNumColors = theImage->GetWidth()*theImage->GetHeight();
	}
	else
	{
		aSrcBits = aSrcMemoryImage->mColorTable;
		aDestBits = anImage->mColorTable = new uint32[256];
		aNumColors = 256;

		anImage->mColorIndices = new uchar[anImage->mWidth*theImage->mHeight];
		memcpy(anImage->mColorIndices, aSrcMemoryImage->mColorIndices, anImage->mWidth*theImage->mHeight);
	}

	if ((theColor.mAlpha <= 255) && (theColor.mRed <= 255) &&
		(theColor.mGreen <= 255) && (theColor.mBlue <= 255))
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32 aColor = aSrcBits[i];

			aDestBits[i] =
				((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
				((((aColor & 0x00FF0000) * theColor.mRed) >> 8) & 0x00FF0000) |
				((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00)|
				((((aColor & 0x000000FF) * theColor.mBlue) >> 8) & 0x000000FF);
		}
	}
	else
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32 aColor = aSrcBits[i];

			int aAlpha = ((aColor >> 24) * theColor.mAlpha) / 255;
			int aRed = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
			int aGreen = (((aColor >> 8) & 0xFF) * theColor.mGreen) / 255;
			int aBlue = ((aColor & 0xFF) * theColor.mBlue) / 255;

			if (aAlpha > 255)
				aAlpha = 255;
			if (aRed > 255)
				aRed = 255;
			if (aGreen > 255)
				aGreen = 255;
			if (aBlue > 255)
				aBlue = 255;

			aDestBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
		}
	}

	anImage->BitsChanged();

	return anImage;
}

Image* SexyAppBase::CopyImage(Image* theImage, const Rect& theRect)
{
	MemoryImage* anImage=  new MemoryImage(this);

	anImage->Create(theRect.mWidth, theRect.mHeight);

	Graphics g(anImage);
	g.DrawImage(theImage, -theRect.mX, -theRect.mY);
	anImage->CopyAttributes(theImage);

	return anImage;
}

Image* SexyAppBase::CopyImage(Image* theImage)
{
	return CopyImage(theImage, Rect(0, 0, theImage->GetWidth(), theImage->GetHeight()));
}

void SexyAppBase::MirrorImage(Image* theImage)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	uint32* aSrcBits = aSrcMemoryImage->GetBits();

	int aPhysSrcWidth = aSrcMemoryImage->mWidth;
	for (int y = 0; y < aSrcMemoryImage->mHeight; y++)
	{
		uint32* aLeftBits = aSrcBits + (y * aPhysSrcWidth);
		uint32* aRightBits = aLeftBits + (aPhysSrcWidth - 1);

		for (int x = 0; x < (aPhysSrcWidth >> 1); x++)
		{
			uint32 aSwap = *aLeftBits;

			*(aLeftBits++) = *aRightBits;
			*(aRightBits--) = aSwap;
		}
	}

	aSrcMemoryImage->BitsChanged();
}

void SexyAppBase::FlipImage(Image* theImage)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	uint32* aSrcBits = aSrcMemoryImage->GetBits();

	int aPhysSrcHeight = aSrcMemoryImage->mHeight;
	int aPhysSrcWidth = aSrcMemoryImage->mWidth;
	for (int x = 0; x < aPhysSrcWidth; x++)
	{
		uint32* aTopBits    = aSrcBits + x;
		uint32* aBottomBits = aTopBits + (aPhysSrcWidth * (aPhysSrcHeight - 1));

		for (int y = 0; y < (aPhysSrcHeight >> 1); y++)
		{
			uint32 aSwap = *aTopBits;

			*aTopBits = *aBottomBits;
			aTopBits += aPhysSrcWidth;
			*aBottomBits = aSwap;
			aBottomBits -= aPhysSrcWidth;
		}
	}

	aSrcMemoryImage->BitsChanged();
}

void SexyAppBase::RotateImageHue(Sexy::MemoryImage *theImage, int theDelta)
{
	while (theDelta < 0)
		theDelta += 256;

	int aSize = theImage->mWidth * theImage->mHeight;
	uint32 *aPtr = theImage->GetBits();
	for (int i=0; i<aSize; i++)
	{
		uint32 aPixel = *aPtr;
		int alpha = aPixel&0xff000000;
		int r = (aPixel>>16)&0xff;
		int g = (aPixel>>8) &0xff;
		int b = aPixel&0xff;

		int maxval = std::max(r, std::max(g, b));
		int minval = std::min(r, std::min(g, b));
		int h = 0;
		int s = 0;
		int l = (minval+maxval)/2;
		int delta = maxval - minval;

		if (delta != 0)
		{
			s = (delta * 256) / ((l <= 128) ? (minval + maxval) : (512 - maxval - minval));

			if (r == maxval)
				h = (g == minval ? 1280 + (((maxval-b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
			else if (g == maxval)
				h = (b == minval ?  256 + (((maxval-r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
			else
				h = (r == minval ?  768 + (((maxval-g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));

			h /= 6;
		}

		h += theDelta;
		if (h >= 256)
			h -= 256;

		double v= (l < 128) ? (l * (255+s))/255 :
				(l+s-l*s/255);

		int y = (int) (2*l-v);

		int aColorDiv = (6 * h) / 256;
		int x = (int)(y+(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
		if (x > 255)
			x = 255;

		int z = (int) (v-(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
		if (z < 0)
			z = 0;

		switch (aColorDiv)
		{
			case 0: r = (int) v; g = x; b = y; break;
			case 1: r = z; g= (int) v; b = y; break;
			case 2: r = y; g= (int) v; b = x; break;
			case 3: r = y; g = z; b = (int) v; break;
			case 4: r = x; g = y; b = (int) v; break;
			case 5: r = (int) v; g = y; b = z; break;
			default: r = (int) v; g = x; b = y; break;
		}

		*aPtr++ = alpha | (r<<16) | (g << 8) | (b);

	}

	theImage->BitsChanged();
}

uint32 SexyAppBase::HSLToRGB(int h, int s, int l)
{
	int r;
	int g;
	int b;

	double v= (l < 128) ? (l * (255+s))/255 :
			(l+s-l*s/255);

	int y = (int) (2*l-v);

	int aColorDiv = (6 * h) / 256;
	int x = (int)(y+(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
	if (x > 255)
		x = 255;

	int z = (int) (v-(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
	if (z < 0)
		z = 0;

	switch (aColorDiv)
	{
		case 0: r = (int) v; g = x; b = y; break;
		case 1: r = z; g= (int) v; b = y; break;
		case 2: r = y; g= (int) v; b = x; break;
		case 3: r = y; g = z; b = (int) v; break;
		case 4: r = x; g = y; b = (int) v; break;
		case 5: r = (int) v; g = y; b = z; break;
		default: r = (int) v; g = x; b = y; break;
	}

	return 0xFF000000 | (r << 16) | (g << 8) | (b);
}

uint32 SexyAppBase::RGBToHSL(int r, int g, int b)
{
	int maxval = std::max(r, std::max(g, b));
	int minval = std::min(r, std::min(g, b));
	int hue = 0;
	int saturation = 0;
	int luminosity = (minval+maxval)/2;
	int delta = maxval - minval;

	if (delta != 0)
	{
		saturation = (delta * 256) / ((luminosity <= 128) ? (minval + maxval) : (512 - maxval - minval));

		if (r == maxval)
			hue = (g == minval ? 1280 + (((maxval-b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
		else if (g == maxval)
			hue = (b == minval ?  256 + (((maxval-r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
		else
			hue = (r == minval ?  768 + (((maxval-g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));

		hue /= 6;
	}

	return 0xFF000000 | (hue) | (saturation << 8) | (luminosity << 16);
}

void SexyAppBase::HSLToRGB(const uint32* theSource, uint32* theDest, int theSize)
{
	for (int i = 0; i < theSize; i++)
	{
		uint32 src = theSource[i];
		theDest[i] = (src & 0xFF000000) | (HSLToRGB((src & 0xFF), (src >> 8) & 0xFF, (src >> 16) & 0xFF) & 0x00FFFFFF);
	}
}

void SexyAppBase::RGBToHSL(const uint32* theSource, uint32* theDest, int theSize)
{
	for (int i = 0; i < theSize; i++)
	{
		uint32 src = theSource[i];
		theDest[i] = (src & 0xFF000000) | (RGBToHSL(((src >> 16) & 0xFF), (src >> 8) & 0xFF, (src & 0xFF)) & 0x00FFFFFF);
	}
}

void SexyAppBase::PrecacheAdditive(MemoryImage* theImage)
{
	theImage->GetRLAdditiveData(mDDInterface);
}

void SexyAppBase::PrecacheAlpha(MemoryImage* theImage)
{
	theImage->GetRLAlphaData();
}

void SexyAppBase::PrecacheNative(MemoryImage* theImage)
{
	theImage->GetNativeAlphaData(mDDInterface);
}


void SexyAppBase::PlaySample(int theSoundNum)
{
	if (!mSoundManager)
		return;

	SoundInstance* aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);
	if (aSoundInstance != NULL)
	{
		aSoundInstance->Play(false, true);
	}
}


void SexyAppBase::PlaySample(int theSoundNum, int thePan)
{
	if (!mSoundManager)
		return;

	SoundInstance* aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);
	if (aSoundInstance != NULL)
	{
		aSoundInstance->SetPan(thePan);
		aSoundInstance->Play(false, true);
	}
}

bool SexyAppBase::IsMuted()
{
	return mMuteCount > 0;
}

void SexyAppBase::Mute(bool autoMute)
{
	mMuteCount++;
	if (autoMute)
		mAutoMuteCount++;

	SetMusicVolume(mMusicVolume);
	SetSfxVolume(mSfxVolume);
}

void SexyAppBase::Unmute(bool autoMute)
{
	if (mMuteCount > 0)
	{
		mMuteCount--;
		if (autoMute)
			mAutoMuteCount--;
	}

	SetMusicVolume(mMusicVolume);
	SetSfxVolume(mSfxVolume);
}


double SexyAppBase::GetMusicVolume()
{
	return mMusicVolume;
}

void SexyAppBase::SetMusicVolume(double theVolume)
{
	mMusicVolume = theVolume;

	if (mMusicInterface != NULL)
		mMusicInterface->SetVolume((mMuteCount > 0) ? 0.0 : mMusicVolume);
}

double SexyAppBase::GetSfxVolume()
{
	return mSfxVolume;
}

void SexyAppBase::SetSfxVolume(double theVolume)
{
	mSfxVolume = theVolume;

	if (mSoundManager != NULL)
		mSoundManager->SetVolume((mMuteCount > 0) ? 0.0 : mSfxVolume);
}

double SexyAppBase::GetMasterVolume()
{
	if (!mSoundManager)
		return 0.0;
	return mSoundManager->GetMasterVolume();
}

void SexyAppBase::SetMasterVolume(double theMasterVolume)
{
	mSfxVolume = theMasterVolume;
	if (!mSoundManager)
		mSoundManager->SetMasterVolume(mSfxVolume);
}

void SexyAppBase::AddMemoryImage(MemoryImage* theMemoryImage)
{
	//AutoCrit anAutoCrit(mDDInterface->mCritSect);
	mMemoryImageSet.insert(theMemoryImage);
}

void SexyAppBase::RemoveMemoryImage(MemoryImage* theMemoryImage)
{
	//AutoCrit anAutoCrit(mDDInterface->mCritSect);
	MemoryImageSet::iterator anItr = mMemoryImageSet.find(theMemoryImage);
	if (anItr != mMemoryImageSet.end())
		mMemoryImageSet.erase(anItr);

	Remove3DData(theMemoryImage);
}

void SexyAppBase::Remove3DData(MemoryImage* theMemoryImage)
{
	if (mDDInterface)
	{
		mDDInterface->RemoveImageData(theMemoryImage);
	}
	else
	{
		theMemoryImage->DeleteExtraBuffers();
	}
}

bool SexyAppBase::Is3DAccelerated()
{
	if (mDDInterface)
		return mDDInterface->Is3DAccelerated();
	return false;
}

bool SexyAppBase::Is3DAccelerationSupported()
{
	if (mDDInterface)
		return mDDInterface->Is3DAccelerationSupported();
	return false;
}

bool SexyAppBase::Is3DAccelerationRecommended()
{
	if (mDDInterface)
		return mDDInterface->Is3DAccelerationRecommended();
	return false;
}

void SexyAppBase::DemoSyncRefreshRate()
{
}

void SexyAppBase::Set3DAcclerated(bool is3D, bool reinit)
{
	mIs3D = is3D;
	if (mIs3D && !Is3DAccelerationSupported())
		mIs3D = false;

	if (reinit)
		SwitchScreenMode();
}

SharedImageRef SexyAppBase::GetSharedImage(const std::string& theFileName, const std::string& theVariant, bool* isNew)
{
	std::string anUpperFileName = StringToUpper(theFileName);
	std::string anUpperVariant = StringToUpper(theVariant);

	std::pair<SharedImageMap::iterator, bool> aResultPair;
	SharedImageRef aSharedImageRef;

	{
		AutoCrit anAutoCrit(mDDInterface->mCritSect);
		aResultPair = mSharedImageMap.insert(SharedImageMap::value_type(SharedImageMap::key_type(anUpperFileName,
													 anUpperVariant),
										SharedImage()));
		aSharedImageRef = &aResultPair.first->second;
	}

	if (isNew != NULL)
		*isNew = aResultPair.second;

	if (aResultPair.second)
	{
		// Pass in a '!' as the first char of the file name to create a new image
		if ((theFileName.length() > 0) && (theFileName[0] == '!'))
			aSharedImageRef.mSharedImage->mImage = new MemoryImage(this);
		else
			aSharedImageRef.mSharedImage->mImage = GetImage(theFileName,false);
	}

	return aSharedImageRef;
}

void SexyAppBase::CleanSharedImages()
{
	mCritSect.Enter();
	if (mCleanupSharedImages)
	{
		// Delete shared images with reference counts of 0
		// This doesn't occur in ~SharedImageRef because sometimes we can not only access the image
		//  through the SharedImageRef returned by GetSharedImage, but also by calling GetSharedImage
		//  again with the same params -- so we can have instances where we do the 'final' deref on
		//  an image but immediately re-request it via GetSharedImage
		SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
		while (aSharedImageItr != mSharedImageMap.end())
		{
			SharedImage* aSharedImage = &aSharedImageItr->second;
			if (aSharedImage->mRefCount == 0)
			{
				Image *anImage = aSharedImage->mImage;
				mSharedImageMap.erase(aSharedImageItr++);

				mCritSect.Leave();
				delete anImage;
				mCritSect.Enter();
			}
			else
				++aSharedImageItr;
		}

		mCleanupSharedImages = false;
	}
	mCritSect.Leave();
}

void SexyAppBase::CheckControllerStatus()
{
	static unsigned int cookie = (unsigned int)-1;

	if (!mLoaded)
		return;

	if (mLoadingThreadStarted && !mLoadingThreadCompleted)
		return;

	unsigned int current = mInputManager->GetCookie();

	if (current == cookie)
		return;
	cookie = current;

	InputStatusInfo aStatus;

	mInputManager->GetStatus(aStatus);
	InputStatusChanged(&aStatus);
}

void SexyAppBase::InputStatusChanged(const InputStatusInfo *theInfo)
{
}

bool SexyAppBase::DrawOneFrame()
{
    int count;

    while (true)
    {
        count = mDrawCount;
        UpdateApp();
        if (mShutdown)
            return false;
        if (mDrawCount != count)
            break;
    }

    return true;
}

void SexyAppBase::PauseApp()
{
	if (!mDDInterface)
		return;

	Event event;

	event.type = EVENT_ACTIVE;
	event.flags = 0;
	event.id = 0;
	event.subid = 0;
	event.u.active.active = 0;
	ProcessMessage(event);

	AutoCrit anAutoCrit(mCritSect);

	// be smart
	MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
	while (anItr != mMemoryImageSet.end())
	{
		MemoryImage* aMemoryImage = *anItr;
		if (mDDInterface)
			mDDInterface->RemoveImageData(aMemoryImage);
		++anItr;
	}
}

void SexyAppBase::ResumeApp()
{
	if (!mDDInterface)
		return;

	Event event;

	event.type = EVENT_ACTIVE;
	event.flags = 0;
	event.id = 0;
	event.subid = 0;
	event.u.active.active = 1;
	ProcessMessage(event);
}

void SexyAppBase::SetLocale(const std::string& locale,
			    const std::string& encoding)
{
	Sexy::setLocale(locale.c_str());
	SexySetLocaleEncoding(encoding);

	std::string lenc = SexyGetLocaleEncoding();
	if (lenc == "UTF-8")
		Graphics::SetPreferedEncoding("UTF-8");
	else
		Graphics::SetPreferedEncoding("");
}

std::string SexyAppBase::GetLocale()
{
	const char* locale = Sexy::setLocale(0);
	if (!locale)
		return "";
	return locale;
}

std::string SexyAppBase::GetLocaleEncoding()
{
	return SexyGetLocaleEncoding();
}
