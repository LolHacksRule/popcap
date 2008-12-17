#include "SexyAppBase.h"
#include "AGLInterface.h"
#include "GLImage.h"
#include "AutoCrit.h"
#include "Graphics.h"
#include "PerfTimer.h"
#include "Debug.h"
#include "MemoryImage.h"
#include "KeyCodes.h"
#include "VideoDriverFactory.h"

#include <cstdio>

using namespace Sexy;

@interface SexyGLView : NSOpenGLView
{
	@public
	AGLInterface * iface;
}
@end

@implementation SexyGLView

- (id)initWithFrame:(NSRect)theFrame
{
	NSOpenGLContext *ctx;
	NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 16,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };

    // Create our non-FullScreen pixel format.
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];	
    /* Create a GL context */
    self = [super initWithFrame:theFrame pixelFormat: pixelFormat];

    return self;
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (BOOL)isOpaque
{
	return YES;
}

- (void)reshape
{
	[super reshape];
}

- (void)drawRect:(NSRect)theRect
{
}

- (void)mouseDown:(NSEvent *)theEvent
{
}

- (void)keyDown:(NSEvent *)theEvent
{
}
@end

AGLInterface::AGLInterface (SexyAppBase* theApp)
	: GLInterface (theApp)
{
	mView = NULL;
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

	NSRect rect = NSMakeRect(0, 0, mWidth, mHeight);
	
	mView = [[SexyGLView alloc] initWithFrame:rect];
	mView->iface = this;
	mContext = [mView openGLContext];
	mScreenImage = static_cast<GLImage*>(CreateImage(mApp, mWidth, mHeight));
	InitGL ();

	mScreenImage->mFlags =
		(ImageFlags)(IMAGE_FLAGS_DOUBLE_BUFFER |
			     IMAGE_FLAGS_FLIP_AS_COPY);

	mInitCount++;
	mInitialized = true;

	return 0;
 close_window:
	mView = NULL; 
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

	//if (mContext)
	//	[mContext clearDrawable];
	mContext = NULL;

	if (mView)
		[mView dealloc];
	mView = NULL;
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
	return false;
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

