#ifndef __SEXYAPPBASE_H__
#define __SEXYAPPBASE_H__

#include "Common.h"
#include "Rect.h"
#include "Color.h"
#include "ButtonListener.h"
#include "DialogListener.h"
#include "Buffer.h"
#include "CritSect.h"
#include "NativeDisplay.h"
#include "SharedImage.h"
#include "Ratio.h"
#include "SexyThread.h"
#include "SexyTimer.h"
#include "Event.h"

namespace ImageLib
{
	class Image;
};

namespace Sexy
{

	class WidgetManager;
	class DDInterface;
	class Image;
	class DDImage;
	class Widget;
	class SoundManager;
	class MusicInterface;
	class MemoryImage;
	class HTTPTransfer;
	class Dialog;

	class ResourceManager;
	class InputManager;
	class RegistryInterface;
	struct InputStatusInfo;

	class WidgetSafeDeleteInfo
	{
	public:
		int	  mUpdateAppDepth;
		Widget*	  mWidget;
	};


	typedef std::list<WidgetSafeDeleteInfo> WidgetSafeDeleteList;
	typedef std::set<MemoryImage*> MemoryImageSet;
	typedef std::map<int, Dialog*> DialogMap;
	typedef std::list<Dialog*> DialogList;
#ifdef WIN32
	typedef std::list<MSG> WindowsMessageList;
#endif
	typedef std::vector<std::string> StringVector;
	//typedef std::basic_string<TCHAR> tstring; // string of TCHARs

	typedef std::map<std::string, SexyString> StringSexyStringMap;
	typedef std::map<std::string, std::string> StringStringMap;
	typedef std::map<std::string, Sexy::WString> StringWStringMap;
	typedef std::map<std::string, bool> StringBoolMap;
	typedef std::map<std::string, int> StringIntMap;
	typedef std::map<std::string, double> StringDoubleMap;
	typedef std::map<std::string, StringVector> StringStringVectorMap;
	typedef std::list<Event> EventList;
	typedef std::vector<Event> EventVector;

	enum
	{
		CURSOR_POINTER,
		CURSOR_HAND,
		CURSOR_DRAGGING,
		CURSOR_TEXT,
		CURSOR_CIRCLE_SLASH,
		CURSOR_SIZEALL,
		CURSOR_SIZENESW,
		CURSOR_SIZENS,
		CURSOR_SIZENWSE,
		CURSOR_SIZEWE,
		CURSOR_WAIT,
		CURSOR_NONE,
		CURSOR_CUSTOM,
		NUM_CURSORS
	};

	enum
	{
		DEMO_MOUSE_POSITION,
		DEMO_ACTIVATE_APP,
		DEMO_SIZE,
		DEMO_KEY_DOWN,
		DEMO_KEY_UP,
		DEMO_KEY_CHAR,
		DEMO_CLOSE,
		DEMO_MOUSE_ENTER,
		DEMO_MOUSE_EXIT,
		DEMO_LOADING_COMPLETE,
		DEMO_REGISTRY_GETSUBKEYS,
		DEMO_REGISTRY_READ,
		DEMO_REGISTRY_WRITE,
		DEMO_REGISTRY_ERASE,
		DEMO_FILE_EXISTS,
		DEMO_FILE_READ,
		DEMO_FILE_WRITE,
		DEMO_HTTP_RESULT,
		DEMO_SYNC,
		DEMO_ASSERT_STRING_EQUAL,
		DEMO_ASSERT_INT_EQUAL,
		DEMO_MOUSE_WHEEL,
		DEMO_HANDLE_COMPLETE,
		DEMO_VIDEO_DATA,
		DEMO_IDLE = 31
	};

	enum {
		FPS_ShowFPS,
		FPS_ShowCoords,
		Num_FPS_Types
	};

	enum
	{
		UPDATESTATE_MESSAGES,
		UPDATESTATE_PROCESS_1,
		UPDATESTATE_PROCESS_2,
		UPDATESTATE_PROCESS_DONE
	};

#ifdef WIN32
	typedef std::map<HANDLE, int> HandleToIntMap;
#endif

	struct PerformanceStats
	{
		float mTheoreticalFPS;
		float mFPS;
		int   mDirtyRate;

		PerformanceStats()
		{
			mTheoreticalFPS = 100.0f;
			mFPS = 100.0f;
			mDirtyRate = 100;
		}
	};

