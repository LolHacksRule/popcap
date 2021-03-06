#include "UdpInputDriver.h"
#include "InputDriverFactory.h"
#include "SexyAppBase.h"
#include "InputManager.h"

#include <cstring>

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include <process.h>
#include <winsock.h>
#endif

#include <cstdlib>

using namespace Sexy;

#if 0
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
#endif

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

#define UDP_INPUT_PORT   2799
#define UDP_INPUT_HOST   ""

UdpInputInterface::UdpInputInterface (InputManager* theManager)
	: InputInterface (theManager), mFd (-1), mDone (false)
{
	mX = 0;
	mY = 0;
	mHasKey = false;
	mHasPointer = false;
	mKeyCount = 0;
	mPointerCount = 0;
}

UdpInputInterface::~UdpInputInterface ()
{
	Cleanup ();
}

bool UdpInputInterface::Init()
{
#ifdef WIN32
	WSADATA aData;
	WSAStartup(MAKEWORD(1, 1), &aData);
#endif

	if (mFd >= 0)
		Cleanup ();

	if (!OpenDevice ())
		return false;

	mDone = false;
	mThread = Thread::Create(UdpInputInterface::Run, this);
	return true;
}

void UdpInputInterface::Cleanup()
{
	if (mFd < 0)
		return;

	mDone = true;
	mThread.Join();

	CloseDevice ();

	mX = 0;
	mY = 0;
	mHasKey = false;
	mHasPointer = false;
	mKeyCount = 0;
	mPointerCount = 0;

#ifdef WIN32
	WSACleanup();
#endif
}

bool UdpInputInterface::OpenDevice ()
{
	mFd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mFd < 0)
		goto open_failed;

	struct sockaddr_in sockaddr;

	memset (&sockaddr, 0, sizeof (sockaddr));
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_port = htons (UDP_INPUT_PORT);
	sockaddr.sin_addr.s_addr = htonl (INADDR_ANY);

	int ret;
	ret = bind (mFd, (struct sockaddr*)&sockaddr, sizeof (sockaddr));
	if (ret < 0)
		goto close_fd;
	return true;
 close_fd:
	close (mFd);
	mFd = -1;
 open_failed:
	return false;
}

void UdpInputInterface::CloseDevice ()
{
	if (mFd < 0)
		return;

	close (mFd);
	mFd = -1;
}

static uint32_t read32(uint32_t i)
{
	unsigned char *byte = (unsigned char*)&i;

	return  byte[0] << 24 |
		byte[1] << 16 |
		byte[2] <<  8 |
		byte[3] <<  0;
}

static void handle_event (struct UdpInput &input,
			  Event &event)
{
	event.type = (EventType)read32 (input.type);
	event.flags = read32 (input.flags);
	if (event.type == EVENT_MOUSE_BUTTON_PRESS ||
	    event.type == EVENT_MOUSE_BUTTON_RELEASE ||
	    event.type == EVENT_MOUSE_WHEEL_UP ||
	    event.type == EVENT_MOUSE_WHEEL_DOWN ||
	    event.type == EVENT_MOUSE_MOTION)
	{
		event.u.mouse.x = read32 (input.x);
		event.u.mouse.y = read32 (input.y);
		event.u.mouse.button = read32 (input.button);
	}
	else if (event.type == EVENT_KEY_DOWN ||
		 event.type == EVENT_KEY_UP)
	{
		event.u.key.keyCode = read32 (input.key_code);
		event.u.key.keyChar = read32 (input.key_char);
	}
	else if (event.type == EVENT_ACTIVE)
	{
		event.u.active.active = !!read32 (input.active);
	}
	else if (event.type == EVENT_QUIT)
	{
		//nothing to do.
	}
}

void UdpInputInterface::UpdateStatus()
{
	const unsigned int MaxIdleTime = 15000;
	bool statusChanged = false;
	DWORD count = Sexy::GetTickCount();
	if (mHasPointer && (count - mPointerCount) > MaxIdleTime)
	{
		statusChanged = true;
		mHasPointer = false;
	}

	if (mHasKey && (count - mKeyCount) > MaxIdleTime)
	{
		statusChanged = true;
		mHasKey = false;
	}
	if (statusChanged)
		mManager->Changed();
}

