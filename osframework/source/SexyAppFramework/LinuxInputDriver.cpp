#include "LinuxInputDriver.h"
#include "InputDriverFactory.h"
#include "InputManager.h"
#include "AutoCrit.h"
#include "SexyAppBase.h"
#include "KeyCodes.h"

#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <limits.h>
#include <ctype.h>

#include <linux/input.h>
#include <linux/keyboard.h>

#ifndef SEXY_LINUX_DEVICE_NO_HOTPLUG
//#define SEXY_LINUX_DEVICE_NO_HOTPLUG
#endif

#ifndef EVIOCGLED
#define EVIOCGLED(len) _IOC(_IOC_READ, 'E', 0x19, len)
#endif

#ifndef EVIOCGRAB
#define EVIOCGRAB      _IOW('E', 0x90, int)
#endif

#define BITS_PER_LONG	     (sizeof(long) * 8)
#define NBITS(x)	     ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)		     ((x) % BITS_PER_LONG)
#define BIT(x)		     (1UL << OFF(x))
#define LONG(x)		     ((x) / BITS_PER_LONG)
#undef test_bit
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

/* compat defines for older kernel like 2.4.x */
#ifndef EV_SYN
#define EV_SYN			0x00
#define SYN_REPORT		0
#define SYN_CONFIG		1
#define ABS_TOOL_WIDTH		0x1c
#define BTN_TOOL_DOUBLETAP	0x14d
#define BTN_TOOL_TRIPLETAP	0x14e
#endif

#define MODIFIERS_SHIFT	   (1 << 0)
#define MODIFIERS_CAPSLOCK (1 << 1)

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

#define MAX_ABS 6
struct input_info {
	int num_keys;
	int num_ext_keys;
	int num_buttons;
	int num_rels;
	int num_abs;

	struct input_id id;
	struct input_absinfo absinfo[MAX_ABS];
};

static bool
get_device_info (int fd, struct input_info* info, bool silent = true);

using namespace Sexy;

namespace Sexy {
class LinuxInputDriver: public InputDriver {
public:
	LinuxInputDriver ()
	 : InputDriver("LinuxInput", 0)
	{
		mHotplugDone = true;
		mStopped = false;
	}

	const std::string& GetDeviceFilePath()
	{
		return mDevicePattern;
	}

	void SetupDeviceFilePath()
	{
		if (mDevicePattern.length())
			return;

		mDevicePattern = "/dev/input/event";
		if (!access ("/dev/input", R_OK | W_OK | X_OK))
			return;

		if (mkdir ("/tmp/popcap", 0755) < 0 && errno != EEXIST)
			return;
		if (mkdir ("/tmp/popcap/input", 0755) < 0 && errno != EEXIST)
			return;

		for (unsigned int i = 0; i < 255; i++)
		{
			char path[2048];

			snprintf (path, sizeof(path),
				  "/tmp/popcap/input/event%d", i);
			if (mknod (path, S_IFCHR | 0644,
				   makedev(13, 64 + i)) < 0 && errno != EEXIST)
				return;
		}

		mDevicePattern = "/tmp/popcap/input/event";
	}

	unsigned int CheckHotplug ()
	{
		unsigned int checksum;
		FILE *fp;

		checksum = 0;
		fp = fopen ("/proc/bus/input/devices", "rb");
		if (!fp)
			return 0;

		unsigned char byte;
		while (fread (&byte, 1, 1, fp) > 0)
			checksum += byte;
		fclose (fp);

		return checksum;
	}

	void HotplugLoop ()
	{
		unsigned int checksum = (unsigned int)-1;

		while (!mHotplugDone)
		{
			unsigned int current;

			CleanupDisconDevices ();
			current = CheckHotplug ();
			if (current != checksum)
			{
				printf ("Rescanning input devices...\n");
				AutoCrit aAutoCrit (mScanCritSect);

				checksum = current;
				ScanAndAddDevice (mApp, true);

				fflush(stdout);
			}

			sleep (1);
		}
	}

	static void HotplugLoopStub (void *arg)
	{
		LinuxInputDriver *driver = (LinuxInputDriver*)arg;
		driver->HotplugLoop ();
	}

	void OnStart (SexyAppBase * theApp, InputManager * theManager)
	{
		mHotplugDone = false;
		mApp = theApp;
		mDisconDevices.clear ();
		mStopped = false;
		SetupDeviceFilePath();
#ifndef SEXY_LINUX_DEVICE_NO_HOTPLUG
		mHotplugThread = Thread::Create (HotplugLoopStub, this);
#endif
	}