	class SexyAppBase : public ButtonListener, public DialogListener
	{
	public:

		uint32					mRandSeed;

		std::string				mCompanyName;
		std::string				mFullCompanyName;
		std::string				mProdName;
		SexyString				mTitle;
		std::string				mRegKey;
		std::string				mChangeDirTo;
		std::string                             mResourceManifest;

		int					mRelaxUpdateBacklogCount; // app doesn't try to catch up for this many frames
		int					mPreferredX;
		int					mPreferredY;
		int					mWidth;
		int					mHeight;
		int					mFullscreenBits;
		double					mMusicVolume;
		double					mSfxVolume;
		double					mDemoMusicVolume;
		double					mDemoSfxVolume;
		bool					mNoSoundNeeded;
		bool					mWantFMod;
		bool					mCmdLineParsed;
		bool					mSkipSignatureChecks;
		bool					mStandardWordWrap;
		bool					mbAllowExtendedChars;

#ifdef WIN32
		HANDLE					mMutex;
#endif

		bool					mOnlyAllowOneCopyToRun;
		UINT					mNotifyGameMessage;
		CritSect				mCritSect;
		bool					mBetaValidate;
		uchar					mAdd8BitMaxTable[512];
		WidgetManager*			        mWidgetManager;
		DialogMap				mDialogMap;
		DialogList				mDialogList;
		DWORD					mPrimaryThreadId;
		bool					mSEHOccured;
		bool					mShutdown;
		bool					mExitToTop;
		bool					mIsWindowed;
		bool					mIsPhysWindowed;
		bool					mFullScreenWindow; // uses ChangeDisplaySettings to run fullscreen with mIsWindowed true
		bool					mForceFullscreen;
		bool					mForceWindowed;
		bool					mInitialized;
		bool					mProcessInTimer;
		DWORD					mTimeLoaded;
		HWND					mHWnd;
		HWND					mInvisHWnd;
		bool					mIsScreenSaver;
		bool					mAllowMonitorPowersave;
#ifdef WIN32
		WindowsMessageList		        mDeferredMessages;
#endif
		bool					mNoDefer;
		bool					mFullScreenPageFlip;
		bool					mTabletPC;
		NativeDisplay*			        mDDInterface;
		bool					mAlphaDisabled;
		MusicInterface*			        mMusicInterface;
		bool					mReadFromRegistry;
		std::string				mRegisterLink;
		std::string				mProductVersion;
		Image*					mCursorImages[NUM_CURSORS];
		Point                                   mCursorHots[NUM_CURSORS];
		bool                                    mCursorEnabled;
#ifdef WIN32
		HCURSOR					mOverrideCursor;
#endif
		bool					mIsOpeningURL;
		bool					mShutdownOnURLOpen;
		std::string				mOpeningURL;
		DWORD					mOpeningURLTime;
		DWORD					mLastTimerTime;
		DWORD					mLastBigDelayTime;
		double					mUnmutedMusicVolume;
		double					mUnmutedSfxVolume;
		int						mMuteCount;
		int						mAutoMuteCount;
		bool					mDemoMute;
		bool					mMuteOnLostFocus;
		MemoryImageSet			        mMemoryImageSet;
		SharedImageMap			        mSharedImageMap;
		bool					mCleanupSharedImages;

		int					mNonDrawCount;
		int					mFrameTime;

		bool					mIsDrawing;
		bool					mLastDrawWasEmpty;
		bool					mHasPendingDraw;
		double					mPendingUpdatesAcc;
		double					mUpdateFTimeAcc;
		DWORD					mLastTimeCheck;
		DWORD					mLastTime;
		DWORD					mLastUserInputTick;

		int					mSleepCount;
		int					mDrawCount;
		int					mUpdateCount;
		int					mUpdateAppState;
		int					mUpdateAppDepth;
		double					mUpdateMultiplier;
		bool                                    mAllowSleep;
		bool					mPaused;
		int					mFastForwardToUpdateNum;
		bool					mFastForwardToMarker;
		bool					mFastForwardStep;
		DWORD					mLastDrawTick;
		DWORD					mNextDrawTick;
		int					mStepMode;  // 0 = off, 1 = step, 2 = waiting for step
		DWORD                                   mLastDrawnTime;

