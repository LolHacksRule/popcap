#include "Common.h"
#include "Win32ModuleLoader.h"

using namespace Sexy;

Win32ModuleLoader::Win32ModuleLoader ()
{
}

Win32ModuleLoader::~Win32ModuleLoader ()
{
}

#define MODULE_SUFFIX ".dll"

static void
GetLastErrorMessage (std::string & result)
{
	DWORD error;
	LPTSTR buffer = NULL;

	error = GetLastError ();

	FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error,
			0, /* Default language */
			(LPTSTR) &buffer, 0, NULL);
	result = std::string (buffer);

	if (result[result.length() - 1] == '\n')
		result[result.length() - 1] = '\0';

	LocalFree (buffer);
}

Module* Win32ModuleLoader::Load (const char* thePath)
{
	HINSTANCE handle;
	UINT old_mode;

	old_mode = SetErrorMode (0);
	SetErrorMode (old_mode &~ SEM_NOOPENFILEERRORBOX);

	handle = LoadLibrary (thePath);
	if (!handle)
	{
		std::string fileName (thePath);
		std::string::size_type lastSlash;
		std::string::size_type lastBackSlash;
		std::string::size_type lastSuffix;
		std::string baseName;
		std::string altName;

		lastSlash = fileName.rfind ("/");
		lastBackSlash = fileName.rfind ("\\");
		lastSlash = std::min(lastSlash, lastBackSlash);
		if (lastSlash != std::string::npos)
			baseName = fileName.substr (lastSlash + 1);
		else
			baseName = fileName;
		lastSuffix = baseName.rfind (MODULE_SUFFIX);
		if (lastSlash != std::string::npos)
			altName = fileName.substr (0, lastSlash);
		altName += baseName;
		if (lastSuffix == std::string::npos)
			altName += MODULE_SUFFIX;

		handle = LoadLibrary (altName.c_str ());
		if (!handle)
		{
			std::string msg;

			GetLastErrorMessage (msg);
#if defined(SEXY_DEBUG) || defined(DEBUG)
			fprintf (stderr, "LoadLibrary error: %s (%s)\n",
				 msg.c_str(), altName.c_str ());
#endif
		}
	}

	SetErrorMode (old_mode);

	if (!handle)
		return 0;
	return new Win32Module (handle, this);
}

void Win32ModuleLoader::Unload (Module* theModule)
{
	delete theModule;
}

Win32Module::Win32Module (HINSTANCE theHandle,
			  Win32ModuleLoader* theLoader) :
	mHandle (theHandle), mLoader (theLoader),
	mError (0)
{
}

Win32Module::~Win32Module()
{
	FreeLibrary (mHandle);
}

void* Win32Module::GetSymbol (const char* theSymbol)
{
	void* sym;

	sym = (void*)GetProcAddress (mHandle, theSymbol);

	std::string msg;
	GetLastErrorMessage (msg);

	delete mError;
	mError = new std::string (msg);
	return sym;
}

std::string* Win32Module::GetLastError (void)
{
	if (!mError)
		return 0;
	return new std::string (*mError);
}
