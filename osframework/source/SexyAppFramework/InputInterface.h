#ifndef __INPUTINTERFACE_H__
#define __INPUTINTERFACE_H__

#include "Common.h"
#include "NativeDisplay.h"

namespace Sexy
{

class InputManager;
class InputInterface {
 public:
        InputInterface(InputManager* theManager);

        virtual ~InputInterface();

 public:
        virtual bool          Init();
        virtual void          Cleanup();

        virtual bool          HasEvent () = 0;
        virtual bool          GetEvent (Event & event) = 0;
        virtual void          Update ();

 protected:
        InputManager*         mManager;
};

}

#endif