		int					mCursorNum;
		SoundManager*			        mSoundManager;
		MemoryImage*                            mArrowCursor;
		MemoryImage*				mHandCursor;
		MemoryImage*			        mDraggingCursor;
		Point                                   mArrowCursorHot;
		Point                                   mHandCursorHot;
		Point                                   mDraggingCursorHot;

		int                                     mMouseX;
		int                                     mMouseY;

		WidgetSafeDeleteList	                mSafeDeleteList;
		bool					mMouseIn;
		bool					mRunning;
		bool					mActive;
		bool					mMinimized;
		bool					mPhysMinimized;
		bool					mIsDisabled;
		bool					mHasFocus;
		int					mDrawTime;
		uint32					mFPSStartTick;
		int				        mFPSFlipCount;
		int					mFPSDirtyCount;
		int					mFPSTime;
		int					mFPSCount;
		bool					mShowFPS;
		int					mShowFPSMode;
		int					mScreenBltTime;
		Sexy::Thread                            mLoadingThread;
		bool					mAutoStartLoadingThread;
		bool					mLoadingThreadStarted;
		bool					mLoadingThreadCompleted;
		bool					mLoaded;
		bool					mYieldMainThread;
		bool					mLoadingFailed;
		bool					mCursorThreadRunning;
		bool					mSysCursor;
		bool					mCustomCursorsEnabled;
		bool					mCustomCursorDirty;
		bool					mLastShutdownWasGraceful;
		bool					mIsWideWindow;

		int					mNumLoadingThreadTasks;
		int					mCompletedLoadingThreadTasks;

		// For recording/playback of program control
		bool					mRecordingDemoBuffer;
		bool					mPlayingDemoBuffer;
		bool					mManualShutdown;
		std::string			        mDemoPrefix;
		std::string			        mDemoFileName;
		Buffer					mDemoBuffer;
		int					mDemoLength;
		int					mLastDemoMouseX;
		int					mLastDemoMouseY;
		int					mLastDemoUpdateCnt;
		bool					mDemoNeedsCommand;
		bool					mDemoIsShortCmd;
		int					mDemoCmdNum;
		int					mDemoCmdOrder;
		int					mDemoCmdBitPos;
		bool					mDemoLoadingComplete;
#ifdef WIN32
		HandleToIntMap			        mHandleToIntMap; // For waiting on handles
#endif
		int					mCurHandleNum;

		typedef std::pair<std::string, int>     DemoMarker;
		typedef std::list<DemoMarker>           DemoMarkerList;
		DemoMarkerList			        mDemoMarkerList;

		bool					mDebugKeysEnabled;
		bool					mEnableMaximizeButton;
		bool					mCtrlDown;
		bool					mAltDown;

		int					mSyncRefreshRate;
		bool					mVSyncUpdates;
		bool					mVSyncBroken;
		int					mVSyncBrokenCount;
		DWORD					mVSyncBrokenTestStartTick;
		DWORD					mVSyncBrokenTestUpdates;
		bool					mWaitForVSync;
		bool					mSoftVSyncWait;
		bool					mUserChanged3DSetting;
		bool					mAutoEnable3D;
		bool                                    mIs3D;
		bool					mTest3D;
		DWORD					mMinVidMemory3D;
		DWORD					mRecommendedVidMemory3D;

		bool					mWidescreenAware;
		Rect					mScreenBounds;
		bool					mEnableWindowAspect;
		Ratio					mWindowAspect;

		StringWStringMap		        mStringProperties;
		StringBoolMap			        mBoolProperties;
		StringIntMap			        mIntProperties;
		StringDoubleMap			        mDoubleProperties;
		StringStringVectorMap	                mStringVectorProperties;
		ResourceManager*		        mResourceManager;

		InputManager*                           mInputManager;
		RegistryInterface*                      mRegistryInterface;

		EventVector                             mAccuEvents;
		std::string                             mCmdLine;
		int                                     mArgc;
		char**                                  mArgv;

		PerformanceStats                        mPerformanceStats;

#ifdef ZYLOM
		uint					mZylomGameId;
#endif

#ifdef WIN32
		LONG					mOldWndProc;
#endif

	protected:
		void					RehupFocus();
		void					ClearKeysDown();
		virtual bool                            ProcessMessage(Event &event);
		bool					ProcessDeferredMessages(bool singleMessage);
		void					UpdateFTimeAcc();
		virtual bool			        Process(bool allowSleep = true);
		virtual void			        UpdateFrames();
		virtual bool			        DoUpdateFrames();
		virtual void			        DoUpdateFramesF(float theFrac);
		virtual void			        MakeWindow();
		virtual void			        EnforceCursor();
		virtual void			        ReInitImages();

