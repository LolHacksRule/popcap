#ifndef __POSIXMODULELOADER_H__
#define __POSIXMODULELOADER_H__

#include "ModuleLoader.h"

#include <dlfcn.h>

namespace Sexy {
class PosixModuleLoader : public ModuleLoader
{
 public:
        PosixModuleLoader ();
        virtual ~PosixModuleLoader ();

 public:
        virtual Module* Load (const char* thePath);
        virtual void Unload (Module* theModule);
};

class PosixModule: public Module
{
 public:
        PosixModule (void* theHandle,
                     PosixModuleLoader* theLoader);
        virtual ~PosixModule();

 public:
        virtual void* GetSymbol (const char* theSymbol);
        virtual std::string* GetLastError (void);

 private:
        void*               mHandle;
        PosixModuleLoader*  mLoader;
        std::string*        mError;
};
}

#endif
