#include "GameView.h"
#include "GameLauncher.h"

extern "C" {

    int
    awGetViewSize(int *width, int *height)
    {
	GameLauncher* launcher = GameLauncher::getInstance();

	if (launcher->getState() < GAME_STATE_INITED)
	    return -1;

	if (width)
	    *width = launcher->getViewWidth();
	if (height)
	    *width = launcher->getViewHeight();

	return 0;
    }
}