	public:
		virtual void			        DeleteNativeImageData();
		virtual void			        DeleteExtraImageData();
		virtual void                            Evict3DImageData(DWORD theMemSize);

	protected:
		// Loading thread methods
		virtual void			        LoadingThreadCompleted();
		static void				LoadingThreadProcStub(void *theArg);

		// Cursor thread methods
		void					CursorThreadProc();
		static void				CursorThreadProcStub(void *theArg);
		void					StartCursorThread();

		void					WaitForLoadingThread();
		void					ProcessSafeDeleteList();
		void					RestoreScreenResolution();
		void					DoExit(int theCode);

		void					TakeScreenshot();
		void					DumpProgramInfo();
		void					ShowMemoryUsage();

		// Registry helpers
		bool					RegistryRead(const std::string& theValueName,
								     ulong * theType, uchar* theValue,
								     ulong* theLength);
		bool					RegistryWrite(const std::string& theValueName,
								      ulong theType, const uchar* theValue,
								      ulong theLength);

		// Demo recording helpers
		void					ProcessDemo();

	public:
		SexyAppBase();
		virtual ~SexyAppBase();

		// Common overrides:
		virtual MusicInterface*	                CreateMusicInterface(HWND theHWnd);
		virtual void			        InitHook();
		virtual void			        ShutdownHook();
		virtual void			        PreTerminate();
		virtual void			        LoadingThreadProc();
		virtual void			        WriteToRegistry();
		virtual void			        ReadFromRegistry();
		virtual Dialog*			        NewDialog(int theDialogId, bool isModal,
								  const SexyString& theDialogHeader,
								  const SexyString& theDialogLines,
								  const SexyString& theDialogFooter,
								  int theButtonMode);
		virtual void			        PreDisplayHook();

		// Public methods
		virtual void			        BeginPopup();
		virtual void			        EndPopup();
#if 0
		virtual int			        MsgBox(const std::string &theText,
							       const std::string &theTitle = "Message",
							       int theFlags = MB_OK);
		virtual int		                MsgBox(const Sexy::WString &theText,
							       const Sexy::WString &theTitle = L"Message",
							       int theFlags = MB_OK);
#endif
		virtual void			        Popup(const std::string& theString);
		virtual void			        Popup(const Sexy::WString& theString);
		virtual void			        LogScreenSaverError(const std::string &theError);
		virtual void			        SafeDeleteWidget(Widget* theWidget);

		virtual void			        URLOpenFailed(const std::string& theURL);
		virtual void			        URLOpenSucceeded(const std::string& theURL);
		virtual bool			        OpenURL(const std::string& theURL,
								bool shutdownOnOpen = false);
		virtual std::string		        GetProductVersion(const std::string& thePath);

		virtual void			        SEHOccured();
		virtual std::string		        GetGameSEHInfo();
		virtual void			        GetSEHWebParams(DefinesMap* theDefinesMap);
		virtual void			        Shutdown();
		virtual void                            Cleanup(bool force = false);

		virtual void			        DoParseCmdLine();
		virtual void                            ParseCmdLine(int argc, char **argv);
		virtual void			        ParseCmdLine(const std::string& theCmdLine);
		virtual void			        HandleCmdLineParam(const std::string& theParamName,
									   const std::string& theParamValue);
		virtual void			        HandleNotifyGameMessage(int theType,
										int theParam);
		// for HWND_BROADCAST of mNotifyGameMessage (0-1000 are reserved for SexyAppBase for theType)
		virtual void			        HandleGameAlreadyRunning();

		virtual void                            PauseApp();
		virtual void                            ResumeApp();
		virtual void                            Startup();
		virtual void			        Start();
		virtual void                            Terminate();
		virtual void                            SetCmdline(int argc, char **argv);
		virtual void			        Init();
		virtual void			        PreDDInterfaceInitHook();
		virtual void			        PostDDInterfaceInitHook();
		virtual bool			        ChangeDirHook(const char *theIntendedPath);
		virtual void			        PlaySample(int theSoundNum);
		virtual void			        PlaySample(int theSoundNum, int thePan);

