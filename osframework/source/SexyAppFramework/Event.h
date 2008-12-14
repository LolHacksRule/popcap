#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef __cplusplus
namespace Sexy
{
#endif

enum EventType {
        EVENT_NONE                  = 0,
        EVENT_KEY_DOWN              = 1,
        EVENT_KEY_UP                = 2,
        EVENT_MOUSE_BUTTON_PRESS    = 3,
        EVENT_MOUSE_BUTTON_RELEASE  = 4,
        EVENT_MOUSE_WHEEL_UP        = 5,
        EVENT_MOUSE_WHELL_DOWN      = 6,
        EVENT_MOUSE_MOTION          = 7,
        EVENT_ACTIVE                = 8,
        EVENT_EXPOSE                = 9,
        EVENT_QUIT
};

#define EVENT_FLAGS_AXIS     (1U << 0)
#define EVENT_FLAGS_REL_AXIS (1U << 1)
#define EVENT_FLAGS_BUTTON   (1U << 2)
#define EVENT_FLAGS_KEY_CODE (1U << 3)
#define EVENT_FLAGS_KEY_CHAR (1U << 4)

struct Event {
        enum EventType type;
        unsigned int   flags;
        int            keyCode;
        int            keyChar;
        int            x;
        int            y;
        int            button;
        int            active;
};

#ifdef __cplusplus
};
#endif

#endif
