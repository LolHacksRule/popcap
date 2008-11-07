#include "ModuleManager.h"

#if defined(SEXY_POSIX_MODULE_LOADER)
#include "PosixModuleLoader.h"
#endif

using namespace Sexy;

ModuleManager::ModuleManager ()
{
#if defined(SEXY_POSIX_MODULE_LOADER)
	mLoader = new PosixModuleLoader ();
#else
	mLoader = 0;
#endif
}

ModuleManager::~ModuleManager ()
{
	delete mLoader;
}

ModuleManager* ModuleManager::GetModuleManager ()
{
	static ModuleManager * manager = 0;

	if (manager)
		return manager;

	manager = new ModuleManager;
	return manager;
}

Module* ModuleManager::LoadModule (const char * thePath)
{
	if (!mLoader)
		return 0;

	return mLoader->Load (thePath);
}
