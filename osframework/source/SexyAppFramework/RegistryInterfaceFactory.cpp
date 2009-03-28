#include "RegistryInterfaceFactory.h"
#include "XMLRegistryInterface.h"

using namespace Sexy;

RegistryInterface* RegistryInterfaceFactory::GetInterface(SexyAppBase * theApp)
{
    return new XMLRegistryInterface(theApp);
}
