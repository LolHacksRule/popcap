#include "GameApp.h"

using namespace Sexy;

static GameApp *app;

extern "C" {

int
GameInit()
{
    if (app)
	return -1;

    app = new GameApp();
    app->mAllowSleep = false;
    app->Init();
    app->Startup();

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

    return app->DrawOneFrame() ? 1 : 0;
}

}
