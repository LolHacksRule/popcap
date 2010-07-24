#include "GameApp.h"

#include <UIKit/UIKit.h>

using namespace Sexy;

int main(int argc, char **argv)
{	
	GameApp* anApp = new GameApp();
	int retval = UIApplicationMain(argc, argv, nil, @"AppDelegate");
	delete anApp;
	
	return 0;
}