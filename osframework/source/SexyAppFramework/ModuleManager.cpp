#include "Common.h"
#include "ModuleManager.h"

#if defined(SEXY_POSIX_MODULE_LOADER)
#include "PosixModuleLoader.h"
#endif

#if defined(SEXY_WIN32_MODULE_LOADER)
#include "Win32ModuleLoader.h"
#endif

using namespace Sexy;

ModuleManager::ModuleManager ()
{
#if defined(SEXY_POSIX_MODULE_LOADER)
	mLoader = new PosixModuleLoader ();
#elif defined(SEXY_WIN32_MODULE_LOADER)
	mLoader = new Win32ModuleLoader ();
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

void ModuleManager::UnloadModule (Module* theModule)
{
	if (!mLoader)
		return;

	mLoader->Unload (theModule);
}