	void OnStop ()
	{
		mHotplugDone = true;
#ifndef SEXY_LINUX_DEVICE_NO_HOTPLUG
		mHotplugThread.Join ();
#endif
		mStopped = true;
	}

	void CleanupDisconDevices ()
	{
		if (mDisconDevices.empty ())
			return;

		while (!mDisconDevices.empty ())
		{
			LinuxInputInterface* device = mDisconDevices.front ();

			printf ("Cleanuping device: %s\n",
				device->GetDeviceName ().c_str ());
			mApp->mInputManager->Remove (device);
			mDisconDevices.pop_front ();
		}
	}

	InputInterface* Create (SexyAppBase * theApp)
	{
		SetupDeviceFilePath();

#ifdef SEXY_LINUX_DEVICE_NO_HOTPLUG
		const char * device = getenv ("SEXY_LINUX_INPUT_DEVICE");
		if (device)
			return new LinuxInputInterface (theApp->mInputManager, this, device);

		ScanAndAddDevice(theApp, false);
#endif
		return 0;
	}

	void ScanAndAddDevice (SexyAppBase * theApp, bool hotpluged)
	{
		for (unsigned i = 0; i < 256; i++)
		{
			struct input_info info;
			int fd;
			char name[1024];

			snprintf (name, sizeof(name), "%s%d",
				  GetDeviceFilePath().c_str(), i);
			fd = open (name, O_RDWR);
			if (fd < 0)
				continue;

			if (FindDevice (name))
				continue;

			if (!get_device_info (fd, &info, true))
			{
				close (fd);
				continue;
			}
			close (fd);

			LinuxInputInterface * anInput;
			anInput = new LinuxInputInterface (theApp->mInputManager, this,
							   name, hotpluged);
			if (!theApp->mInputManager->Add(anInput, this, hotpluged))
				delete anInput;
		}
	}

	std::string GetRealDevice (std::string device)
	{
		char buf[PATH_MAX];
		char* path = realpath (device.c_str (), buf);
		if (!path)
			return device;

		std::string result (path);
		return result;
	}

	std::string AddDevice (std::string device, LinuxDeviceInfo info)
	{
		device = GetRealDevice (device);
		if (!device.length ())
			return device;

		AutoCrit aAutoCrit (mCritSect);

		mDeviceMap.insert (std::pair<std::string, LinuxDeviceInfo>(device, info));

		printf ("Added device: %s.\n", device.c_str ());
		return device;
	}

	void RemoveDevice (std::string device)
	{
		AutoCrit aAutoCrit (mCritSect);

		DeviceMap::iterator it = mDeviceMap.find (device);
		if (it != mDeviceMap.end())
			mDeviceMap.erase (it);

		printf ("Removed device: %s.\n", device.c_str ());
	}

	bool FindDevice (std::string device)
	{
		AutoCrit aAutoCrit (mCritSect);

		DeviceMap::iterator it = mDeviceMap.find (device);
		return it != mDeviceMap.end();
	}

	void AddDisconDevice (LinuxInputInterface *theDevice)
	{
		AutoCrit aAutoCrit (mScanCritSect);

		if (mHotplugDone)
			return;

		if (std::find (mDisconDevices.begin (),
			       mDisconDevices.end (),
			       theDevice) != mDisconDevices.end ())
			return;
		mDisconDevices.push_back (theDevice);
	}

	int DoReprobe (LinuxDeviceInfo *dinfo,
		       std::string &theDeviceName)
	{
		for (unsigned i = 0; i < 256; i++)
		{
			struct input_info info;
			int fd;
			char name[1024];

			snprintf (name, sizeof(name), "%s%d",
				  GetDeviceFilePath().c_str(), i);

			if (FindDevice (std::string (name)))
				continue;

			fd = open (name, O_RDWR);
			if (fd < 0)
				continue;

			if (!get_device_info (fd, &info, true))
			{
				close (fd);
				continue;
			}

			if (!dinfo || (dinfo->pid == info.id.product &&
				       dinfo->vid == info.id.vendor &&
				       dinfo->version == info.id.version &&
				       dinfo->bustype == info.id.bustype))
			{
				theDeviceName = std::string (name);
				return fd;
			}

			close (fd);
		}

		return -1;
	}

