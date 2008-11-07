#include "PosixModuleLoader.h"

using namespace Sexy;

PosixModuleLoader::PosixModuleLoader ()
{
}

PosixModuleLoader::~PosixModuleLoader ()
{
}

#ifndef RTLD_LOCAL
#define RTLD_LOCAL 0
#endif

#ifndef RTLD_LAZY
#define RTLD_LAZY
#endif

Module* PosixModuleLoader::Load (const char* thePath)
{
	void* handle;

	handle = dlopen (thePath, RTLD_LOCAL | RTLD_LAZY);
	if (!handle)
	{
		printf ("dlerror: %s\n", dlerror ());
		return 0;
	}

	return new PosixModule (handle, this);
}

void PosixModuleLoader::Unload (Module* theModule)
{
	delete theModule;
}

PosixModule::PosixModule (void* theHandle,
			  PosixModuleLoader* theLoader) :
	mHandle (theHandle), mLoader (theLoader)
{
}

PosixModule::~PosixModule()
{
	dlclose (mHandle);
	dlerror ();
}

void* PosixModule::GetSymbol (const char* theSymbol)
{
	void* sym;

	sym = dlsym (mHandle, theSymbol);

	char * error = dlerror ();
	delete mError;
	mError = 0;
	if (error)
		mError = new std::string (error);
	return sym;
}

std::string* PosixModule::GetLastError (void)
{
	if (!mError)
		return 0;
	return new std::string (*mError);
}
