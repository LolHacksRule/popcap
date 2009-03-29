#include "SexyAppBase.h"
#include "AGLInterface.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "InputManager.h"
#include "VideoDriverFactory.h"

#include <cstdio>

using namespace Sexy;

@interface AppDelegate : NSResponder
{
	BOOL mQuit;
	AGLInterface* mInterface;
}

- (id)initWithInterface:(AGLInterface *)aInterface;
@end

@implementation AppDelegate

- (id)initWithInterface:(AGLInterface *)aInterface
{
	self = [super init];
	if (self)
		mInterface = aInterface;;
	return (self);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	mQuit = FALSE;
}

- (void)orderFrontStandardAboutPanel:(id)sender
{
	[NSApp orderFrontStandardAboutPanel:sender];
}

- (void)unhideAllApplications:(id)sender
{
	[NSApp unhideAllApplications:sender];
}

- (void)hide:(id)sender
{
	[NSApp hide:sender];
}

- (void)hideOtherApplications:(id)sender
{
	[NSApp hideOtherApplications:sender];
}

- (void)terminate:(id)sender
{
	mQuit = TRUE;
}

- (void)windowDidResize:(NSNotification *)aNotification
{
	NSWindow*window;
	NSRect frame;

	window = [aNotification object];
	frame = [window frame];
}

- (BOOL)isQuit
{
	return mQuit;
}

- (BOOL)AcceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	return YES;
}

- (void)MouseMoved:(NSEvent *)event
{
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	mQuit = TRUE;
}
@end

AGLInterface::AGLInterface (SexyAppBase* theApp)
	: GLInterface (theApp)
{
	static bool first = false;

	if (!first)
	{
		NSString* path;

		first = true;

		[[NSAutoreleasePool alloc] init];
		[NSApplication sharedApplication];
		[NSApp setDelegate:[[[AppDelegate alloc] initWithInterface:this] autorelease]];
		[NSBundle loadNibNamed:@"MainMenu" owner:[NSApp delegate]];
		[NSApp finishLaunching];

		path = [[NSBundle mainBundle] bundlePath];
		chdir([path UTF8String]);
		NSLog (path);
	}
	mWindow = NULL;
	mContext = NULL;
	mScreenCapture = false;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mScreenImage = 0;
	mCGLContext = 0;

	InitKeyMap ();
}

void AGLInterface::InitKeyMap ()
{
	struct {
		int NSKeyCode;
		int KeyCode;
	} keymap[] = {
		{ NSUpArrowFunctionKey, KEYCODE_UP },
		{ NSDownArrowFunctionKey, KEYCODE_DOWN },
		{ NSLeftArrowFunctionKey, KEYCODE_LEFT },
		{ NSRightArrowFunctionKey, KEYCODE_RIGHT },
		{ 0x0d, KEYCODE_RETURN },
		{ 0x1b, KEYCODE_ESCAPE },
		{ 0x7f, KEYCODE_BACK },
		{ 0x31, KEYCODE_SPACE },
		{ 0x33, KEYCODE_BACK },
		{ 0x35, KEYCODE_ESCAPE },
		{ 0x7b, KEYCODE_LEFT },
		{ 0x7c, KEYCODE_RIGHT },
		{ 0x7d, KEYCODE_DOWN },
		{ 0x7e, KEYCODE_UP },
		{ 0, 0 }
	};

	for (unsigned int i = 0; keymap[i].NSKeyCode; i++)
		mKeyMap[keymap[i].NSKeyCode] = keymap[i].KeyCode; 
}

int AGLInterface::KeyCodeFromNSKeyCode (int NSKeyCode)
{
	std::map<int, int>::iterator it;

	it = mKeyMap.find (NSKeyCode);
	if (it == mKeyMap.end())
		return 0;
	return it->second;
}

AGLInterface::~AGLInterface ()
{
	Cleanup();
}

