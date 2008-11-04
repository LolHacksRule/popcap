#ifndef __LINUXINPUTDRIVER_H__
#define __LINUXINPUTDRIVER_H__

#include "Common.h"
#include "InputInterface.h"

namespace Sexy
{

class SexyAppBase;
class LinuxInputInterface: public InputInterface {
public:
        LinuxInputInterface ();
        virtual ~LinuxInputInterface ();

public:
        virtual bool          HasEvent ();
        virtual bool          GetEvent (Event & event);
};

}

#endif
