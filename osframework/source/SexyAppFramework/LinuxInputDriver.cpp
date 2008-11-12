#include "LinuxInputDriver.h"
#include "InputDriverFactory.h"
#include "SexyAppBase.h"

#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <cstdlib>

#include <linux/input.h>
#include <linux/keyboard.h>

#ifndef EVIOCGLED
#define EVIOCGLED(len) _IOC(_IOC_READ, 'E', 0x19, len)
#endif

#ifndef EVIOCGRAB
#define EVIOCGRAB      _IOW('E', 0x90, int)
#endif

#define BITS_PER_LONG        (sizeof(long) * 8)
#define NBITS(x)             ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)               ((x) % BITS_PER_LONG)
#define BIT(x)               (1UL << OFF(x))
#define LONG(x)              ((x) / BITS_PER_LONG)
#undef test_bit
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

/* compat defines for older kernel like 2.4.x */
#ifndef EV_SYN
#define EV_SYN			0x00
#define SYN_REPORT              0
#define SYN_CONFIG              1
#define ABS_TOOL_WIDTH		0x1c
#define BTN_TOOL_DOUBLETAP	0x14d
#define BTN_TOOL_TRIPLETAP	0x14e
#endif

/*
 * Touchpads related stuff
 */
enum {
     TOUCHPAD_FSM_START,
     TOUCHPAD_FSM_MAIN,
     TOUCHPAD_FSM_DRAG_START,
     TOUCHPAD_FSM_DRAG_MAIN,
};
struct touchpad_axis {
     int old, min, max;
};
struct touchpad_fsm_state {
     int fsm_state;
     struct touchpad_axis x;
     struct touchpad_axis y;
     struct timeval timeout;
};

using namespace Sexy;

LinuxInputInterface::LinuxInputInterface (InputManager* theManager)
	: InputInterface (theManager), mFd (-1), mDone (false)
{
	mX = 0;
	mY = 0;
	mThread = NULL;
}

LinuxInputInterface::~LinuxInputInterface ()
{
    Cleanup ();
}

bool LinuxInputInterface::Init()
{
	if (mFd >= 0)
		Cleanup ();

	if (!OpenDevice ())
		goto open_failed;

	mRetry = 0;
	mDone = false;
	mThread = new pthread_t;
	if (!mThread)
		goto close_device;

	int ret;
	ret = pthread_create (mThread, NULL, LinuxInputInterface::Run, this);
	if (ret)
		goto delete_thread;
	return true;
 delete_thread:
	delete mThread;
	mThread = 0;
 close_device:
	CloseDevice ();
 open_failed:
	return false;
}

void LinuxInputInterface::Cleanup()
{
	if (mFd < 0)
		return;

	mDone = true;
	if (mThread)
		pthread_join (*mThread, NULL);
	delete mThread;
	mThread = 0;

	CloseDevice ();

	mX = 0;
	mY = 0;
}

static void
get_device_info (int              fd)
{
     unsigned int  num_keys     = 0;
     unsigned int  num_ext_keys = 0;
     unsigned int  num_buttons  = 0;
     unsigned int  num_rels     = 0;
     unsigned int  num_abs      = 0;

     char name[256];
     unsigned long eventbit[NBITS(EV_MAX)];
     unsigned long keybit[NBITS(KEY_MAX)];
     unsigned long relbit[NBITS(REL_MAX)];
     unsigned long absbit[NBITS(ABS_MAX)];

     /* get device name */
     ioctl (fd, EVIOCGNAME(255), name);
     printf ("device name: %s\n", name);

     /* get event type bits */
     ioctl (fd, EVIOCGBIT(0, sizeof(eventbit)), eventbit);

     if (test_bit (EV_KEY, eventbit))
     {
          int i;

          /* get keyboard bits */
          ioctl (fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit);

	  /**  count typical keyboard keys only */
          for (i = KEY_Q; i < KEY_M; i++)
               if (test_bit (i, keybit))
                    num_keys++;

          for (i = KEY_OK; i < KEY_MAX; i++)
               if (test_bit (i, keybit))
                    num_ext_keys++;

          for (i = BTN_MOUSE; i < BTN_JOYSTICK; i++)
               if (test_bit (i, keybit))
                    num_buttons++;
     }

     if (test_bit (EV_REL, eventbit))
     {
          int i;

          /* get bits for relative axes */
          ioctl (fd, EVIOCGBIT (EV_REL, sizeof(relbit)), relbit);

          for (i = 0; i < REL_MAX; i++)
               if (test_bit (i, relbit))
                    num_rels++;
     }

     if (test_bit (EV_ABS, eventbit))
     {
          int i;

          /* get bits for absolute axes */
          ioctl (fd, EVIOCGBIT (EV_ABS, sizeof (absbit)), absbit);

          for (i = 0; i < ABS_PRESSURE; i++)
               if (test_bit (i, absbit))
                    num_abs++;
     }

     printf ("keys: %d\n", num_keys);
     printf ("extend keys: %d\n", num_ext_keys);
     printf ("buttons: %d\n", num_buttons);
     printf ("relative events: %d\n", num_rels);
     printf ("absolute events: %d\n", num_abs);
}

bool LinuxInputInterface::OpenDevice ()
{
	const char * device = getenv ("SEXY_LINUX_INPUT_DEVICE");
	if (!device)
		device = "/dev/input/event0";
	mFd = open (device, O_RDWR);
	if (mFd < 0) {
		printf ("open mouse device failed.\n");
		goto open_failed;
	}

	int ret;
	ret = ioctl (mFd, EVIOCGRAB, 1);
	if (ret)
	{
		printf ("grab device failed.\n");
		goto close_fd;
	}

	get_device_info (mFd);
	return true;
 close_fd:
	close (mFd);
	mFd = -1;
 open_failed:
	return false;
}

