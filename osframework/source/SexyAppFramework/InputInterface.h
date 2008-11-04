#ifndef __INPUTINTERFACE_H__
#define __INPUTINTERFACE_H__

#include "Common.h"
#include "NativeDisplay.h"

namespace Sexy
{

class InputInterface {
 public:
        InputInterface();

        virtual ~InputInterface();

 public:
        virtual bool          Init();
        virtual void          Cleanup();

        virtual bool          HasEvent() = 0;
        virtual bool          GetEvent(Event & event) = 0;
};

}

#endif
