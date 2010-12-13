#include "GameView.h"
#include "GameLauncher.h"

extern "C" {

    int
    awGetViewSize(int *width, int *height)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

        if (width)
            *width = launcher->getViewWidth();
        if (height)
            *height = launcher->getViewHeight();

        return 0;
    }

    void
    awAddViewEventListener(awEventListener listener,
			   void* data)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

	return launcher->addEventListener(listener, data);
    }
}
