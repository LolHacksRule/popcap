//////////////////////////////////////////////////////////////////////////
//						main.cpp
//
//	This is the starting point for all new projects. This file's purpose is
//	pretty small, but important. In here we create our application, initialize
//	it, and begin processing all the game code.
//
//	This demo will teach you:
//	* Loading and displaying fonts
//	* Loading and displaying images
//	* Colorizing images
//	* Additive drawing
//	* Palletizing images to use less RAM (when possible)
//	* Loading/playing sounds
//	* Widget introduction: buttons, listeners, events, adding/removing
//////////////////////////////////////////////////////////////////////////

#include "GameApp.h"

// The SexyAppFramework resides in the "Sexy" namespace. As a convenience,
// you'll see in all the .cpp files "using namespace Sexy" to avoid
// having to prefix everything with Sexy::
using namespace Sexy;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	

	// Make sure to set this. Some classes, like the exception handler and custom cursors
	// will need to use it.
	gHInstance = hInstance;

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