int AGLInterface::Init (void)
{
	Cleanup();

	AutoCrit anAutoCrit (mCritSect);
	mInitialized = false;

	GLInterface::Init();

	int index;
	bool result;
	NSOpenGLPixelFormat* format;
	CGDirectDisplayID display;
	CGRect displayRect;
	NSOpenGLPixelFormatAttribute windowattribs[32];

	result = false;
	display = CGMainDisplayID ();
	displayRect = CGDisplayBounds (display);

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mDesktopWidth = displayRect.size.width;
	mDesktopHeight = displayRect.size.height;

	if (mApp->mIsWindowed)
	{
		mWindowWidth = mWidth;
		mWindowHeight = mHeight;
	}
	else
	{
		mWindowWidth = mDesktopWidth;
		mWindowHeight = mDesktopHeight;
	}
	
	if (mApp->mIsWindowed)
	{
		mWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, mWindowWidth, mWindowHeight)
					    styleMask:NSTitledWindowMask + NSClosableWindowMask +
					    NSResizableWindowMask
					    backing:NSBackingStoreBuffered defer:FALSE];
		if (!mWindow)
			goto fail;
	}
	else
	{
		mWindow = 0;
	}

	index = 0;
	if (!mApp->mIsWindowed)
	{
		windowattribs[index++] = NSOpenGLPFAScreenMask;
		windowattribs[index++] = CGDisplayIDToOpenGLDisplayMask (display);
	}
	windowattribs[index++] = NSOpenGLPFANoRecovery;
	windowattribs[index++] = NSOpenGLPFADoubleBuffer;
	windowattribs[index++] = NSOpenGLPFAAccelerated;
	windowattribs[index++] = NSOpenGLPFADepthSize;
	windowattribs[index++] = (NSOpenGLPixelFormatAttribute)16;
	windowattribs[index++] = NSOpenGLPFAColorSize;
	windowattribs[index++] = (NSOpenGLPixelFormatAttribute)32;
	windowattribs[index++] = (NSOpenGLPixelFormatAttribute)NULL;

	format = [[NSOpenGLPixelFormat alloc] initWithAttributes:windowattribs];
	if (!format)
		goto close_window;
	mContext = [[NSOpenGLContext alloc] initWithFormat:format
					    shareContext:NULL];
	[format release];
	if (!mContext)
		goto close_window;

	NSView* view;

	if (!mApp->mIsWindowed)
	{
		CGCaptureAllDisplays ();
		mScreenCapture = true;
		[mContext setFullScreen];
		view = 0;
		mWindowWidth = mDesktopWidth;
		mWindowHeight = mDesktopHeight;
	}
	else
	{
		view = [mWindow contentView];
		[mWindow center];
		[mWindow setDelegate:[NSApp delegate]];
		[mContext setView:[mWindow contentView]];
		[mWindow setAcceptsMouseMovedEvents:TRUE];
		[mWindow setIgnoresMouseEvents:NO];
		[mWindow setIsVisible:TRUE];
		[mWindow makeKeyAndOrderFront:nil];

		NSRect frame = [view bounds];
		mWindowWidth = frame.size.width;
		mWindowHeight = frame.size.height;
		[mContext setView:view];
		[view addTrackingRect:[view bounds] owner:view userData:NULL
		      assumeInside:NO];
		[mWindow makeFirstResponder:view];
		[mContext setView:view];
		[view addTrackingRect:[view bounds] owner:view userData:NULL
		      assumeInside:NO];
		[mWindow makeFirstResponder:view];

		[mWindow setTitle:[NSString stringWithCString:mApp->mTitle.c_str()
					    length:mApp->mTitle.length()]];
		[NSCursor hide];
	}

	mCGLContext = (CGLContextObj) [mContext CGLContextObj];
	CGLSetCurrentContext (mCGLContext);

	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mPresentationRect.mX = 0;
	mPresentationRect.mY = 0;
	mPresentationRect.mWidth = mDisplayWidth;
	mPresentationRect.mHeight = mDisplayHeight;

	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	InitGL ();

	mScreenImage->mFlags = IMAGE_FLAGS_DOUBLE_BUFFER;

	mInitCount++;
	mInitialized = true;

	return 0;
 close_window:
	[mWindow release];
 fail:
	return -1;
}

void AGLInterface::Cleanup ()
{
	AutoCrit anAutoCrit(mCritSect);

	mInitialized = false;

	GLInterface::Cleanup ();

	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = NULL;

	if (mScreenCapture)
		CGReleaseAllDisplays ();

	if (mWindow)
	{
		[mWindow setIsVisible:FALSE];
	}

	if (mContext)
	{
		[mContext clearDrawable];
		[mContext release];
	}
	mContext = NULL;

	if (mCGLContext)
	{
		CGLSetCurrentContext (NULL);
		CGLClearDrawable (mCGLContext);
		CGLDestroyContext (mCGLContext);
		mCGLContext = NULL;
	}

	if (mWindow)
	{
		[mWindow setReleasedWhenClosed:TRUE];
		[mWindow release];
		mWindow = NULL;
	}
}

void AGLInterface::RemapMouse(int& theX, int& theY)
{
	theX = theX * (float)mDisplayWidth / mWindowWidth;
	theY = theY * (float)mDisplayHeight / mWindowHeight;
}

Image* AGLInterface::CreateImage(SexyAppBase * theApp,
				 int width, int height)
{
	GLImage* anImage = new GLImage(this);

	if (anImage == NULL)
		return NULL;
	anImage->Create (width, height);
	return anImage;
}

bool AGLInterface::HasEvent()
{
	return false;
}

