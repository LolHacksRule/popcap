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
#include <ctype.h>

using namespace Sexy;

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
	BOOL quit;
	NSTimer *timer;
}

- (void)updateApp:(id)sender;
@end

@implementation AppDelegate

- (id)init
{
	self = [super init];
	if (self)
	{
		timer = 0;
		quit = FALSE;
	}
	return (self);
}

- (void)applicationDidFinishLaunching:(UIApplication *)anApp
{
#ifdef DEBUG
	NSLog(@"applicationDidFinishLaunching");
#endif
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
#ifdef DEBUG
	NSLog(@"applicationWillResignActive");
#endif
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
#ifdef DEBUG
	NSLog(@"applicationDidBecomeActive");
#endif
	if (!timer)
		timer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0 / 400.0)
							        target:self selector:@selector(updateApp:)
						                userInfo:nil repeats:TRUE];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
#ifdef DEBUG
	NSLog(@"applicatioWillTerminaten");
#endif
	quit = TRUE;

	if (timer)
		[timer invalidate];
	timer = nil;

	if (gSexyAppBase)
		gSexyAppBase->Terminate();
}
- (void)updateApp:(id)sender
{
	if (!gSexyAppBase)
		return;

	gSexyAppBase->UpdateApp();
	if (gSexyAppBase->mShutdown)
	{
		[self applicationWillTerminate:[UIApplication sharedApplication]];
		exit(0);
	}
}

@end

typedef std::map<UITouch*, int> TouchMap;
@interface TouchDelegate : NSObject <EAGLTouchDelegate>
{
	int touchId;
	TouchMap touchMap;
}
@end

@implementation TouchDelegate

