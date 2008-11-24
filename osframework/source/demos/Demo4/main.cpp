//////////////////////////////////////////////////////////////////////////
//						main.cpp
//
//	This is the starting point for all new projects. This file's purpose is
//	pretty small, but important. In here we create our application, initialize
//	it, and begin processing all the game code.
//
//	This demo will teach you:
//	* Using the resource manager
//	* Title screen with progress bar
//	* Loading/playing music
//	* Playing sounds with pitch and panning values changed
//	* smooth motion with UpdateF
//	* Reading/writing to files/registry
//	* Getting command line switches
//	* Widgets: Hyperlink widget, edit widget, checkbox, list, scrollbars, safedeletewidget
//////////////////////////////////////////////////////////////////////////

#include "GameApp.h"

// The SexyAppFramework resides in the "Sexy" namespace. As a convenience,
// you'll see in all the .cpp files "using namespace Sexy" to avoid
// having to prefix everything with Sexy::
using namespace Sexy;

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{	
#ifdef WIN32
	// Make sure to set this. Some classes, like the exception handler and custom cursors
	// will need to use it.
	gHInstance = hInstance;
#endif
	// Create and initialize our game application.
	GameApp* anApp = new GameApp();
	anApp->Init();

	// Starts the entire application: sets up the resource loading thread and 
	// custom cursor thread, and enters the game loop where the application
	// will remain until it is shut down. You will most likely not need to
	// override this function.
	anApp->Start();


	delete anApp;

	return 0;
}
