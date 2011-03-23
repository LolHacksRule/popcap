#include "GameView.h"
#include "GameLauncher.h"

extern "C" {

    int
    AGViewGetSize(int *width, int *height)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

        if (width)
            *width = launcher->getViewWidth();
        if (height)
            *height = launcher->getViewHeight();

        return 0;
    }

    void
    AGViewAddEventListener(AGEventListener listener,
			   void* data)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

	return launcher->addEventListener(listener, data);
    }

    void AGViewSwapBuffers(void)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

	return launcher->viewSwapBuffers();
    }

    void AGViewUpdate(void)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

	return launcher->viewUpdate();
    }
}
