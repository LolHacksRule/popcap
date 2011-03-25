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

    void AGViewShowKeyboard(enum AGKeyboardMode mode,
			    const char *title,
			    const char *hint,
			    const char *initial)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

	return launcher->viewShowKeyboard((int)mode, title, hint, initial);
    }

    void AGViewHideKeyboard(void)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

	return launcher->viewHideKeyboard();
    }

    const char*
    AGViewGetTextInput(void)
    {
        GameLauncher* launcher = GameLauncher::getInstance();

	return launcher->getTextInput();
    }
}