- (id)init
{
	self = [super init];
	if (self)
		touchId = 0;
	return (self);
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	//NSLog(@"touchesBegan");
	for (UITouch* touch in touches)
	{
		if (touchMap.find(touch) == touchMap.end())
			touchMap.insert(TouchMap::value_type(touch, touchId++));
	}

	SexyAppBase *app = gSexyAppBase;
	EAGLDisplay *dpy = (EAGLDisplay*)app->mDDInterface;
	std::list<Event> events;
	NSUInteger i = 0;
	for (UITouch *touch in touches)
	{
		CGPoint currentPosition = [touch locationInView:dpy->mView];
		//NSLog(@"position: %.1f %.1f", currentPosition.x, currentPosition.y);
		int x = currentPosition.x * dpy->mWidth / dpy->mWindowWidth;
		int y = currentPosition.y * dpy->mHeight / dpy->mWindowHeight;

		Event evt;
		evt.type = EVENT_TOUCH;
		evt.flags = 0;
		evt.id = 0;
		evt.subid = 0;
		if (i++ != [touches count] - 1)
			evt.flags |= EVENT_FLAGS_INCOMPLETE;
		evt.u.touch.id = touchMap[touch];
		evt.u.touch.state = TOUCH_DOWN;
		evt.u.touch.x = x;
		evt.u.touch.y = y;
		evt.u.touch.pressure = 100;
		events.push_back(evt);
		
	}
	app->mInputManager->PushEvents(events);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	//NSLog(@"touchesMoved");

	if (!gSexyAppBase)
		return;

	SexyAppBase *app = gSexyAppBase;
	EAGLDisplay *dpy = (EAGLDisplay*)app->mDDInterface;
	std::list<Event> events;
	NSUInteger i = 0;
	for (UITouch *touch in touches)
	{
		CGPoint currentPosition = [touch locationInView:dpy->mView];
		//NSLog(@"position: %.1f %.1f", currentPosition.x, currentPosition.y);
		int x = currentPosition.x * dpy->mWidth / dpy->mWindowWidth;
		int y = currentPosition.y * dpy->mHeight / dpy->mWindowHeight;

		Event evt;
		evt.type = EVENT_TOUCH;
		evt.flags = 0;
		evt.id = 0;
		evt.subid = 0;
		if (i++ != [touches count] - 1)
			evt.flags |= EVENT_FLAGS_INCOMPLETE;
		evt.u.touch.id = touchMap[touch];
		evt.u.touch.state = TOUCH_MOVE;
		evt.u.touch.x = x;
		evt.u.touch.y = y;
		evt.u.touch.pressure = 100;
		events.push_back(evt);
	}
	app->mInputManager->PushEvents(events);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	//NSLog(@"touchesEnded");

	if (!gSexyAppBase)
		return;

	SexyAppBase *app = gSexyAppBase;
	UITouch *touch = [touches anyObject];
	EAGLDisplay *dpy = (EAGLDisplay*)app->mDDInterface;
	std::list<Event> events;
	NSUInteger i = 0;
	for (UITouch *touch in touches)
	{
		CGPoint currentPosition = [touch locationInView:dpy->mView];
		int x = currentPosition.x * dpy->mWidth / dpy->mWindowWidth;
		int y = currentPosition.y * dpy->mHeight / dpy->mWindowHeight;

		Event evt;
		evt.type = EVENT_TOUCH;
		evt.flags = 0;
		evt.id = 0;
		evt.subid = 0;
		if (i++ != [touches count] - 1)
			evt.flags |= EVENT_FLAGS_INCOMPLETE;
		evt.u.touch.id = touchMap[touch];
		evt.u.touch.state = TOUCH_UP;
		evt.u.touch.x = x;
		evt.u.touch.y = y;
		evt.u.touch.pressure = 100;
		events.push_back(evt);
		
	}
	app->mInputManager->PushEvents(events);

	if ([touches count] == 1)
	{
		touchMap.clear();
		touchId = 0;
	}
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
	//NSLog(@"touchCancelled");
	if (!gSexyAppBase)
		return;

	SexyAppBase *app = gSexyAppBase;
	UITouch *touch = [touches anyObject];
	EAGLDisplay *dpy = (EAGLDisplay*)app->mDDInterface;
	std::list<Event> events;
	NSUInteger i = 0;
	for (UITouch *touch in touches)
	{
		CGPoint currentPosition = [touch locationInView:dpy->mView];
		int x = currentPosition.x * dpy->mWidth / dpy->mWindowWidth;
		int y = currentPosition.y * dpy->mHeight / dpy->mWindowHeight;

		Event evt;
		evt.type = EVENT_TOUCH;
		evt.flags = 0;
		evt.id = 0;
		evt.subid = 0;
		if (i++ != [touches count] - 1)
			evt.flags |= EVENT_FLAGS_INCOMPLETE;
		evt.u.touch.id = touchMap[touch];
		evt.u.touch.state = TOUCH_CANCEL;
		evt.u.touch.x = x;
		evt.u.touch.y = y;
		evt.u.touch.pressure = 100;
		events.push_back(evt);
		
	}
	app->mInputManager->PushEvents(events);

	if ([touches count] == 1)
	{
		touchMap.clear();
		touchId = 0;
	}
}
@end

@interface TextFieldDelegate: NSObject<UITextFieldDelegate>
{
}
@end

@implementation TextFieldDelegate
/* UITextFieldDelegate method.  Invoked when user types something. */
- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	if (!gSexyAppBase)
		return NO;

	InputManager *mgr = gSexyAppBase->mInputManager;
	Event evt;

	evt.id = 0;
	evt.subid = 0;
	evt.flags = 0;
	if ([string length] == 0)
	{
		evt.type = EVENT_KEY_DOWN;
		evt.flags |= EVENT_FLAGS_KEY_CODE;
		evt.u.key.keyCode = KEYCODE_DELETE;
		mgr->PushEvent(evt);

		evt.type = EVENT_KEY_UP;
		mgr->PushEvent(evt);
	}
	else
	{
		NSUInteger i;
		for (i = 0; i < [string length]; i++)
		{
			unichar c = [string characterAtIndex: i];
			
			if (c >= 127)
				continue;

			evt.type = EVENT_KEY_DOWN;
			evt.flags |= EVENT_FLAGS_KEY_CODE;
			evt.u.key.keyCode = toupper(c);
			if (isprint(c))
			{
				evt.flags |= EVENT_FLAGS_KEY_CHAR;
				evt.u.key.keyChar = c;
			}
			mgr->PushEvent(evt);
		}
	}

	return NO;
}

