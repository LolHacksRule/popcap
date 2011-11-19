#include "VideoDriverFactory.h"

using namespace Sexy;

VideoDriver::VideoDriver (const std::string theName,
			  int		    thePriority)
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

namespace Sexy {

class StaticVideoDriverFactory
{
public:
	struct StaticData {
		VideoDriverFactory* mFactory;
		bool mDone;
	};

	StaticVideoDriverFactory(StaticData* data)
	{
		mData = data;
	}

	VideoDriverFactory* Get(StaticData* data)
	{
		if (data->mDone)
			return 0;

		if (data->mFactory)
			return data->mFactory;

		data->mFactory = new VideoDriverFactory;
		return data->mFactory;
	}

	~StaticVideoDriverFactory()
	{
		if (!mData)
			return;

		mData->mDone = true;
		if (mData->mFactory)
			delete mData->mFactory;
	}

private:
	StaticData* mData;
};

static StaticVideoDriverFactory::StaticData aData;
static StaticVideoDriverFactory videoDriverFactory(&aData);

}

VideoDriverFactory* VideoDriverFactory::GetVideoDriverFactory ()
{
	return videoDriverFactory.Get(&aData);
}

/* This is a hack that preventing linker from striping drivers out of
 * binary.
 */
extern VideoDriver* GetAGLVideoDriver();
extern VideoDriver* GetEAGLVideoDriver();
extern VideoDriver* GetGLXVideoDriver();
extern VideoDriver* GetDFBVideoDriver();
extern VideoDriver* GetWGLVideoDriver();
extern VideoDriver* GetCEGLESVideoDriver();
extern VideoDriver* GetXGLESVideoDriver();
extern VideoDriver* GetAndroidVideoDriver();
typedef VideoDriver* (* VideoDriverGetter)();
VideoDriverGetter VideoDriverGetters []= {
#ifdef SEXY_AGL_DRIVER
       GetAGLVideoDriver,
#endif
#ifdef SEXY_EAGL_DRIVER
       GetEAGLVideoDriver,
#endif
#ifdef SEXY_GLX_DRIVER
	GetGLXVideoDriver,
#endif
#ifdef SEXY_DFB_DRIVER
	GetDFBVideoDriver,
#endif
#ifdef SEXY_WGL_DRIVER
	GetWGLVideoDriver,
#endif
#ifdef SEXY_CEGLES_DRIVER
	GetCEGLESVideoDriver,
#endif

#ifdef SEXY_XGLES_DRIVER
       GetXGLESVideoDriver,
#endif

#ifdef SEXY_ANDROIDGLES_DRIVER
	GetAndroidVideoDriver,
#endif
	NULL
};

void VideoDriverFactory::Load(void)
{
	int i = 0;
	for (i = 0; VideoDriverGetters[i]; i++)
		VideoDriverGetters[i]();
}

