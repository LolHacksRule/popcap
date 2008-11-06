#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <stdint.h>
#include <cstdio>
#include <signal.h>

#include <linux/input.h>
#include <linux/keyboard.h>

#include <string>

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
	bool           active;
};

/** Udp Input Protocol
 * uint32 type     the event type
 * uint32 flags    the event flags
 * uint32 x        the x coord
 * uint32 y        the y coord
 * uint32 button   the button
 * uint32 key_code the key code
 * uint32 key_char the key charactor
 * uint32 active   the active
 */

struct UdpInput {
	uint32_t type;
	uint32_t flags;
	uint32_t x;
	uint32_t y;
	uint32_t button;
	uint32_t key_code;
	uint32_t key_char;
	uint32_t active;
	uint32_t reserved[2];
};

#define UDP_INPUT_PORT   "2799"

class InputServer {
public:
        InputServer (const char * device = "/dev/input/event0",
		     const char * host = "127.0.0.1",
		     const char * port = UDP_INPUT_PORT);
        virtual ~InputServer ();

public:
        virtual bool          Init ();
        virtual void          Cleanup ();

public:
        bool                  OpenDevice ();
        void                  CloseDevice ();

	void                  PostEvent (Event &event);

        void                  Run ();
	static void           SignalHandler (int num);

 private:
        int                   mFd;
	int                   mSock;

	std::string           mDevice;
	std::string           mHost;
	std::string           mPort;

        int                   mX;
        int                   mY;
        int                   mRetry;

	struct sockaddr_in    mSockaddr;
};

static volatile bool done = false;

InputServer::InputServer (const char * device, const char * host,
			  const char * port)
	: mFd (-1), mSock (-1), mDevice (device),
	  mHost (host), mPort (port)
{
	mX = 0;
	mY = 0;

	printf ("device: %s\n", device);
	printf ("host: %s\n", host);
	printf ("port: %s\n", port);
}

InputServer::~InputServer ()
{
    Cleanup ();
}

bool InputServer::Init()
{
	if (mFd >= 0)
		Cleanup ();

	if (!OpenDevice ())
		return false;

	mRetry = 0;
	done = false;

	return true;
}

void InputServer::Cleanup()
{
	if (mFd < 0)
		return;

	done = true;
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

void InputServer::SignalHandler (int num)
{
	printf ("caught a signal(%d(%x)).\n", num, num);
	done = true;
}

bool InputServer::OpenDevice ()
{
	mFd = open (mDevice.c_str (), O_RDWR);
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

	mSock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mSock < 0)
		goto ungrab_device;

	mSockaddr.sin_family = PF_INET;
	mSockaddr.sin_port = htons (atoi (mPort.c_str ()));
	mSockaddr.sin_addr.s_addr = inet_addr (mHost.c_str ());

	signal (SIGSEGV, SignalHandler);
	signal (SIGTERM, SignalHandler);
	signal (SIGKILL, SignalHandler);
	signal (SIGILL, SignalHandler);
	signal (SIGQUIT, SignalHandler);
	signal (SIGINT, SignalHandler);

	return true;
 ungrab_device:
	ioctl (mFd, EVIOCGRAB, 0);
 close_fd:
	close (mFd);
	mFd = -1;
 open_failed:
	return false;
}

void InputServer::CloseDevice ()
{
	if (mFd < 0)
		return;

	ioctl (mFd, EVIOCGRAB, 0);
	close (mFd);

	close (mSock);
	mSock = -1;
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

static int write32 (unsigned char* buf,
		    uint32_t u)
{
	buf[0] = (u >> 24) & 0xff;
	buf[1] = (u >> 16) & 0xff;
	buf[2] = (u >>  8) & 0xff;
	buf[3] = (u >>  0) & 0xff;

	return 4;
}

void InputServer::PostEvent (Event &event)
{
	unsigned char buf[sizeof (struct UdpInput) * sizeof (uint32_t)];
	unsigned int offset = 0;

	printf ("event type: %d\n", event.type);
	printf ("event x: %d\n", event.x);
	printf ("event y: %d\n", event.y);

	offset += write32 (buf + offset, (uint32_t)event.type);
	offset += write32 (buf + offset, (uint32_t)event.flags);
	offset += write32 (buf + offset, (uint32_t)event.x);
	offset += write32 (buf + offset, (uint32_t)event.y);
	offset += write32 (buf + offset, (uint32_t)event.button);
	offset += write32 (buf + offset, (uint32_t)event.keyCode);
	offset += write32 (buf + offset, (uint32_t)event.keyChar);
	offset += write32 (buf + offset, (uint32_t)event.active);
	offset += write32 (buf + offset, 0);
	offset += write32 (buf + offset, 0);

	sendto (mSock, buf, sizeof (buf), 0,
		(const struct sockaddr *)&mSockaddr,
		sizeof (mSockaddr));
}

void InputServer::Run ()
{
	struct input_event linux_event[64];
	fd_set             set;
	int                status;
	int                fd = mFd;

	while (!done)
	{
		if (mFd >= 0)
		{
			FD_ZERO (&set);
			FD_SET (fd, &set);

			status = select (fd + 1, &set, NULL, NULL, NULL);
			if (status < 0 && errno != EINTR)
			{
				printf ("device disconnected.\n");
				CloseDevice ();
				continue;
			}
			if (status < 0)
				continue;
			if (done)
				break;
		}
		else
		{
			if (mRetry > 10)
				break;

			usleep (100);
			printf ("try to reopen the input device.\n");
			if (!OpenDevice ())
				mRetry++;
			else
				mRetry = 0;
			continue;
		}

		ssize_t readlen;
		readlen = read (fd, linux_event, sizeof (linux_event));

		if (done)
			break;

		if (readlen < 0 && errno != EINTR)
		{
			printf ("device disconnected.\n");
			CloseDevice ();
			continue;
		}

		if (readlen <= 0)
			continue;

		readlen /= sizeof (linux_event[0]);
		for (int i = 0; i < readlen; i++) {
			Event event;

			event.type = EVENT_NONE;
			event.flags = 0;
			event.x = mX;
			event.y = mY;

			handle_event (linux_event[i], event);

			if (event.type == EVENT_MOUSE_MOTION)
			{
				if (event.flags & EVENT_FLAGS_AXIS)
				{
					mX = event.x;
					mY = event.y;
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
				PostEvent (event);
			}
		}
	}
}

int main (int argc, char ** argv)
{
	if (argc < 3)
	{
		printf ("usage: luis device [host port]\n");
		return -1;
	}

	InputServer server (argv[1], argv[2]);

	server.Init ();
	server.Run ();
	server.Cleanup ();
	return 0;
}
