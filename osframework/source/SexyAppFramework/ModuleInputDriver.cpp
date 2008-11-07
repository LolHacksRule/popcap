#include "ModuleInputDriver.h"
#include "InputDriverFactory.h"
#include "SexyAppBase.h"

using namespace Sexy;

ModuleInputInterface::ModuleInputInterface (SexyAppBase* theApp,
					    std::string theModuleName)
	: InputInterface (theApp->mInputManager),
	  mApp (theApp), mModule (0), mModuleName (theModuleName),
	  mHandle (0)
{
	mModuleName = std::string ("SexyInputModule");
}

ModuleInputInterface::~ModuleInputInterface ()
{
	Cleanup ();
}

bool ModuleInputInterface::Init()
{
	return OpenDevice ();
}

void ModuleInputInterface::Cleanup()
{
	CloseDevice ();
}

bool ModuleInputInterface::OpenDevice ()
{
	ModuleManager* manager = ModuleManager::GetModuleManager ();
	mModule = manager->LoadModule (mModuleName.c_str ());
	if (!mModule)
		goto open_failed;
	mOpen = (InputModuleOpen)mModule->GetSymbol ("InputModuleOpen");
	mClose = (InputModuleClose)mModule->GetSymbol ("InputModuleClose");
	if (!mOpen || !mClose)
		goto close_module;

	struct InputModuleInfo info;

	memset (&info, 0, sizeof (info));
	info.width = mApp->mWidth;
	info.height = mApp->mHeight;
	mHandle = mOpen (&info);
	if (!mHandle)
		goto close_module;
	return true;
 close_module:
	manager->UnloadModule (mModule);
	mModule = 0;
 open_failed:
	return false;
}

void ModuleInputInterface::CloseDevice ()
{
	if (!mHandle)
		return;

	ModuleManager* manager = ModuleManager::GetModuleManager ();

	mClose (mHandle);
	manager->UnloadModule (mModule);

	mHandle = 0;
	mModule = 0;
	mOpen = 0;
	mClose = 0;
}

bool ModuleInputInterface::HasEvent ()
{
	return false;
}

bool ModuleInputInterface::GetEvent (Event & event)
{
	return false;
}

class ModuleInputDriver: public InputDriver {
public:
	ModuleInputDriver ()
	 : InputDriver("ModuleInputInterface", 0)
	{
	}

	InputInterface* Create (SexyAppBase * theApp)
	{
		return new ModuleInputInterface (theApp);
        }
};

static ModuleInputDriver aModuleInputDriver;
InputDriverRegistor aModuleInputDriverRegistor (&aModuleInputDriver);
InputDriver* GetModuleInputDriver()
{
	return &aModuleInputDriver;
}
