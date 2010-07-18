#include "SexyAppBase.h"
#include "EAGLDisplay.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "SexyUtf8.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "InputManager.h"
#include "VideoDriverFactory.h"

#include <cstdio>

using namespace Sexy;

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
	BOOL quit;
	EAGLDisplay* dpy;
	NSTimer *timer;
}

- (id)initWithDisplay:(EAGLDisplay *)aDpy;
- (void)updateApp:(id)sender;
@end

@implementation AppDelegate

- (id)init
{
	self = [super init];
	if (self)
	{
		dpy = 0;
		timer = 0;
		quit = FALSE;
	}
	return (self);
}

- (id)initWithDisplay:(EAGLDisplay *)aDpy
{
	self = [super init];
	if (self)
	{
		dpy = aDpy;
		timer = 0;
		quit = FALSE;
	}
	return (self);
}

- (void)applicationDidFinishLaunching:(UIApplication *)anApp
{
	NSLog(@"applicationDidFinishLaunching");
	quit = FALSE;
	if (gSexyAppBase)
	{
		gSexyAppBase->Init();
		gSexyAppBase->Startup();
	}
}

- (BOOL)isQuit
{
	return quit;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	NSLog(@"applicationWillResignActive");
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	NSLog(@"applicationDidBecomeActive");
	if (!timer)
		timer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0 / 100.0)
							        target:self selector:@selector(updateApp:)
						                userInfo:nil repeats:TRUE];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	NSLog(@"applicatioWillTerminaten");
	quit = TRUE;

	if (timer)
		[timer invalidate];
	timer = nil;

	if (gSexyAppBase)
	{
		gSexyAppBase->Terminate();
		//delete gSexyAppBase;
	}
}
- (void)updateApp:(id)sender
{
	if (!gSexyAppBase)
		return;

	gSexyAppBase->UpdateApp();
}
@end

EAGLDisplay::EAGLDisplay (SexyAppBase* theApp)
	: GLDisplay (theApp)
{
	static bool first = false;

	if (!first)
	{
		NSString* path;

		first = true;

		[[NSAutoreleasePool alloc] init];

		UIApplication* application = [UIApplication sharedApplication];
		[application setDelegate:[[[AppDelegate alloc] initWithDisplay:this] autorelease]];

		NSBundle* bundle = [NSBundle mainBundle];
		[bundle loadNibNamed:@"MainMenu" owner:[application delegate] options:nil];

		path = [bundle bundlePath];
		NSLog (@"bounde path: %@", path);
		chdir([path UTF8String]);
		mApp->mChangeDirTo = std::string([path UTF8String]);
	}
	mView = NULL;
	mWindow = NULL;
	mContext = NULL;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mScreenImage = 0;

	InitKeyMap ();
}

void EAGLDisplay::InitKeyMap ()
{
}

EAGLDisplay::~EAGLDisplay ()
{
	Cleanup();
}

int EAGLDisplay::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLDisplay::Init();

	int index;
	bool result;
	CGRect displayRect;

	result = false;
	displayRect = [[UIScreen mainScreen] bounds];

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mDesktopWidth = displayRect.size.width;
	mDesktopHeight = displayRect.size.height;

	if (mApp->mIsWindowed && false)
	{
		mWindowWidth = mWidth;
		mWindowHeight = mHeight;
	}
	else
	{
		mWindowWidth = mDesktopWidth;
		mWindowHeight = mDesktopHeight;
	}
	mWindow = [[UIWindow alloc] initWithFrame:displayRect];
	if (!mWindow)
	  goto fail;
	mView = [[EAGLView alloc] initWithFrame:displayRect];
	if (!mView)
	  goto close_window;

	[mWindow addSubview:mView];
	[mView retain];

	[mWindow makeKeyAndVisible];

	CGRect frame;
	frame = [mView frame];
	NSLog(@"UIView: frame %.1fx%.1f", frame.size.width, frame.size.height);

	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mDisplayWidth;
	mPresentationRect.mHeight = mDisplayHeight;

	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	InitGL ();
	mTexBGRA = GL_FALSE;
	
	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	mInitCount++;
	mInitialized = true;

	return 0;
 close_window:
	[mWindow release];
 fail:
	return -1;
}

void EAGLDisplay::Cleanup ()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLDisplay::Cleanup ();

	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = NULL;

	if (mView)
	{
		[mView removeFromSuperview];
		[mView release];
		mView = NULL;
	}

	if (mWindow)
	{
		[mWindow release];
		mWindow = NULL;
	}
}

void EAGLDisplay::RemapMouse(int& theX, int& theY)
{
	theX = theX * (float)mDisplayWidth / mWindowWidth;
	theY = theY * (float)mDisplayHeight / mWindowHeight;
}

Image* EAGLDisplay::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

bool EAGLDisplay::HasEvent()
{
	return false;
}

bool EAGLDisplay::GetEvent(struct Event &event)
{
	UIEvent* nsevent;
	UIApplication* app = [UIApplication sharedApplication];
	AppDelegate* delegate = [app delegate];

	if ([delegate isQuit])
	{
		event.type = EVENT_QUIT;
		mApp->mInputManager->PushEvent (event);
	}

	event.type = EVENT_NONE;
	if (event.type == EVENT_NONE)
		return false;

	return true;
}

void EAGLDisplay::SwapBuffers()
{
	if (mWindow && mView)
	{
		EAGLView* view = (EAGLView*)mView;
		[view swapBuffers];
		Reshape();
	}
}

class AGLVideoDriver: public VideoDriver {
public:
	AGLVideoDriver ()
	 : VideoDriver("EAGL", 10)
	{
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new EAGLDisplay (theApp);
        }
};

static AGLVideoDriver aEAGLVideoDriver;
VideoDriverRegistor aEAGLVideoDriverRegistor (&aEAGLVideoDriver);
VideoDriver* GetEAGLVideoDriver()
{
	return &aEAGLVideoDriver;
}

