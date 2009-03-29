#include "RegistryInterfaceFactory.h"
#include "XMLRegistryInterface.h"

#ifdef WIN32
#include "Win32RegistryInterface.h"
#endif

#ifdef __APPLE__
#include "DarwinRegistryInterface.h"
#endif

using namespace Sexy;

RegistryInterface* RegistryInterfaceFactory::GetInterface(SexyAppBase * theApp)
{
#if defined(WIN32)
	return new Win32RegistryInterface(theApp);
#elif defined (__APPLE__)
	return new DarwinRegistryInterface(theApp);
#else
	return new XMLRegistryInterface(theApp);
#endif
}
