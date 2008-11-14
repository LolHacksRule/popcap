#ifndef __SMINPUTDRIVER_H__
#define __SMINPUTDRIVER_H__

#include "Common.h"
#include "InputInterface.h"

#include <pthread.h>

namespace Sexy
{

class SexyAppBase;
struct input_event;
class SMInputInterface: public InputInterface {
public:
        SMInputInterface (InputManager* theManager);
        virtual ~SMInputInterface ();

public:
        virtual bool          Init ();
        virtual void          Cleanup ();

        virtual bool          HasEvent ();
        virtual bool          GetEvent (Event & event);

private:
        bool                  OpenDevice ();
        void                  CloseDevice ();

        void                  Process (struct input_event& event,
                                       struct input_event& old_event);

        static void *         Run (void * data);

 private:
        int                   mFd;
        volatile int          mDone;
        pthread_t*            mThread;

        int                   mXferSize;
        bool                  mKeys[7];
};

}

#endif
