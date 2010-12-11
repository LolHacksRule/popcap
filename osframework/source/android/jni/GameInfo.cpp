#include "GameInfo.h"
#include "GameLauncher.h"

extern "C" const char*
awGetSourceDir(void)
{
	GameLauncher* launcher = GameLauncher::getInstance();

	if (launcher->getState() < GAME_STATE_INITED)
	    return 0;

	return launcher->getSourceDir();
}

extern "C" const char*
awGetDataDir(void)
{
	GameLauncher* launcher = GameLauncher::getInstance();

	if (launcher->getState() < GAME_STATE_INITED)
	    return 0;

	return launcher->getDataDir();
}

extern "C" const char*
awGetFilesDir(void)
{
	GameLauncher* launcher = GameLauncher::getInstance();

	if (launcher->getState() < GAME_STATE_INITED)
	    return 0;

	return launcher->getFilesDir();
}