	int Reprobe (LinuxDeviceInfo *dinfo,
		     std::string &theDeviceName)
	{
		AutoCrit aAutoCrit (mScanCritSect);
		int fd;

		if (mStopped)
			return -1;

		fd = DoReprobe (dinfo, theDeviceName);
		if (fd >= 0)
			return fd;

		if (!dinfo)
			return -1;

		return DoReprobe (NULL, theDeviceName);
	}

private:
	typedef std::map<std::string, LinuxDeviceInfo> DeviceMap;
	DeviceMap	      mDeviceMap;

	CritSect	      mCritSect;
	CritSect	      mScanCritSect;

	std::list<LinuxInputInterface*> mDisconDevices;

	Thread		      mHotplugThread;
	bool		      mHotplugDone;
	bool		      mStopped;

	SexyAppBase*	      mApp;

	std::string           mDevicePattern;
};

}

LinuxInputInterface::LinuxInputInterface (InputManager* theManager,
					  LinuxInputDriver *driver,
					  const char * theName,
					  bool hotpluged)
	: InputInterface (theManager), mFd (-1), mDone (false)
{
	mX = 0;
	mY = 0;
	mMinX = 0;
	mMaxX = 0;
	mMinY = 0;
	mMaxY = 0;
	mHasPointer = false;
	mHasKey = false;
	mInitialized = false;
	mHotpluged = hotpluged;
	mDriver = driver;
	mDeviceName = std::string(theName ? theName : "");
	memset (&mInfo, 0, sizeof (mInfo));
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

	int ret;
	ret = pthread_create (&mThread, NULL, LinuxInputInterface::Run, this);
	if (ret)
		goto close_device;
	mInitialized = true;
	return true;
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
	if (mInitialized)
		pthread_join (mThread, NULL);
	CloseDevice ();

	mInitialized = false;
	mX = 0;
	mY = 0;
	mMinX = 0;
	mMaxX = 0;
	mMinY = 0;
	mMaxY = 0;
	mHasPointer = false;
	mHasKey = false;
}

static bool
get_device_info (int fd, struct input_info* info, bool silent)
{
	unsigned int  num_keys	   = 0;
	unsigned int  num_ext_keys = 0;
	unsigned int  num_buttons  = 0;
	unsigned int  num_rels	   = 0;
	unsigned int  num_abs	   = 0;
	unsigned int i;
	int ret;

	char name[256];
	unsigned long eventbit[NBITS(EV_MAX)];
	unsigned long keybit[NBITS(KEY_MAX)];
	unsigned long relbit[NBITS(REL_MAX)];
	unsigned long absbit[NBITS(ABS_MAX)];

	/* get device name */
	ret = ioctl (fd, EVIOCGNAME(255), name);
	if (ret < 0)
		return false;

	if (!silent)
		printf ("** Device name: %s\n", name);

	/* get event type bits */
	ret = ioctl (fd, EVIOCGBIT(0, sizeof(eventbit)), eventbit);
	if (ret < 0)
		return false;

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
		ret = ioctl (fd, EVIOCGBIT (EV_REL, sizeof(relbit)), relbit);
		if (ret < 0)
			return false;

		for (i = 0; i < REL_MAX; i++)
			if (test_bit (i, relbit))
				num_rels++;
	}

	if (test_bit (EV_ABS, eventbit))
	{
		/* get bits for absolute axes */
		ret = ioctl (fd, EVIOCGBIT (EV_ABS, sizeof (absbit)), absbit);
		if (ret < 0)
			return false;

		for (i = 0; i < ABS_PRESSURE; i++)
			if (test_bit (i, absbit))
				num_abs++;
	}

	if (ioctl (fd, EVIOCGID, &info->id) < 0)
	{
		perror ("Failed to get device id");
		return false;
	}

	for (i = 0; i < num_abs; i++)
	{
		if (ioctl (fd, EVIOCGABS(i), &info->absinfo[i]) < 0)
		{
			perror ("Failed to get absinfo");
			return false;
		}
	}

	if (!silent)
	{
		printf ("** keys: %d\n", num_keys);
		printf ("** extend keys: %d\n", num_ext_keys);
		printf ("** buttons: %d\n", num_buttons);
		printf ("** relative events: %d\n", num_rels);
		printf ("** absolute events: %d\n", num_abs);
		for (i = 0; i < num_abs; i++)
		{
			printf ("** \taxes %d: value %d min %d max %d flatness %d fuzz %d\n",
				i,
				info->absinfo[i].value,
				info->absinfo[i].minimum,
				info->absinfo[i].maximum,
				info->absinfo[i].fuzz,
				info->absinfo[i].flat);
		}
	}

	if (info)
	{
		info->num_keys = num_keys;
		info->num_ext_keys = num_ext_keys;
		info->num_buttons = num_buttons;
		info->num_rels = num_rels;
		info->num_abs = num_abs;
	}


	return true;
}

