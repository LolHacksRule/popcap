#include "GameApp.h"

#include <unistd.h>

#include <android/log.h>
#include <jni.h>

#define  LOG_TAG    "Hungarr2"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace Sexy;

static GameApp *app;

extern "C" {

int
GameInit()
{
    if (app)
	return -1;

    char path[2048];

    getcwd(path, sizeof(path));

    LOGI("current working directory: %s\n", path);
    LOGI("starting Hungarr2...\n");
    app = new GameApp();
    app->mAllowSleep = false;
    LOGI("Initalizing Hungarr2...\n");
    app->Init();
    LOGI("Hungarr2 intialized.\n");
    app->Startup();
    LOGI("Hungarr2 set to run state now.\n");

    return 0;
}

int
GameUninit(void)
{
    if (!app)
	return 0;

    app->Shutdown();
    delete app;

    app = 0;

    return 0;
}

int
GameRender(void)
{
    if (!app)
	return 0;

    int count;
    while (true)
    {
	count = app->mDrawCount;
	app->UpdateApp();
	if (app->mShutdown)
	    return 0;
	if (app->mDrawCount != count)
	    break;
    }

    return 1;
}

}
