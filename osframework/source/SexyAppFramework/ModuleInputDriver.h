#ifndef __MODULEINPUTDRIVER_H__
#define __MODULEINPUTDRIVER_H__

#include "Common.h"
#include "InputInterface.h"
#include "ModuleManager.h"
#include "CModuleInput.h"

namespace Sexy
{

class SexyAppBase;
class ModuleInputInterface: public InputInterface {
public:
        ModuleInputInterface (SexyAppBase* theApp,
                              std::string theModuleName = "");
        virtual ~ModuleInputInterface ();

public:
        virtual bool          Init ();
        virtual void          Cleanup ();

        virtual bool          HasEvent ();
        virtual bool          GetEvent (Event & event);

private:
        bool                  OpenDevice ();
        void                  CloseDevice ();

private:
        SexyAppBase*          mApp;

        Module*               mModule;
        std::string           mModuleName;
        void*                 mHandle;

        InputModuleOpen       mOpen;
        InputModuleClose      mClose;
};

}

#endif