bool LinuxInputInterface::OpenDevice ()
{
	const char * device = 0;

	if (mDeviceName.length ())
		device = mDeviceName.c_str();
	if (!device)
		device = getenv ("SEXY_LINUX_INPUT_DEVICE");
	if (!device)
		device = "/dev/input/event0";
	mFd = open (device, O_RDWR);
	if (mFd < 0) {
#if defined(SEXY_DEBUG) || defined(DEBUG)
		printf ("Failed to open '%s'.\n", device);
#endif
		goto open_failed;
	}

	int ret;

	mGrabed = false;
	bool wantGrab;
#ifdef SEXY_LINUX_INPUT_GRAB_DEVICE
	wantGrab = getenv ("SEXY_LINUX_INPUT_NO_GRAB") == 0;
#else
	wantGrab = getenv ("SEXY_LINUX_INPUT_GRAB_DEVICE") != 0;
#endif
	if (wantGrab)
	{
		ret = ioctl (mFd, EVIOCGRAB, 1);
		if (ret)
			printf ("Couldn't grab device: %s.\n", device);
		else
			mGrabed = true;
	}
	if (mGrabed)
		printf ("Graded device: %s.\n", device);
	mModifiers = 0;

	struct input_info info;
	get_device_info (mFd, &info, false);
	mInfo.pid = info.id.product;
	mInfo.vid = info.id.vendor;
	mInfo.version = info.id.version;
	mInfo.bustype = info.id.bustype;

	mHasPointer = info.num_rels || info.num_abs;
	mHasKey = info.num_keys || info.num_ext_keys;

	mDeviceName = std::string (device);
	mDriver->AddDevice (mDeviceName, mInfo);

	return true;
 open_failed:
	return false;
}

bool LinuxInputInterface::ReopenDevice ()
{
	int ret;

	CloseDevice ();

	ret = mDriver->Reprobe (&mInfo, mDeviceName);
	if (ret < 0)
		return false;

	mFd = ret;
	if (mGrabed)
	{
		ret = ioctl (mFd, EVIOCGRAB, 1);
		if (ret)
		{
			printf ("Couldn't grab %s.\n", mDeviceName.c_str ());
			mGrabed = false;
		}
		else
		{
			mGrabed = true;
		}
	}
	mModifiers = 0;

	struct input_info info;
	get_device_info (mFd, &info, false);
	mInfo.pid = info.id.product;
	mInfo.vid = info.id.vendor;
	mInfo.version = info.id.version;
	mInfo.bustype = info.id.bustype;

	mDriver->AddDevice (mDeviceName, mInfo);

	printf ("Device %s reopened.\n", mDeviceName.c_str ());
	return true;
}

void LinuxInputInterface::CloseDevice ()
{
	if (mFd < 0)
		return;

	if (mGrabed)
		ioctl (mFd, EVIOCGRAB, 0);
	mGrabed = false;
	close (mFd);
	mFd = -1;

	mDriver->RemoveDevice (mDeviceName);
}

