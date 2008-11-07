#ifndef __MODULELOADER_H__
#define __MODULELOADER_H__

#include <string>

namespace Sexy {
class Module;
class ModuleLoader
{
 public:
        ModuleLoader ();
        virtual ~ModuleLoader ();

 public:
        virtual Module* Load (const char* thePath) = 0;
        virtual void Unload (Module* theModule) = 0;
};

class Module
{
 public:
        Module ();
        virtual ~Module();

 public:
        virtual void* GetSymbol (const char* theSymbol) = 0;
        virtual std::string* GetLastError (void) = 0;
};
}

#endif
