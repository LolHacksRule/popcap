#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef __cplusplus
namespace Sexy
{
#endif

enum EventType {
	EVENT_NONE		    = 0,
	EVENT_KEY_DOWN		    = 1,
	EVENT_KEY_UP		    = 2,
	EVENT_MOUSE_BUTTON_PRESS    = 3,
	EVENT_MOUSE_BUTTON_RELEASE  = 4,
	EVENT_MOUSE_WHEEL_UP	    = 5,
	EVENT_MOUSE_WHEEL_DOWN	    = 6,
	EVENT_MOUSE_MOTION	    = 7,
	EVENT_ACTIVE		    = 8,
	EVENT_EXPOSE		    = 9,
	EVENT_QUIT		    = 10,
	EVENT_USER		    = 65536
};

#define MAKE_USER_EVENT_TYPE(a, b, c)				   \
    ((((a) & 0xff) << 24) | (((b) & 0xff) << 16) | ((c) & 0xffff)) \

#define EVENT_FLAGS_AXIS     (1U << 0)
#define EVENT_FLAGS_REL_AXIS (1U << 1)
#define EVENT_FLAGS_BUTTON   (1U << 2)
#define EVENT_FLAGS_KEY_CODE (1U << 3)
#define EVENT_FLAGS_KEY_CHAR (1U << 4)

struct MouseEvent {
	int	       x;
	int	       y;
	int	       button;
};

struct KeyEvent {
	int	       keyCode;
	int	       keyChar;
};

struct ActiveEvent {
	int	       active;
};

struct UserEvent {
	int	       reserved[8];
};

struct Event {
	enum EventType		     type;
	unsigned int		     flags;
	int			     id;
	union {
		struct MouseEvent    mouse;
		struct KeyEvent	     key;
		struct ActiveEvent   active;
		struct UserEvent     user;
	}			     u;
};

#ifdef __cplusplus
};
#endif

#endif