static int translate_key(int keysym)
{
	static const struct {
		int    keySym;
		int    keyCode;
	} keymap[] = {
		{ KEY_LEFT, KEYCODE_LEFT },
		{ KEY_RIGHT, KEYCODE_RIGHT },
		{ KEY_UP, KEYCODE_UP },
		{ KEY_DOWN, KEYCODE_DOWN },
		{ KEY_ENTER, KEYCODE_RETURN },
		{ KEY_ESC, KEYCODE_ESCAPE },
		{ KEY_BACKSPACE, KEYCODE_BACK },
		{ KEY_DELETE, KEYCODE_DELETE },
		{ KEY_LEFTSHIFT, KEYCODE_SHIFT },
		{ KEY_RIGHTSHIFT, KEYCODE_SHIFT },
		{ KEY_LEFTCTRL, KEYCODE_CONTROL },
		{ KEY_RIGHTCTRL, KEYCODE_CONTROL },
		{ KEY_CAPSLOCK, KEYCODE_CAPITAL },
		{ KEY_SPACE, KEYCODE_SPACE },
		{ KEY_A, 'A' },
		{ KEY_B, 'B' },
		{ KEY_C, 'C' },
		{ KEY_D, 'D' },
		{ KEY_E, 'E' },
		{ KEY_F, 'F' },
		{ KEY_G, 'G' },
		{ KEY_H, 'H' },
		{ KEY_I, 'I' },
		{ KEY_J, 'J' },
		{ KEY_K, 'K' },
		{ KEY_L, 'L' },
		{ KEY_M, 'M' },
		{ KEY_N, 'N' },
		{ KEY_O, 'O' },
		{ KEY_P, 'P' },
		{ KEY_Q, 'Q' },
		{ KEY_R, 'R' },
		{ KEY_S, 'S' },
		{ KEY_T, 'T' },
		{ KEY_U, 'U' },
		{ KEY_V, 'V' },
		{ KEY_W, 'W' },
		{ KEY_X, 'X' },
		{ KEY_Y, 'Y' },
		{ KEY_Z, 'Z' },
		{ 0, 0 }
	};

	for (int i = 0; keymap[i].keyCode; i++) {
		if (keymap[i].keySym == keysym)
			return keymap[i].keyCode;
	}

	if (keysym >= KEY_1 && keysym <= KEY_9)
		return '1' + keysym - KEY_1;
	if (keysym == KEY_0)
		return '0';
	return 0;
}

static bool
handle_key_event (struct input_event & linux_event,
		  int &modifiers, Event & event)
{
     if (linux_event.code == BTN_TOUCH || linux_event.code == BTN_TOOL_FINGER)
	  linux_event.code = BTN_MOUSE;

	if ((linux_event.code >= BTN_MOUSE && linux_event.code < BTN_JOYSTICK) ||
	    linux_event.code == BTN_TOUCH)
	{
#if 0
		printf ("button: %d(%x) %d\n", linux_event.code, linux_event.code,
			linux_event.value);
#endif
		if (linux_event.code == BTN_MOUSE)
			event.u.mouse.button = 1;
		else if (linux_event.code == (BTN_MOUSE + 1))
			event.u.mouse.button = 2;
		else if (linux_event.code == (BTN_MOUSE + 2))
			event.u.mouse.button = 3;
		else
			return true;
		if (linux_event.value == 1)
			event.type = EVENT_MOUSE_BUTTON_PRESS;
		else if (linux_event.value == 0)
			event.type = EVENT_MOUSE_BUTTON_RELEASE;
		event.flags |= EVENT_FLAGS_BUTTON;
	}
	else
	{
		int keycode = translate_key(linux_event.code);

		if (!keycode)
			return false;


		if (linux_event.value == 0)
			event.type = EVENT_KEY_UP;
		else if (linux_event.value == 1)
			event.type = EVENT_KEY_DOWN;
		else if (linux_event.value == 2)
			event.type = EVENT_KEY_DOWN;
		else
			return false;

		event.flags |= EVENT_FLAGS_KEY_CODE;
		event.u.key.keyCode = keycode;

		printf ("keycode: %s\n", GetKeyNameFromCode((KeyCode)keycode).c_str());

		if (isalnum(keycode) && event.type == EVENT_KEY_DOWN)
		{
			event.flags |= EVENT_FLAGS_KEY_CHAR;
			if (keycode >= 'A' && keycode <= 'Z')
				keycode = keycode - 'A' + 'a';
			if (((modifiers & MODIFIERS_SHIFT) != 0) ^
			    ((modifiers & MODIFIERS_CAPSLOCK)))
			{
				if (keycode >= 'a' && keycode <= 'z')
				{
					event.u.key.keyChar = keycode - 'a' + 'A';
				}
				else if (keycode >= '0' && keycode <= '9')
				{
					static int numbermap[10] =
					{
						'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'
					};

					event.u.key.keyChar = numbermap[keycode - '0'];
				}
				else
				{
					event.u.key.keyChar = keycode;
				}
			}
			else
			{
				event.u.key.keyChar = keycode;
			}
		}
		if (event.type == EVENT_KEY_DOWN)
		{
			if (event.u.key.keyCode == KEYCODE_SHIFT)
				modifiers |= MODIFIERS_SHIFT;
			if (event.u.key.keyCode == KEYCODE_CAPITAL)
				modifiers |= MODIFIERS_CAPSLOCK;
		}
		else if (event.type == EVENT_KEY_UP)
		{
			if (event.u.key.keyCode == KEYCODE_SHIFT)
				modifiers &= ~MODIFIERS_SHIFT;
			if (event.u.key.keyCode == KEYCODE_CAPITAL)
				modifiers &= ~MODIFIERS_CAPSLOCK;
		}
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
		event.flags |= EVENT_FLAGS_REL_AXIS;
		event.u.mouse.x = linux_event.value;
		break;
	case REL_Y:
		event.type = EVENT_MOUSE_MOTION;
		event.flags |= EVENT_FLAGS_REL_AXIS;
		event.u.mouse.y = linux_event.value;
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
		event.flags |= EVENT_FLAGS_AXIS;
		event.u.mouse.x = linux_event.value;
		break;
	case ABS_Y:
		event.type = EVENT_MOUSE_MOTION;
		event.flags |= EVENT_FLAGS_AXIS;
		event.u.mouse.y = linux_event.value;
		break;
	default:
		break;
	}

	return true;
}