/* Terminates the editing session */
- (BOOL)textFieldShouldReturn:(UITextField*)textField {
	[textField resignFirstResponder];
	return YES;
}
@end

EAGLDisplay::EAGLDisplay (SexyAppBase* theApp)
	: GLDisplay (theApp)
{
	mView = NULL;
	mTextField = NULL;
	mWindow = NULL;
	mContext = NULL;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mScreenImage = 0;
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
	CGRect windowRect;
	UIInterfaceOrientation orientation;

	orientation = [[UIApplication sharedApplication] statusBarOrientation];

	result = false;
	displayRect = [[UIScreen mainScreen] bounds];
	windowRect = displayRect;

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;

	if (orientation == UIInterfaceOrientationLandscapeRight)
		std::swap(windowRect.size.width, windowRect.size.height);

	mDesktopWidth = windowRect.size.width;
	mDesktopHeight = windowRect.size.height;
	mWindowWidth = mDesktopWidth;
	mWindowHeight = mDesktopHeight;
	mWindow = [[UIWindow alloc] initWithFrame:displayRect];
	if (!mWindow)
		goto fail;
	mView = [[EAGLView alloc] initWithFrame:windowRect];
	if (!mView)
		goto close_window;

        if (orientation == UIInterfaceOrientationLandscapeRight)
        { 
		CGAffineTransform transform = mView.transform;

		// use the status bar frame to determine the center point of the window's content area.
		CGRect bounds = mView.frame;
		CGPoint center = CGPointMake(bounds.size.height / 2.0, bounds.size.width / 2.0);
		// set the center point of the view to the center point of the window's content area.
		mView.center = center;

		// Rotate the view 90 degrees around its new center point. 
		transform = CGAffineTransformRotate(transform, (M_PI / 2.0));
		mView.transform = transform;
        }

	[(EAGLView*)mView setTouchDelegate:[[TouchDelegate alloc] init]];
	[mWindow addSubview:mView];
	[mView retain];

	[mWindow makeKeyAndVisible];
	[mWindow setUserInteractionEnabled:YES];
	[mWindow setMultipleTouchEnabled:YES];
	[mView setUserInteractionEnabled:YES];
	[mView setMultipleTouchEnabled:YES];

	mTextField = [[[UITextField alloc] initWithFrame: CGRectZero] autorelease];
	mTextField.delegate = [[TextFieldDelegate alloc] init];
	/* placeholder so there is something to delete! */
	mTextField.text = @" ";	
	
	/* set UITextInputTrait properties, mostly to defaults */
	mTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
	mTextField.autocorrectionType = UITextAutocorrectionTypeNo;
	mTextField.enablesReturnKeyAutomatically = NO;
	mTextField.keyboardAppearance = UIKeyboardAppearanceDefault;
	mTextField.keyboardType = UIKeyboardTypeDefault;
	mTextField.returnKeyType = UIReturnKeyDefault;
	mTextField.secureTextEntry = NO;	
	
	mTextField.hidden = YES;
	[mView addSubview: mTextField];
	mKeyboardVisible = false;

	CGRect frame;

	frame = [mWindow frame];
	//NSLog(@"UIWindow: frame %.1fx%.1f", frame.size.width, frame.size.height);
	frame = [mView frame];
	//NSLog(@"UIView: frame %.1fx%.1f", frame.size.width, frame.size.height);

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

	mTextField = NULL;
}

bool EAGLDisplay::ShowKeyBoard()
{
	if (!mTextField)
		return false;

	if (mKeyboardVisible)
		return true;

	[mTextField becomeFirstResponder];
	mKeyboardVisible = true;
	return true;
}

void EAGLDisplay::HideKeyBoard()
{
	if (!mKeyboardVisible)
		return;

	[mTextField resignFirstResponder];
	mKeyboardVisible = false;
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