void UdpInputInterface::UpdateStatus(const Event &event)
{
	bool statusChanged = false;
	DWORD count = Sexy::GetTickCount();

	if (event.type == EVENT_MOUSE_BUTTON_PRESS ||
	    event.type == EVENT_MOUSE_BUTTON_RELEASE ||
	    event.type == EVENT_MOUSE_WHEEL_UP ||
	    event.type == EVENT_MOUSE_WHEEL_DOWN ||
	    event.type == EVENT_MOUSE_MOTION)
	{
		if (!mHasPointer)
		{
			statusChanged = true;
			mHasPointer = true;
		}
		mPointerCount = count;
	}
	else if (event.type == EVENT_KEY_DOWN ||
		 event.type == EVENT_KEY_UP)
	{
		if (!mHasKey)
		{
			statusChanged = true;
			mHasKey = true;
		}
		mKeyCount = count;
	}
	if (statusChanged)
		mManager->Changed();
}

void UdpInputInterface::Run (void * data)
{
	UdpInputInterface * driver = (UdpInputInterface *)data;
	struct UdpInput    input[64];
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
				driver->mManager->Changed();
				continue;
			}
			if (status < 0)
				continue;
			if (driver->mDone)
				break;
			if (!FD_ISSET (fd, &set))
			{
				driver->UpdateStatus();
				continue;
			}
		}
		else
		{
			usleep (100);
			printf ("try to reopen the input device.\n");
			driver->OpenDevice ();
			continue;
		}

		ssize_t readlen;
		readlen = recv (fd, input, sizeof (struct UdpInput), 0);

		if (driver->mDone)
			break;

		if (readlen < 0 && errno != EINTR)
		{
			printf ("device disconnected.\n");
			driver->CloseDevice ();
			continue;
		}

		if (readlen <= 0)
		{
			driver->UpdateStatus();
			continue;
		}

		readlen /= sizeof (input[0]);
		for (int i = 0; i < readlen; i++) {
			Event event;

			event.type = EVENT_NONE;
			event.flags = 0;
			event.u.mouse.x = driver->mX;
			event.u.mouse.y = driver->mY;

			handle_event (input[i], event);

			if (event.type == EVENT_MOUSE_MOTION)
			{
				if (event.flags & EVENT_FLAGS_AXIS)
				{
					driver->mX = event.u.mouse.x;
					driver->mY = event.u.mouse.y;
				}
			}
			if (event.type != EVENT_NONE)
			{
				if (0)
				{
					printf ("event.type: %d\n", event.type);
					printf ("event.x: %d\n", event.u.mouse.x);
					printf ("event.y: %d\n", event.u.mouse.y);
				}

				driver->UpdateStatus(event);
				driver->PostEvent (event);
			}
		}
	}
}

bool UdpInputInterface::HasEvent ()
{
	return false;
}

bool UdpInputInterface::GetEvent (Event & event)
{
	return false;
}

bool UdpInputInterface::GetInfo(InputInfo &theInfo, int subid)
{
	if (!mHasKey && !mHasPointer)
		return false;

	theInfo.mName = "UdpInput";
	theInfo.mHasKey = mHasKey;
	theInfo.mHasPointer = mHasPointer;
	return true;
}

class UdpInputDriver: public InputDriver {
public:
	UdpInputDriver ()
	 : InputDriver("UdpInput", 0)
	{
	}

	InputInterface* Create (SexyAppBase * theApp)
	{
		return new UdpInputInterface (theApp->mInputManager);
        }
};

static UdpInputDriver aUdpInputDriver;
InputDriverRegistor aUdpInputDriverRegistor (&aUdpInputDriver);
InputDriver* GetUdpInputDriver()
{
	return &aUdpInputDriver;
}