static bool handle_event (struct input_event& linux_event,
			  int &modifiers, Event &event)
{
	switch (linux_event.type) {
	case EV_KEY:
		return handle_key_event (linux_event, modifiers, event);
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
	fd_set		   set;
	int		   status;
	int		   fd = driver->mFd;

	while (!driver->mDone)
	{
		fd = driver->mFd;
		if (driver->mFd >= 0)
		{
			FD_ZERO (&set);
			FD_SET (fd, &set);

			struct timeval timeout;

			timeout.tv_sec = 0;
			timeout.tv_usec = 300;
			status = select (fd + 1, &set, NULL, NULL, &timeout);
			if (status < 0 && errno != EINTR)
			{
				printf ("Device disconnected(hotpluged ? %s).\n",
					driver->mHotpluged ? "yes" : "no");

				driver->CloseDevice ();

				/* don't bother, it's hotpluged */
				if (driver->mHotpluged)
					goto disconnected;
				else
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
			usleep (500000);
			if (!driver->ReopenDevice ())
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
			printf ("Device disconnected (hotpluged? %s).\n",
				driver->mHotpluged ? "yes" : "no");
			driver->CloseDevice ();
			/* don't bother, it's hotpluged */
			if (driver->mHotpluged)
				goto disconnected;
			else
				continue;
		}

		if (readlen <= 0)
			continue;

		readlen /= sizeof (linux_event[0]);

		Event event;
		memset (&event, 0, sizeof (event));
		for (int i = 0; i < readlen; i++) {
			if (0)
				printf ("linux_event: type %d code %d value: %d\n",
					linux_event[i].type, linux_event[i].code,
					linux_event[i].value);

			handle_event (linux_event[i], driver->mModifiers, event);

#if 0
			if (linux_event[i].type != EV_SYN ||
			    linux_event[i].code != SYN_REPORT)
			    continue;
#endif

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
#if 1
				if (0)
#else
				if (event.type == EVENT_MOUSE_BUTTON_PRESS ||
				    event.type == EVENT_MOUSE_BUTTON_RELEASE)
#endif
				{
					printf ("event.type: %d\n", event.type);
					printf ("event.button: %d\n", event.u.mouse.button);
					printf ("event.x: %d\n", event.u.mouse.x);
					printf ("event.y: %d\n", event.u.mouse.y);
				}
				driver->PostEvent (event);
				memset (&event, 0, sizeof (event));
			}
		}
		fflush(stdout);
	}

	return NULL;
  disconnected:
	driver->mDriver->AddDisconDevice (driver);
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

bool LinuxInputInterface::GetInfo (InputInfo &theInfo, int subid)
{
	if (mFd < 0)
		return false;

	theInfo.mName = mDeviceName;
	theInfo.mHasPointer = mHasPointer;
	theInfo.mHasKey = mHasKey;
	return true;
}

static LinuxInputDriver aLinuxInputDriver;
InputDriverRegistor aLinuxInputDriverRegistor (&aLinuxInputDriver);
InputDriver* GetLinuxInputDriver()
{
	return &aLinuxInputDriver;
}
