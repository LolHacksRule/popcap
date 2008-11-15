#include "SMInputDriver.h"
#include "InputDriverFactory.h"
#include "SexyAppBase.h"

#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <cstdlib>

using namespace Sexy;

namespace Sexy {
struct input_event {
	signed char button;
	signed char x;
	signed char y;
	signed char wheel;
	unsigned char roll;
	signed char reserved[3];
};
}

SMInputInterface::SMInputInterface (InputManager* theManager)
	: InputInterface (theManager), mFd (-1), mDone (false)
{
	mThread = NULL;
}

SMInputInterface::~SMInputInterface ()
{
    Cleanup ();
}

bool SMInputInterface::Init()
{
	if (mFd >= 0)
		Cleanup ();

	if (!OpenDevice ())
		goto open_failed;

	mDone = false;
	mThread = new pthread_t;
	if (!mThread)
		goto close_device;

	int ret;
	ret = pthread_create (mThread, NULL, SMInputInterface::Run, this);
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

void SMInputInterface::Cleanup()
{
	if (mFd < 0)
		return;

	mDone = true;
	if (mThread)
		pthread_join (*mThread, NULL);
	delete mThread;
	mThread = 0;

	CloseDevice ();
}

bool SMInputInterface::OpenDevice ()
{
	const char * device = getenv ("SEXY_SM_INPUT_DEVICE");
	if (!device)
		device = "/dev/msid";
	mFd = open (device, O_RDWR);
	if (mFd < 0) {
		printf ("open mouse device failed.\n");
		goto open_failed;
	}

	int ret;
	ret = ioctl (mFd, 1, &mXferSize);
	if (ret)
	{
		printf ("get device info failed.\n");
		goto close_fd;
	}
	memset (&mKeys, 0, sizeof (mKeys));

	return true;
 close_fd:
	close (mFd);
	mFd = -1;
 open_failed:
	return false;
}

void SMInputInterface::CloseDevice ()
{
	if (mFd < 0)
		return;

	close (mFd);
	mFd = -1;
}

static bool
handle_key_event (int index, bool pressed,
		  Event & event)
{
	if (pressed)
		event.type = EVENT_MOUSE_BUTTON_PRESS;
	else
		event.type = EVENT_MOUSE_BUTTON_RELEASE;

	if (index == 0)
		event.button = 1;
	else if (index == 1)
		event.button = 2;
	else if (index == 6)
		event.button = 3;
	else
		return false;

	return true;
}


static bool
handle_rel_event (struct input_event & sm_event,
		  Event & event)
{
	event.type = EVENT_MOUSE_MOTION;
	event.flags = EVENT_FLAGS_REL_AXIS;
	event.x = sm_event.x;
	event.y = sm_event.y;

	return true;
}

void SMInputInterface::Process (struct input_event& sm_event,
				struct input_event& sm_old_event)
{
	Event event;

	if (0)
	{
		printf ("event:\n");
		printf ("\tbutton: 0x%x\n", sm_event.button);
		printf ("\tx: %d\n", sm_event.x);
		printf ("\ty: %d\n", sm_event.y);
	}

	if ((sm_event.button & 0x7f) != (sm_old_event.button & 0x7f))
	{
		/* key pressed or released */
		bool keys[7];

		for (int i = 0; i < 7; i++)
			keys[i] = sm_event.button & (1 << i) ? true : false;

		for (int i = 0; i < 7; i++)
		{
			if (keys[i] != mKeys[i])
			{
				event.type = EVENT_NONE;
				event.flags = 0;

				if (handle_key_event (i, keys[i], event))
					PostEvent (event);
			}
		}
		memcpy (&mKeys, &keys, sizeof (mKeys));
	}

	event.type = EVENT_NONE;
	event.flags = 0;
	if (handle_rel_event (sm_event, event))
		PostEvent (event);

	sm_old_event = sm_event;
}

void* SMInputInterface::Run (void * data)
{
	SMInputInterface * driver = (SMInputInterface *)data;
	struct input_event sm_old_event;
	struct input_event sm_event[64];
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
				memset (&sm_old_event, 0, sizeof (sm_old_event));
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
			usleep (100);
			printf ("try to reopen the input device.\n");
			driver->OpenDevice ();
			continue;
		}

		ssize_t readlen;
		readlen = read (fd, sm_event, sizeof (sm_event));

		if (driver->mDone)
			break;

		if (readlen < 0 && errno != EINTR)
		{
			printf ("device disconnected.\n");
			driver->CloseDevice ();
			memset (&sm_old_event, 0, sizeof (sm_old_event));
			continue;
		}

		if (readlen <= 0)
			continue;

		readlen /= sizeof (sm_event[0]);
		for (int i = 0; i < readlen; i++)
			driver->Process (sm_event[i], sm_old_event);
	}

	return NULL;
}

bool SMInputInterface::HasEvent ()
{
	return false;
}

bool SMInputInterface::GetEvent (Event & event)
{
	return false;
}

class SMInputDriver: public InputDriver {
public:
	SMInputDriver ()
	 : InputDriver("SMInputInterface", 0)
	{
	}

	InputInterface* Create (SexyAppBase * theApp)
	{
		return new SMInputInterface (theApp->mInputManager);
        }
};

static SMInputDriver aSMInputDriver;
InputDriverRegistor aSMInputDriverRegistor (&aSMInputDriver);
InputDriver* GetSMInputDriver()
{
	return &aSMInputDriver;
}