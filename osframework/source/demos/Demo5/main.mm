#include "GameApp.h"

#include <UIKit/UIKit.h>

using namespace Sexy;

int main(int argc, char **argv)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];

	GameApp* anApp = new GameApp();

	int retval = UIApplicationMain(argc, argv, nil, @"AppDelegate");

	delete anApp;

	[pool release];
	return retval;
}
