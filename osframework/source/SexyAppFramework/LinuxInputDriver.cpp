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

using namespace Sexy;

LinuxInputInterface::LinuxInputInterface (InputManager* theManager)
	: InputInterface (theManager), mFd (-1), mDone (false)
{
}

LinuxInputInterface::~LinuxInputInterface ()
{
}

bool LinuxInputInterface::Init()
{
	if (mFd >= 0)
		Cleanup ();

	mFd = open ("/dev/input/mice", O_RDWR);
	if (mFd < 0) {
		printf ("open mouse device failed.");
		goto open_failed;
	}

	int ret;
	ret = ioctl (mFd, EVIOCGRAB, 1);
	if (ret)
	{
		printf ("grab device failed.\n");
		goto close_fd;
	}

	mDone = false;
	mThread = new pthread_t;
	if (mThread)
		goto ungrab_fd;
	ret = pthread_create (mThread, NULL, LinuxInputInterface::Run, this);
	if (ret)
		goto delete_thread;
	return true;
 delete_thread:
	delete mThread;
	mThread = 0;
 ungrab_fd:
	ioctl (mFd, EVIOCGRAB, 0);
 close_fd:
	close (mFd);
	mFd = -1;
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

	ioctl (mFd, EVIOCGRAB, 0);
	close (mFd);
	mFd = -1;
}

void* LinuxInputInterface::Run (void * data)
{
	LinuxInputInterface * driver = (LinuxInputInterface *)data;

	while (!driver->mDone)
	{
		usleep (1);
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
	 : InputDriver("LinuxInputInterface", 10)
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
