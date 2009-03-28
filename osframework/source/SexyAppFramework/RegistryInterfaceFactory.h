#ifndef REGISTRYINTERFACEFACTORY_H
#define REGISTRYINTERFACEFACTORY_H

#include "RegistryInterface.h"

namespace Sexy
{
	class RegistryInterfaceFactory
	{
	public:
		static RegistryInterface* GetInterface(SexyAppBase* theApp);
	};
}

#endif