void LinuxInputInterface::CloseDevice ()
{
	if (mFd < 0)
		return;

	ioctl (mFd, EVIOCGRAB, 0);
	close (mFd);
	mFd = -1;
}

static bool
handle_key_event (struct input_event & linux_event,
		  Event & event)
{
     if (linux_event.code == BTN_TOUCH || linux_event.code == BTN_TOOL_FINGER)
          linux_event.code = BTN_MOUSE;

	if ((linux_event.code >= BTN_MOUSE && linux_event.code < BTN_JOYSTICK) ||
	    linux_event.code == BTN_TOUCH)
	{
		if (linux_event.value == 1)
			event.type = EVENT_MOUSE_BUTTON_PRESS;
		else if (linux_event.value == 0)
			event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.flags = EVENT_FLAGS_BUTTON;
		if (linux_event.code == BTN_MOUSE)
			event.button = 1;
		else if (linux_event.code == (BTN_MOUSE + 1))
			event.button = 2;
		else if (linux_event.code == (BTN_MOUSE + 2))
			event.button = 3;
	}

	return true;
}


static bool
handle_rel_event (struct input_event & linux_event,
		  Event & event)
{
	switch (linux_event.code)
	{
	case REL_X:
		event.type = EVENT_MOUSE_MOTION;
		event.flags = EVENT_FLAGS_REL_AXIS;
		event.x = linux_event.value;
		break;
	case REL_Y:
		event.type = EVENT_MOUSE_MOTION;
		event.flags = EVENT_FLAGS_REL_AXIS;
		event.y = linux_event.value;
		break;
	default:
		break;
	}

	return true;
}

static bool
handle_abs_event (struct input_event & linux_event,
		  Event & event)
{
	switch (linux_event.code)
	{
	case ABS_X:
		event.type = EVENT_MOUSE_MOTION;
		event.flags = EVENT_FLAGS_AXIS;
		event.x = linux_event.value;
		break;
	case ABS_Y:
		event.type = EVENT_MOUSE_MOTION;
		event.flags = EVENT_FLAGS_AXIS;
		event.y = linux_event.value;
		break;
	default:
		break;
	}

	return true;
}

static bool handle_event (struct input_event& linux_event,
			  Event &event)
{
	switch (linux_event.type) {
	case EV_KEY:
		return handle_key_event (linux_event, event);
		break;
	case EV_REL:
		return handle_rel_event (linux_event, event);
	case EV_ABS:
		return handle_abs_event (linux_event, event);
	default:
		break;
	}
	return false;
}

void* LinuxInputInterface::Run (void * data)
{
	LinuxInputInterface * driver = (LinuxInputInterface *)data;
	struct input_event linux_event[64];
	fd_set             set;
	int                status;
	int                fd = driver->mFd;

	while (!driver->mDone)
	{
		if (driver->mFd >= 0)
		{
			FD_ZERO (&set);
			FD_SET (fd, &set);

			struct timeval timeout;

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			status = select (fd + 1, &set, NULL, NULL, &timeout);
			if (status < 0 && errno != EINTR)
			{
				printf ("device disconnected.\n");
				driver->CloseDevice ();
				continue;
			}
			if (status < 0)
				continue;
			if (driver->mDone)
				break;
			if (!FD_ISSET (fd, &set))
				continue;
		}
		else
		{
			if (driver->mRetry > 10)
				break;

			usleep (100);
			printf ("try to reopen the input device.\n");
			if (!driver->OpenDevice ())
				driver->mRetry++;
			else
				driver->mRetry = 0;
			continue;
		}

		ssize_t readlen;
		readlen = read (fd, linux_event, sizeof (linux_event));

		if (driver->mDone)
			break;

		if (readlen < 0 && errno != EINTR)
		{
			printf ("device disconnected.\n");
			driver->CloseDevice ();
			continue;
		}

		if (readlen <= 0)
			continue;

		readlen /= sizeof (linux_event[0]);
		for (int i = 0; i < readlen; i++) {
			Event event;

			event.type = EVENT_NONE;
			event.flags = 0;
			event.x = driver->mX;
			event.y = driver->mY;

			handle_event (linux_event[i], event);

			if (event.type == EVENT_MOUSE_MOTION)
			{
				if (event.flags & EVENT_FLAGS_AXIS)
				{
					driver->mX = event.x;
					driver->mY = event.y;
				}
			}
			if (event.type != EVENT_NONE)
			{
				if (0)
				{
					printf ("event.type: %d\n", event.type);
					printf ("event.x: %d\n", event.x);
					printf ("event.y: %d\n", event.y);
				}
				driver->PostEvent (event);
			}
		}
	}

	return NULL;
}

bool LinuxInputInterface::HasEvent ()
{
	return false;
}

bool LinuxInputInterface::GetEvent (Event & event)
{
	return false;
}

class LinuxInputDriver: public InputDriver {
public:
	LinuxInputDriver ()
	 : InputDriver("LinuxInputInterface", 0)
	{
	}

	InputInterface* Create (SexyAppBase * theApp)
	{
		return new LinuxInputInterface (theApp->mInputManager);
        }
};

static LinuxInputDriver aLinuxInputDriver;
InputDriverRegistor aLinuxInputDriverRegistor (&aLinuxInputDriver);
InputDriver* GetLinuxInputDriver()
{
	return &aLinuxInputDriver;
}
