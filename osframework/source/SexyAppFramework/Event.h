#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef __cplusplus
namespace Sexy
{
#endif

enum EventType {
	EVENT_NONE		     = 0,
	EVENT_KEY_DOWN		     = 1,
	EVENT_KEY_UP		     = 2,
	EVENT_MOUSE_BUTTON_PRESS     = 3,
	EVENT_MOUSE_BUTTON_RELEASE   = 4,
	EVENT_MOUSE_WHEEL_UP	     = 5,
	EVENT_MOUSE_WHEEL_DOWN	     = 6,
	EVENT_MOUSE_MOTION	     = 7,
	EVENT_ACTIVE		     = 8,
	EVENT_EXPOSE		     = 9,
	EVENT_QUIT		     = 10,
	EVENT_DEVICE_SEARCHING       = 11,
	EVENT_DEVICE_LOST            = 12,
	EVENT_DEVICE_DISCOVERED      = 13,
	EVENT_ACC                    = 14,
	EVENT_ANGLE                  = 15,
	EVENT_TOUCH                  = 16,
	EVENT_MINIMIZED              = 17,
	EVENT_GYRO                   = 18,
	EVENT_AXIS_MOVED             = 19,
	EVENT_USER		     = 65535
};

#define MAKE_USER_EVENT_TYPE(a, b, c)				   \
    ((((a) & 0xff) << 24) | (((b) & 0xff) << 16) | ((c) & 0xffff)) \

#define EVENT_FLAGS_AXIS       (1U << 0)
#define EVENT_FLAGS_REL_AXIS   (1U << 1)
#define EVENT_FLAGS_BUTTON     (1U << 2)
#define EVENT_FLAGS_KEY_CODE   (1U << 3)
#define EVENT_FLAGS_KEY_CHAR   (1U << 4)
#define EVENT_FLAGS_AXIS_RANGE (1U << 5)
#define EVENT_FLAGS_TIMESTAMP  (1U << 6)
#define EVENT_FLAGS_REPEAT     (1U << 7)
#define EVENT_FLAGS_FAKED      (1U << 8)
#define EVENT_FLAGS_INCOMPLETE (1U << 31)

struct MouseEvent {
	int	       x;
	int	       y;
	int	       button;
	int            maxx;
	int            maxy;
};

struct KeyEvent {
	int	       keyCode;
	int	       keyChar;
};

struct ActiveEvent {
	int	       active;
};

struct DeviceEvent {
	int            prevCount;
	int            currCount;
};

struct AccEvent {
	int	       keyCode;
	unsigned int   acc;
	float          x;
	float          y;
	float          z;
};

struct GyroEvent {
	float          pitch;
	float          yaw;
	float          roll;
};

struct AngleEvent {
	float angleX;
	float angleY;
	float angleZ;
};

enum TouchState {
	TOUCH_DOWN   = 0,
	TOUCH_MOVE   = 1,
	TOUCH_UP     = 2,
	TOUCH_CANCEL = 3
};

struct TouchEvent {
	int id;
	int state;
	int x;
	int y;
	int pressure;
};

struct MinimizedEvent {
	int minimized;
};

enum Axis {
        AXIS_X        = 0x00,
        AXIS_Y        = 0x01,
        AXIS_Z        = 0x02,
        AXIS_RX       = 0x03,
        AXIS_RY       = 0x04,
        AXIS_RZ       = 0x05,
        AXIS_THROTTLE = 0x06,
        AXIS_RUDDER   = 0x07,
        AXIS_WHEEL    = 0x08,
        AXIS_GAS      = 0x09,
        AXIS_BRAKE    = 0x0a,
        AXIS_HAT0X    = 0x10,
        AXIS_HAT0Y    = 0x11,
        AXIS_HAT1X    = 0x12,
        AXIS_HAT1Y    = 0x13,
        AXIS_HAT2X    = 0x14,
        AXIS_HAT2Y    = 0x15,
        AXIS_HAT3X    = 0x16,
        AXIS_HAT3Y    = 0x17
};

struct AxisMoved {
        int   axis;
        float flat;
        float fuzz;
        float minimum;
        float maximum;
        float resolution;
        float value;
};

struct UserEvent {
	int	       reserved[7];
};

struct Event {
	enum EventType		      type;
	unsigned int		      flags;
	int			      id;
	int                           subid;
	unsigned int                  timestamp;
	union {
		struct MouseEvent     mouse;
		struct KeyEvent	      key;
		struct ActiveEvent    active;
		struct DeviceEvent    device;
		struct UserEvent      user;
		struct AccEvent       acc;
		struct AngleEvent     angle;
		struct TouchEvent     touch;
		struct MinimizedEvent minimized;
		struct GyroEvent      gyro;
		struct AxisMoved      axis;
	}			      u;
};

#ifdef __cplusplus
};
#endif

#endif
