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

#ifdef __APPLE__
#define MODULE_SUFFIX ".dylib"
#else
#define MODULE_SUFFIX ".so"
#endif

Module* PosixModuleLoader::Load (const char* thePath)
{
	void* handle;

	handle = dlopen (thePath, RTLD_LOCAL | RTLD_LAZY);
	if (!handle)
	{
		std::string fileName (thePath);
		std::string::size_type lastSlash;
		std::string::size_type firstPrefix;
		std::string::size_type lastSuffix;
		std::string baseName;
		std::string altName;

		lastSlash = fileName.rfind ("/");
		if (lastSlash != std::string::npos)
			baseName = fileName.substr (lastSlash + 1);
		else
			baseName = fileName;
		firstPrefix = baseName.find ("lib");
		lastSuffix = baseName.rfind (MODULE_SUFFIX);
		if (lastSlash != std::string::npos)
			altName = fileName.substr (0, lastSlash);
		if (firstPrefix == std::string::npos)
			altName += "lib";
		altName += baseName;
		if (lastSuffix == std::string::npos)
			altName += MODULE_SUFFIX;

		handle = dlopen (altName.c_str (), RTLD_LOCAL | RTLD_LAZY);
		if (!handle)
		{
			printf ("dlerror: %s\n", dlerror ());
			return 0;
		}
	}

	return new PosixModule (handle, this);
}

void PosixModuleLoader::Unload (Module* theModule)
{
	delete theModule;
}

PosixModule::PosixModule (void* theHandle,
			  PosixModuleLoader* theLoader) :
	mHandle (theHandle), mLoader (theLoader),
	mError (0)
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
