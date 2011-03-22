#include "GameInfo.h"
#include "GameLauncher.h"

extern "C" const char*
AGGetSourceDir(void)
{
	GameLauncher* launcher = GameLauncher::getInstance();

	if (launcher->getState() < GAME_STATE_INITED)
	    return 0;

	return launcher->getSourceDir();
}

extern "C" const char*
AGGetDataDir(void)
{
	GameLauncher* launcher = GameLauncher::getInstance();

	if (launcher->getState() < GAME_STATE_INITED)
	    return 0;

	return launcher->getDataDir();
}

extern "C" const char*
AGGetFilesDir(void)
{
	GameLauncher* launcher = GameLauncher::getInstance();

	if (launcher->getState() < GAME_STATE_INITED)
	    return 0;

	return launcher->getFilesDir();
}
