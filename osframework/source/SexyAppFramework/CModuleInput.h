#ifndef __CMODULEINPUT_H__
#define __CMODULEINPUT_H__

#include "Event.h"

#ifdef __cplusplus
extern "C" {
#endif

struct InputModuleInfo {
        void* manager;

        int  width;
        int  height;
        int  reserved[8];

        void
        (* postevent) (void * manager, struct Event* event);

        void
        (* reserved1) (void);

        void
        (* reserved2) (void);

        void
        (* reserved3) (void);

        void
        (* reserved4) (void);
};

typedef void* (* InputModuleOpenFunc) (const struct InputModuleInfo*);
typedef void (* InputModuleCloseFunc) (void*);

#ifdef __cplusplus
}
#endif

#endif
