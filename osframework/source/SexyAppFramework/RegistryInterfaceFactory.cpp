#include "RegistryInterfaceFactory.h"
#include "XMLRegistryInterface.h"

#ifdef WIN32
#include "Win32RegistryInterface.h"
#endif

using namespace Sexy;

RegistryInterface* RegistryInterfaceFactory::GetInterface(SexyAppBase * theApp)
{
#ifdef WIN32
    return new Win32RegistryInterface(theApp);
#else
    return new XMLRegistryInterface(theApp);
#endif
}
