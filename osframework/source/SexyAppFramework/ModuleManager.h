#ifndef __MODULEMANAGER_H__
#define __MODULEMANAGER_H__

#include "ModuleLoader.h"

namespace Sexy
{
class ModuleManager
{
public:
        ModuleManager ();
        ~ModuleManager ();

public:
        static ModuleManager* GetModuleManager ();
        Module* LoadModule (const char * thePath);

private:
        ModuleLoader*        mLoader;
};
}

#endif