bool AGLInterface::GetEvent(struct Event &event)
{
	NSEvent* nsevent;

	if ([[NSApp delegate] isQuit])
	{
		event.type = EVENT_QUIT;
		mApp->mInputManager->PushEvent (event);
	}

	event.type = EVENT_NONE;

	int keycode;
	NSString * chars;

	nsevent = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil
			 inMode:NSDefaultRunLoopMode dequeue:YES];
	if (nsevent != nil)
	{
		switch ([nsevent type])
		{
		case NSKeyDown:
			if ([nsevent modifierFlags] & NSCommandKeyMask)
				[NSApp sendEvent:nsevent];
			keycode = [nsevent keyCode];
			NSString * chars = [nsevent characters];
			event.type = EVENT_KEY_DOWN;
			event.flags = EVENT_FLAGS_KEY_CODE;
			event.u.key.keyCode = KeyCodeFromNSKeyCode (keycode); 	
			if ([chars length])
			{
				event.flags |= EVENT_FLAGS_KEY_CHAR;
				event.u.key.keyChar = [chars characterAtIndex:0];
				if (isalnum (event.u.key.keyChar))
				{
					chars = [nsevent charactersIgnoringModifiers];
					event.u.key.keyCode = [chars characterAtIndex:0];
				}
			}
			break;

		case NSKeyUp:
			if ([nsevent modifierFlags] & NSCommandKeyMask)
                                [NSApp sendEvent:nsevent];
			keycode = [nsevent keyCode];
			chars = [nsevent charactersIgnoringModifiers];
			event.type = EVENT_KEY_UP;
			event.flags = EVENT_FLAGS_KEY_CODE;
			event.u.key.keyCode = KeyCodeFromNSKeyCode (keycode); 	
			if (!event.u.key.keyCode && [chars length])
				event.u.key.keyCode = [chars characterAtIndex:0];
			break;

		case NSLeftMouseDown:
			event.type = EVENT_MOUSE_BUTTON_PRESS;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			event.u.mouse.button = 1;
                        event.u.mouse.x = int([nsevent locationInWindow].x);
                        event.u.mouse.y = mWindowHeight - int([nsevent locationInWindow].y);
			RemapMouse (event.u.mouse.x, event.u.mouse.y);
                        [NSApp sendEvent:nsevent];
			break;

		case NSLeftMouseUp:
                        event.type = EVENT_MOUSE_BUTTON_RELEASE;
                        event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
                        event.u.mouse.button = 1;
                        event.u.mouse.x = int([nsevent locationInWindow].x);
                        event.u.mouse.y = mWindowHeight - int([nsevent locationInWindow].y);
			RemapMouse (event.u.mouse.x, event.u.mouse.y);
                        [NSApp sendEvent:nsevent];
			break;

                case NSRightMouseDown:
			event.type = EVENT_MOUSE_BUTTON_PRESS;
                        event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
                        event.u.mouse.button = 2;
                        event.u.mouse.x = int([nsevent locationInWindow].x);
			event.u.mouse.y = mWindowHeight - int([nsevent locationInWindow].y);
			RemapMouse (event.u.mouse.x, event.u.mouse.y);
			[NSApp sendEvent:nsevent];
			break;

		case NSRightMouseUp:
                        event.type = EVENT_MOUSE_BUTTON_RELEASE;
                        event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
                        event.u.mouse.button = 2;
                        event.u.mouse.x = int([nsevent locationInWindow].x);
                        event.u.mouse.y = mWindowHeight - int([nsevent locationInWindow].y);
                        RemapMouse (event.u.mouse.x, event.u.mouse.y);
			[NSApp sendEvent:nsevent];
                        break;

		case NSMouseMoved:
			event.type = EVENT_MOUSE_MOTION;
			event.flags = EVENT_FLAGS_AXIS;
			event.u.mouse.x = int([nsevent locationInWindow].x);
			event.u.mouse.y = mWindowHeight - int([nsevent locationInWindow].y);
			RemapMouse (event.u.mouse.x, event.u.mouse.y);
			[NSApp sendEvent:nsevent];
			break;

		case NSScrollWheel:
			break;

		case NSMouseEntered:
			event.type = EVENT_ACTIVE;
			event.flags = 0;
			event.u.active.active = true;
			break;

		case NSMouseExited:
			event.type = EVENT_ACTIVE;
			event.flags = 0;
			event.u.active.active = true;
                        break;

		default:
			[NSApp sendEvent:nsevent];
			break;
		}
	}

	if (event.type == EVENT_NONE)
		return false;

	return true;
}

void AGLInterface::SwapBuffers()
{
	if (mContext)
		[mContext flushBuffer ];
}

class AGLVideoDriver: public VideoDriver {
public:
	AGLVideoDriver ()
	 : VideoDriver("AGL", 10)
	{
	}

	NativeDisplay* Create (SexyAppBase * theApp)
	{
		return new AGLInterface (theApp);
        }
};

static AGLVideoDriver aAGLVideoDriver;
VideoDriverRegistor aAGLVideoDriverRegistor (&aAGLVideoDriver);
VideoDriver* GetAGLVideoDriver()
{
	return &aAGLVideoDriver;
}

