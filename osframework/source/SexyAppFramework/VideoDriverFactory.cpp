#include "VideoDriverFactory.h"

using namespace Sexy;

VideoDriver::VideoDriver (const std::string theName,
			  int               thePriority)
	: Driver(theName, thePriority)
{
}

VideoDriver::~VideoDriver ()
{
}

VideoDriverFactory::VideoDriverFactory ()
	: DriverFactory ()
{
}

VideoDriverFactory::~VideoDriverFactory ()
{
}

VideoDriverFactory*  VideoDriverFactory::GetVideoDriverFactory ()
{
	static VideoDriverFactory  * theVideoDriverFactory;

	if (!theVideoDriverFactory)
		theVideoDriverFactory = new VideoDriverFactory ();
	return theVideoDriverFactory;
}

/* This is a hack that preventing gcc from striping drivers out of
 * binary.
 */
extern VideoDriver* GetGLXVideoDriver();
extern VideoDriver* GetDFBVideoDriver();
extern VideoDriver* GetWGLVideoDriver();
typedef VideoDriver* (* VideoDriverGetter)();
VideoDriverGetter VideoDriverGetters []= {
#ifdef SEXY_GLX_DRIVER
	GetGLXVideoDriver,
#endif
#ifdef SEXY_DFB_DRIVER
	GetDFBVideoDriver,
#endif
#ifdef SEXY_WGL_DRIVER
	GetWGLVideoDriver,
#endif
	NULL
};

void VideoDriverFactory::Load(void)
{
	int i = 0;
	for (i = 0; VideoDriverGetters[i]; i++)
		VideoDriverGetters[i]();
}