		virtual double			        GetMasterVolume();
		virtual double			        GetMusicVolume();
		virtual double			        GetSfxVolume();
		virtual bool			        IsMuted();

		virtual void			        SetMasterVolume(double theVolume);
		virtual void			        SetMusicVolume(double theVolume);
		virtual void			        SetSfxVolume(double theVolume);
		virtual void			        Mute(bool autoMute = false);
		virtual void			        Unmute(bool autoMute = false);

		void					StartLoadingThread();
		virtual double			        GetLoadingThreadProgress();

		void					CopyToClipboard(const std::string& theString);
		std::string				GetClipboard();

		void					SetCursor(int theCursorNum);
		int				        GetCursor();
		void					EnableCustomCursors(bool enabled);
		MemoryImage*                            CreateCursorFromAndMask(unsigned char * data,
										unsigned char * mask,
										int width, int height);
		Image*                                  CreateImage(int theWidth, int theHeight);
		virtual Image*		                GetImage(const std::string& theFileName,
								 bool commitBits = true);
		virtual SharedImageRef	                GetSharedImage(const std::string& theFileName,
								       const std::string& theVariant = "",
								       bool* isNew = NULL);

		void					CleanSharedImages();
		void					PrecacheAdditive(MemoryImage* theImage);
		void					PrecacheAlpha(MemoryImage* theImage);
		void					PrecacheNative(MemoryImage* theImage);
		void					SetCursorImage(int theCursorNum, Image* theImage);

		Image*				        CreateCrossfadeImage(Image* theImage1,
									     const Rect& theRect1,
									     Image* theImage2,
									     const Rect& theRect2,
									     double theFadeFactor);
		void					ColorizeImage(Image* theImage,
								      const Color& theColor);
		Image*				        CreateColorizedImage(Image* theImage,
									     const Color& theColor);
		Image*				        CopyImage(Image* theImage, const Rect& theRect);
		Image*				        CopyImage(Image* theImage);
		void					MirrorImage(Image* theImage);
		void					FlipImage(Image* theImage);
		void					RotateImageHue(Sexy::MemoryImage *theImage,
								       int theDelta);
		uint32					HSLToRGB(int h, int s, int l);
		uint32					RGBToHSL(int r, int g, int b);
		void					HSLToRGB(const uint32* theSource,
								 uint32* theDest, int theSize);
		void					RGBToHSL(const uint32* theSource,
								 uint32* theDest, int theSize);

		void					AddMemoryImage(MemoryImage* theMemoryImage);
		void					RemoveMemoryImage(MemoryImage* theMemoryImage);
		void					Remove3DData(MemoryImage* theMemoryImage);
		virtual void			        SwitchScreenMode();
		virtual void			        SwitchScreenMode(bool wantWindowed);
		virtual void			        SwitchScreenMode(bool wantWindowed,
									 bool is3d, bool force = false);
		virtual void			        SetAlphaDisabled(bool isDisabled);

		virtual Dialog*			        DoDialog(int theDialogId, bool isModal,
								 const SexyString& theDialogHeader,
								 const SexyString& theDialogLines,
								 const SexyString& theDialogFooter,
								 int theButtonMode);
		virtual Dialog*			         GetDialog(int theDialogId);
		virtual void			         AddDialog(int theDialogId, Dialog* theDialog);
		virtual void			         AddDialog(Dialog* theDialog);
		virtual bool			         KillDialog(int theDialogId,
								    bool removeWidget,
								    bool deleteWidget);
		virtual bool			         KillDialog(int theDialogId);
		virtual bool			         KillDialog(Dialog* theDialog);
		virtual int				 GetDialogCount();
		virtual void			        ModalOpen();
		virtual void			        ModalClose();
		virtual void			        DialogButtonPress(int theDialogId, int theButtonId);
		virtual void			        DialogButtonDepress(int theDialogId, int theButtonId);

		virtual void			        GotFocus();
		virtual void			        LostFocus();
		virtual bool			        IsAltKeyUsed(int wParam);

		virtual bool			        DebugKeyDown(int theKey);
		virtual bool			        DebugKeyDownAsync(int theKey, bool ctrlDown, bool altDown);
		virtual void			        CloseRequestAsync();
		bool					Is3DAccelerated();
		bool					Is3DAccelerationSupported();
		bool					Is3DAccelerationRecommended();
		void					DemoSyncRefreshRate();
		void					Set3DAcclerated(bool is3D, bool reinit = true);
		virtual void			        Done3dTesting();

