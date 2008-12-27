#ifndef __WIN32MODULELOADER_H__
#define __WIN32MODULELOADER_H__

#include "ModuleLoader.h"

namespace Sexy {
class Win32ModuleLoader : public ModuleLoader
{
 public:
        Win32ModuleLoader ();
        virtual ~Win32ModuleLoader ();

 public:
        virtual Module* Load (const char* thePath);
        virtual void Unload (Module* theModule);
};

class Win32Module: public Module
{
 public:
        Win32Module (HINSTANCE theHandle,
                     Win32ModuleLoader* theLoader);
        virtual ~Win32Module();

 public:
        virtual void* GetSymbol (const char* theSymbol);
        virtual std::string* GetLastError (void);

 private:
        HINSTANCE           mHandle;
        Win32ModuleLoader*  mLoader;
        std::string*        mError;
};
}

#endif
