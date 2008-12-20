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
	printf ("mouse moved\n");
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

		path = [[NSBundle mainBundle] bundlePath]; // stringByDeletingLastPathComponent];
		printf ("changing working dir to %s, %s\n", [path UTF8String], getcwd(NULL, 0));
		chdir([path UTF8String]);
                printf ("working dir changed to %s\n", getcwd(NULL, 0));
		NSLog (path);
	}
	mWindow = NULL;
	mContext = NULL;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
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
	NSOpenGLPixelFormatAttribute windowattribs[32];
	
	result = false;
	display = CGMainDisplayID ();

	mWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, mWidth, mHeight)
				    styleMask:NSTitledWindowMask + NSClosableWindowMask + NSResizableWindowMask
				    backing:NSBackingStoreBuffered defer:FALSE];
	if (!mWindow)
		goto fail;

	index = 0;
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
	mContext = [[NSOpenGLContext alloc] initWithFormat:format shareContext:NULL];
	[format release];
	if (!mContext)
		goto close_window;

	[mWindow center];
	[mWindow setDelegate:[NSApp delegate]];
	[mContext setView:[mWindow contentView]];
	[mWindow setAcceptsMouseMovedEvents:TRUE];
	[mWindow setIgnoresMouseEvents:NO];
	[mWindow setIsVisible:TRUE];
	[mWindow makeKeyAndOrderFront:nil];

	NSView* view = [mWindow contentView];
	[view addTrackingRect:[view bounds] owner:view userData:NULL assumeInside:NO];
        [mWindow makeFirstResponder:view];

	mCGLContext = (CGLContextObj) [mContext CGLContextObj];

	[mWindow setTitle:[NSString stringWithCString:mApp->mTitle.c_str()
				    length:mApp->mTitle.length()]];


	CGLSetCurrentContext (mCGLContext);
	
	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	InitGL ();

	mScreenImage->mFlags =
		(ImageFlags)(IMAGE_FLAGS_DOUBLE_BUFFER |
			     IMAGE_FLAGS_FLIP_AS_COPY);

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

	if (mWindow)
		[mWindow setIsVisible:FALSE];
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
}

bool AGLInterface::EnableCursor(bool enable)
{
	return false;
}

bool AGLInterface::SetCursorImage(Image* theImage, int theHotX, int theHotY)
{
	return false;
}

void AGLInterface::SetCursorPos(int theCursorX, int theCursorY)
{
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

	nsevent = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil
			 inMode:NSDefaultRunLoopMode dequeue:YES];
	if (nsevent != nil)
	{
		switch ([nsevent type])
		{
		case NSKeyDown:
			if ([nsevent modifierFlags] & NSCommandKeyMask)
				[NSApp sendEvent:nsevent];
			break;
			
		case NSKeyUp:
			if ([nsevent modifierFlags] & NSCommandKeyMask)
                                [NSApp sendEvent:nsevent];
			break;

		case NSLeftMouseDown:
			event.type = EVENT_MOUSE_BUTTON_PRESS;
			event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
			event.button = 1;
                        event.x = int([nsevent locationInWindow].x);
                        event.y = mHeight - int([nsevent locationInWindow].y);
                        [NSApp sendEvent:nsevent];
			break;

		case NSLeftMouseUp:
                        event.type = EVENT_MOUSE_BUTTON_RELEASE;
                        event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
                        event.button = 1;
                        event.x = int([nsevent locationInWindow].x);
                        event.y = mHeight - int([nsevent locationInWindow].y);
                        [NSApp sendEvent:nsevent];
			break;

                case NSRightMouseDown:
			event.type = EVENT_MOUSE_BUTTON_PRESS;
                        event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
                        event.button = 2;
                        event.x = int([nsevent locationInWindow].x);
			event.y = mHeight - int([nsevent locationInWindow].y);
			[NSApp sendEvent:nsevent];
			break;

		case NSRightMouseUp:
                        event.type = EVENT_MOUSE_BUTTON_RELEASE;
                        event.flags = EVENT_FLAGS_AXIS | EVENT_FLAGS_BUTTON;
                        event.button = 2;
                        event.x = int([nsevent locationInWindow].x);
                        event.y = mHeight - int([nsevent locationInWindow].y);
                        [NSApp sendEvent:nsevent];
                        break;

		case NSMouseMoved:
			event.type = EVENT_MOUSE_MOTION;
			event.flags = EVENT_FLAGS_AXIS;
			event.x = int([nsevent locationInWindow].x);
			event.y = mHeight - int([nsevent locationInWindow].y);
			[NSApp sendEvent:nsevent];
			break;

		case NSScrollWheel:
			break;

		case NSMouseEntered:
			event.type = EVENT_ACTIVE;
			event.flags = 0;
			event.active = true;
			break;

		case NSMouseExited:
			event.type = EVENT_ACTIVE;
			event.flags = 0;
			event.active = true;
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