		// return file name that you want to upload
		virtual std::string		        NotifyCrashHook();

		virtual bool			        CheckSignature(const Buffer& theBuffer,
								       const std::string& theFileName);
		virtual bool			        DrawDirtyStuff();
		virtual void			        Redraw(Rect* theClipRect);

		// Properties access methods
		bool					LoadProperties(const std::string& theFileName,
								       bool required, bool checkSig);
		bool					LoadProperties();
		virtual void			        InitPropertiesHook();

		// Resource access methods
		void					LoadResourceManifest();
		void					ShowResourceError(bool doExit = false);

		bool					GetBoolean(const std::string& theId);
		bool					GetBoolean(const std::string& theId, bool theDefault);
		int					GetInteger(const std::string& theId);
		int				        GetInteger(const std::string& theId, int theDefault);
		double					GetDouble(const std::string& theId);
		double					GetDouble(const std::string& theId, double theDefault);
		SexyString				GetString(const std::string& theId);
		SexyString				GetString(const std::string& theId,
								  const SexyString& theDefault);
		Sexy::WString				GetWString(const std::string& theId);
		Sexy::WString				GetWString(const std::string& theId,
								   const Sexy::WString& theDefault);
		StringVector			        GetStringVector(const std::string& theId);

		void					SetBoolean(const std::string& theId, bool theValue);
		void					SetInteger(const std::string& theId, int theValue);
		void					SetDouble(const std::string& theId, double theValue);
		void					SetString(const std::string& theId,
								  const Sexy::WString& theValue);

		// Demo access methods
		bool					PrepareDemoCommand(bool required);
		void					WriteDemoTimingBlock();
		void					WriteDemoBuffer();
		bool					ReadDemoBuffer(std::string &theError);//UNICODE
		void					DemoSyncBuffer(Buffer* theBuffer);
		void					DemoSyncString(std::string* theString);
		void					DemoSyncInt(int* theInt);
		void					DemoSyncBool(bool* theBool);
		void					DemoAssertStringEqual(const std::string& theString);
		void					DemoAssertIntEqual(int theInt);
		void					DemoAddMarker(const std::string& theString);

		// Registry access methods
		bool					RegistryGetSubKeys(const std::string& theKeyName, StringVector* theSubKeys);
		bool					RegistryReadString(const std::string& theValueName, std::string* theString);
		bool					RegistryReadInteger(const std::string& theValueName, int* theValue);
		bool					RegistryReadBoolean(const std::string& theValueName, bool* theValue);
		bool					RegistryReadData(const std::string& theValueName, uchar* theValue, ulong* theLength);
		bool					RegistryWriteString(const std::string& theValueName, const std::string& theString);
		bool					RegistryWriteInteger(const std::string& theValueName, int theValue);
		bool					RegistryWriteBoolean(const std::string& theValueName, bool theValue);
		bool					RegistryWriteData(const std::string& theValueName, const uchar* theValue, ulong theLength);
		bool					RegistryEraseKey(const SexyString& theKeyName);
		void					RegistryEraseValue(const SexyString& theValueName);

		// File access methods
		bool					WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer);
		bool					ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer, bool dontWriteToDemo = false);//UNICODE
		bool					WriteBytesToFile(const std::string& theFileName, const void *theData, unsigned long theDataLen);
		bool					FileExists(const std::string& theFileName);
		bool					EraseFile(const std::string& theFileName);

		// Misc methods
		virtual void				DoMainLoop();
		virtual bool				UpdateAppStep(bool* updated);
		virtual bool				UpdateApp();
		virtual void                            InitVideoDriver();
		virtual int				InitDDInterface();
		virtual void                            InitSoundManager();
		virtual bool                            DrawOneFrame();

		void					ClearUpdateBacklog(bool relaxForASecond = false);
		bool					IsScreenSaver();
		virtual bool				AppCanRestore();

		char*                                   GetCmdLine();
		void                                    CheckControllerStatus();
		virtual void                            InputStatusChanged(const InputStatusInfo *theInfo);

		static void                             SetLocale(const std::string& locale,
								  const std::string& encoding = "");
		static std::string                      GetLocale();
		static std::string                      GetLocaleEncoding();
	};

	extern SexyAppBase* gSexyAppBase;

};

#endif //__SEXYAPPBASE_H__
