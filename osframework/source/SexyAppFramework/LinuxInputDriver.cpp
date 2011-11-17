#include "LinuxInputDriver.h"
#include "InputDriverFactory.h"
#include "InputManager.h"
#include "AutoCrit.h"
#include "SexyAppBase.h"
#include "KeyCodes.h"
#include "SexyLog.h"

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

#include <map>
#include <string>
#include <vector>
#include <algorithm>

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

struct input_absinfo_v1 {
	__s32 value;
	__s32 minimum;
	__s32 maximum;
	__s32 fuzz;
	__s32 flat;
};

struct input_absinfo_v2 {
	__s32 value;
	__s32 minimum;
	__s32 maximum;
	__s32 fuzz;
	__s32 flat;
	__s32 resolution;
};

#define EVIOCGABSV1(abs)		_IOR('E', 0x40 + abs, struct input_absinfo_v1)	/* get abs value/limits */
#define EVIOCGABSV2(abs)		_IOR('E', 0x40 + abs, struct input_absinfo_v2)	/* get abs value/limits */

#ifndef ABS_MAX
#define ABS_MAX ABS_MISC
#endif
struct input_info {
	char name[255];
	int num_keys;
	int num_ext_keys;
	int num_buttons;
	int num_joysticks;
	int num_rels;
	int num_abs;

	struct input_id id;
	struct input_absinfo_v2 absinfo[ABS_MAX];
	int axis[ABS_MAX];
};

static bool
get_device_info (int fd, struct input_info* info, bool silent = true);

using namespace Sexy;

namespace Sexy {

class Value {
public:
	enum Type {
		UNKNOWN = 0,
		STRING  = 1,
		INT     = 2,
		FLOAT   = 3
	};

	Value()
		: mType(UNKNOWN), mInt(0), mFloat(0) {
	}

	Value(const Value &v)
		: mType(v.mType), mStr(v.mStr), mInt(v.mInt), mFloat(v.mFloat) {
	}

	Value(const std::string& s, Type type = UNKNOWN)
		: mType(type), mStr(s), mInt(0), mFloat(0) {
	}

	Value(const std::string& s, int v)
		: mType(INT), mStr(s), mInt(v), mFloat(v) {
	}

	Value(const std::string& s, float v)
		: mType(FLOAT), mStr(s), mInt(v), mFloat(v) {
	}

	Value(int v)
		: mType(INT), mInt(v), mFloat(v) {
		mStr = StrFormat("%d", v);
	}

	Value(float v)
		: mType(FLOAT), mInt(v), mFloat(v) {
		mStr = StrFormat("%f", v);
	}

	std::string getString() const {
		return mStr;
	}

	int getInteger () const {
		update(INT);
		return mInt;
	}

	int getFloat () const {
		update(FLOAT);
		return mFloat;
	}

	Type getType() const {
		return mType;
	}

private:
	void update(Type type) const {
		if (type == INT) {
			if (mType >= type)
				return;
			if (mStr.substr(0, 2) == "0x")
				mInt = strtol(mStr.c_str(), 0, 16);
			else
				mInt = atoi(mStr.c_str());
			mFloat = mInt;
			mType = type;
		} else if (type == FLOAT) {
			if (mType >= type)
				return;
			if (mStr.substr(0, 2) == "0x")
				mFloat = (float)strtol(mStr.c_str(), 0, 16);
			else
				mFloat = atof(mStr.c_str());
			mInt = mFloat;
			mType = type;
		}
	}

	void update() {
		if (mType != UNKNOWN || mType == STRING)
			return;

		if (mType == INT) {
			if (mStr.substr(0, 2) == "0x")
				mInt = strtol(mStr.c_str(), 0, 16);
			else
				mInt = atoi(mStr.c_str());
		} else if (mType == FLOAT) {
			if (mStr.substr(0, 2) == "0x")
				mFloat = (float)strtol(mStr.c_str(), 0, 16);
			else
				mFloat = atof(mStr.c_str());
			mInt = mFloat;
		}
	}

private:
	mutable Type mType;
	std::string mStr;
	mutable int mInt;
	mutable float mFloat;
};
typedef std::map<std::string, Value> KeyValue;

class Rule {
public:
	enum Type {
		ACCEPT = 0,
		REJECT = 1
	};

	enum Operator {
		EQUAL,
		NEQUAL
	};

	Rule() : mOperator(EQUAL) {
	}

	void setOperator(Operator op) {
		mOperator = op;
	}

	void setLeftOperand(const Value& operand) {
		mOperand[0] = operand;
	}

	void setRightOperand(const Value& operand) {
		mOperand[1] = operand;
	}

	bool match(const KeyValue& kv) {
		const std::string key = mOperand[0].getString();
		KeyValue::const_iterator it = kv.find(key);
		if (it == kv.end())
			return false;

		const Value& lv = it->second;
		const Value& rv = mOperand[1];
		Value::Type lt = lv.getType();
		Value::Type rt = rv.getType();
		Value::Type pt = std::max(lt, rt);

		if (pt == Value::FLOAT) {
			float lf = lv.getFloat();
			float rf = rv.getFloat();

			if (mOperator == EQUAL)
				return lf == rf;
			return lf != rf;
		} else if (pt == Value::INT) {
			int li = lv.getInteger();
			int ri = rv.getInteger();

			if (mOperator == EQUAL)
				return li == ri;
			return li != ri;
		} else {
			std::string ls = lv.getString();
			std::string rs = rv.getString();

			if (mOperator == EQUAL)
				return ls == rs;
			return ls != rs;
		}
		return false;
	}

private:
	Operator mOperator;
	Value mOperand[2];
};
typedef std::vector<Rule> RuleVector;

class Match {
public:
	enum Operator {
		AND = 0,
		OR  = 1
	};

	Match() {
	}

	Match(const Match &o) : mRules(o.mRules) {
	}

	Match(const char* str) {
		if (str)
			parseRules(std::string(str));
	}

	Match(const std::string& str) {
		parseRules(str);
	}

	void resetRules() {
		mRules.clear();
		mOperators.clear();
	}

	static void trim(std::string& str) {
		static const char chars[] = " \t\r\n";

		str.erase(0, str.find_first_not_of(chars));
		str.resize(str.find_last_not_of(chars) + 1);
	}

	bool parseRule(std::string& str) {
		if (str.empty())
			return false;

		Rule::Operator rOp = Rule::EQUAL;

		std::string rulestr = str;
		size_t oppos = 0;
		oppos = rulestr.find("!=");
		if (oppos == std::string::npos)
			oppos = rulestr.find("=");
		else
			rOp = Rule::NEQUAL;

		// bad operator
		if (oppos == std::string::npos)
			return false;

		Rule rule;
		rule.setOperator(rOp);
		std::string lop = rulestr.substr(0, oppos);
		std::string rop = rulestr.substr(oppos + 1 + rOp);

		trim(lop);
		trim(rop);
		if (lop.empty())
			return false;
		rule.setLeftOperand(Value(lop));
		rule.setRightOperand(Value(rop));
		mRules.push_back(rule);
		return true;
	}

	bool parseRules(const std::string& str) {
		if (str.empty())
			return true;

		size_t start = 0;
		size_t end = str.find_first_of(",;", start);

		while (start < end) {
			size_t len = end == std::string::npos ? end : end - start;
			std::string cur = str.substr(start, len);

			trim(cur);
			if (!parseRule(cur))
				return false;

			if (end != std::string::npos) {
				if (str[end] == ',')
					mOperators.push_back(AND);
				else
					mOperators.push_back(OR);
			} else {
				mOperators.push_back(OR);
			}

			if (end == std::string::npos)
				break;
			if (end + 1 >= str.length())
				break;

			start = end + 1;
			end = str.find_first_of(",;", start);

			// empty rule
			if (start == end)
				return false;
		}
		return true;
	}

	bool parseRules(const char* str) {
		if (!str)
			return true;
		return parseRules(std::string(str));
	}

	bool hasRules() {
		return mRules.size() != 0;
	}

	bool matchRules(const KeyValue &keyvalue) {
		for (size_t i = 0; i < mRules.size(); i++) {
			bool result = mRules[i].match(keyvalue);
			if (result == false &&
			    (i == mRules.size() - 1 || mOperators[i] == AND))
				return false;
			if (result == true &&
			    (i == mRules.size() - 1 || mOperators[i] == OR))
				return true;
		}

		return true;
	}

private:
	RuleVector mRules;

	typedef std::vector<Operator> OperatorVector;
	OperatorVector mOperators;
};

#ifndef SEXY_LINUX_INPUT
#define SEXY_LINUX_INPUT 1
#endif
class LinuxInputDriver: public InputDriver {
public:
	LinuxInputDriver ()
	 : InputDriver("LinuxInput", 0)
	{
		mHotplugDone = true;
		mStopped = false;
		mHotplug = true;

		if (GetEnvOption("SEXY_NO_LINUX_INPUT", !SEXY_LINUX_INPUT))
			mHotplug = false;

		const char *s = GetEnv("SEXY_LINUX_INPUT_FILTER");
#ifdef SEXY_LINUX_INPUT_FILTER
		if (!s)
			s = SEXY_LINUX_INPUT_FILTER;
#endif
		if (s && !mDeviceFilter.parseRules(s))
		{
			mDeviceFilter.resetRules();
			logfe("LinuxInput: Bad filter rule: %s\n", s);
		}

		s = GetEnv("SEXY_LINUX_INPUT_ENABLE_FILTER");
#ifdef SEXY_LINUX_INPUT_ENABLE_FILTER
		if (!s)
			s = SEXY_LINUX_INPUT_ENABLE_FILTER;
#endif
		if (s && !mDeviceEnableFilter.parseRules(s))
		{
			mDeviceEnableFilter.resetRules();
			logfe("LinuxInput: Bad filter rule: %s\n", s);
		}
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

		for (unsigned int i = 0; i < 64; i++)
		{
			char path[2048];

			snprintf (path, sizeof(path),
				  "/tmp/popcap/input/event%d", i);
			remove(path);
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
				logfi ("Rescanning input devices...\n");
				AutoCrit aAutoCrit (mScanCritSect);

				checksum = current;
                                Sleep(200);
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
		if (mHotplug)
			mHotplugThread = Thread::Create (HotplugLoopStub, this);
	}

	void OnStop ()
	{
		mHotplugDone = true;
		if (mHotplug)
			mHotplugThread.Join ();
		mStopped = true;
	}

	void CleanupDisconDevices ()
	{
		if (mDisconDevices.empty ())
			return;

		while (!mDisconDevices.empty ())
		{
			LinuxInputInterface* device = mDisconDevices.front ();

			logfi ("Cleanuping device: %s\n",
				device->GetDeviceName ().c_str ());
			mApp->mInputManager->Remove (device);
			mDisconDevices.pop_front ();
		}
	}

	InputInterface* Create (SexyAppBase * theApp)
	{
		return 0;
	}

	void ScanAndAddDevice (SexyAppBase * theApp, bool hotpluged)
	{
		for (unsigned i = 0; i < 64; i++)
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

			if (!info.num_abs && !info.num_rels && !info.num_buttons &&
			    !info.num_keys && !info.num_ext_keys && !info.num_joysticks)
				continue;

			KeyValue kv;
			if (mDeviceFilter.hasRules() || mDeviceEnableFilter.hasRules())
			{
				kv["bustype"] = Value(info.id.bustype);
				kv["vendor"] = Value(info.id.vendor);
				kv["product"] = Value(info.id.product);
				kv["version"] = Value(info.id.version);
				kv["name"] = Value(info.name);
				kv["nabs"] = Value(info.num_abs);
				kv["nrel"] = Value(info.num_rels);
				kv["nkey"] = Value(info.num_keys + info.num_ext_keys);
				kv["nbtn"] = Value(info.num_buttons);
			}

			if (mDeviceFilter.hasRules() && !mDeviceFilter.matchRules(kv))
			{
				logfd("LinuxInput: ignoring %s\n", info.name);
				continue;
			}

			// Ignore the compass input device
			if (info.id.bustype == 0 && info.id.vendor == 0 &&
			    info.id.product == 0 && info.id.version == 0 &&
			    !strcmp(info.name, "compass"))
				continue;

#if 0
			if (GetEnvOption("SEXY_LINUX_INPUT_IGNORE_ABS_POINTER", false) &&
			    info.num_abs > 0)
				continue;
#endif

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

		logfi ("Added device: %s.\n", device.c_str ());
		return device;
	}

	void RemoveDevice (std::string device)
	{
		AutoCrit aAutoCrit (mCritSect);

		DeviceMap::iterator it = mDeviceMap.find (device);
		if (it != mDeviceMap.end())
			mDeviceMap.erase (it);

		logfi ("Removed device: %s.\n", device.c_str ());
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
		for (unsigned i = 0; i < 64; i++)
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
	bool                  mHotplug;
	bool		      mStopped;

	SexyAppBase*	      mApp;

	std::string           mDevicePattern;
	Match                 mDeviceFilter;
	Match                 mDeviceEnableFilter;
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
	unsigned int  num_keys	    = 0;
	unsigned int  num_ext_keys  = 0;
	unsigned int  num_buttons   = 0;
	unsigned int  num_rels	    = 0;
	unsigned int  num_abs	    = 0;
	unsigned int  num_joysticks = 0;
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
		logfi ("** Device name: %s\n", name);

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

		for (i = BTN_JOYSTICK; i < BTN_DIGI; i++)
			if (test_bit (i, keybit))
				num_joysticks++;
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

		for (i = 0; i < ABS_MISC; i++)
		{
			if (test_bit (i, absbit))
			{
				info->axis[num_abs] = i;
				num_abs++;
			}
		}
		if (num_abs > ABS_MAX)
			num_abs = ABS_MAX;
	}

	if (ioctl (fd, EVIOCGID, &info->id) < 0)
	{
		logfe ("Failed to get device id");
		return false;
	}

	memset(info->absinfo, 0, sizeof(info->absinfo));
	for (i = 0; i < num_abs; i++)
	{
		if (ioctl (fd, EVIOCGABSV2(info->axis[i]), &info->absinfo[i]) < 0 &&
		    ioctl (fd, EVIOCGABSV1(info->axis[i]), &info->absinfo[i]) < 0)
		{
			logfe ("Failed to get absinfo");
			return false;
		}
	}

	if (!silent)
	{
		logfi ("** keys: %d\n", num_keys);
		logfi ("** extend keys: %d\n", num_ext_keys);
		logfi ("** buttons: %d\n", num_buttons);
		logfi ("** joystick buttons: %d\n", num_joysticks);
		logfi ("** relative events: %d\n", num_rels);
		logfi ("** absolute events: %d\n", num_abs);
		for (i = 0; i < num_abs; i++)
		{
			logfi ("** \taxes %02d:0x%02x: value %d min %d max %d "
			       "flatness %d fuzz %d resolution %d\n",
			       i, info->axis[i],
			       info->absinfo[i].value,
			       info->absinfo[i].minimum,
			       info->absinfo[i].maximum,
			       info->absinfo[i].fuzz,
			       info->absinfo[i].flat,
			       info->absinfo[i].resolution);
		}
	}

	if (info)
	{
		strcpy (info->name, name);
		info->num_keys = num_keys;
		info->num_ext_keys = num_ext_keys;
		info->num_buttons = num_buttons;
		info->num_joysticks = num_joysticks;
		info->num_rels = num_rels;
		info->num_abs = num_abs;
	}


	return true;
}

static bool IsRoutonKeyboard(const char *name)
{
	if (!GetEnvOption("SEXY_LINUX_INPUT_ROUTON_FILTER", false))
		return false;

	return !strcmp(name, "Routon H2        Keyboard");
}

bool LinuxInputInterface::OpenDevice ()
{
	const char * device = 0;

	device = mDeviceName.c_str();
	mFd = open (device, O_RDWR);
	if (mFd < 0) {
#if defined(SEXY_DEBUG) || defined(DEBUG)
		logfe ("Failed to open '%s'.\n", device);
#endif
		goto open_failed;
	}

	int ret;

	mGrabed = false;
	bool wantGrab;
#ifdef SEXY_LINUX_INPUT_GRAB_DEVICE
	wantGrab = true;
#else
	wantGrab = false;
#endif
	wantGrab = GetEnvOption ("SEXY_LINUX_INPUT_GRAB_DEVICE", wantGrab);
	if (wantGrab)
	{
		ret = ioctl (mFd, EVIOCGRAB, 1);
		if (ret)
			logfe ("Couldn't grab device: %s.\n", device);
		else
			mGrabed = true;
	}
	if (mGrabed)
		logfi ("Graded device: %s.\n", device);
	mModifiers = 0;

	// Update device info
	struct input_info info;
	get_device_info (mFd, &info, false);
	mInfo.pid = info.id.product;
	mInfo.vid = info.id.vendor;
	mInfo.version = info.id.version;
	mInfo.bustype = info.id.bustype;

	mHasPointer = info.num_rels && info.num_buttons;
	mHasKey = info.num_keys || info.num_ext_keys;
	mHasJoystick = info.num_abs && info.num_joysticks;
	mDeviceName = std::string (device);
	mRouton = IsRoutonKeyboard(info.name);

	for (int i = 0; i < info.num_abs; i++)
	{
		LinuxAxisInfo axis;

		axis.devFlat = info.absinfo[i].flat;
		axis.devFuzz = info.absinfo[i].fuzz;
		axis.devMinimum = info.absinfo[i].minimum;
		axis.devMaximum = info.absinfo[i].maximum;
		axis.devResolution = info.absinfo[i].resolution;

		float range;
		if (axis.devMaximum != axis.devMinimum)
		{
			range = axis.devMaximum - axis.devMinimum;
		}
		else
		{
			range = 1.0f;
		}
		//rescaled = (value + coef[0]) * coef[1] + coef[2]
		axis.coef[0] = -axis.devMinimum;
		axis.coef[1] = 2.0f / range;
		axis.coef[2] = -1.0f;
		axis.flat = axis.devFlat / range;
		axis.fuzz = axis.devFuzz / range;
		axis.minimum = -1.0f;
		axis.maximum = 1.0f;
		axis.resolution = axis.devResolution / range;
		mAxisInfoMap.insert(AxisInfoMap::value_type(info.axis[i], axis));
	}
	if (info.num_joysticks)
	{
		mButtonMap[BTN_TRIGGER] = KEYCODE_GAMEPAD_A;
		mButtonMap[BTN_THUMB]	= KEYCODE_GAMEPAD_B;
		mButtonMap[BTN_THUMB2]	= KEYCODE_GAMEPAD_C;
		mButtonMap[BTN_TOP]	= KEYCODE_GAMEPAD_X;
		mButtonMap[BTN_TOP2]	= KEYCODE_GAMEPAD_Y;
		mButtonMap[BTN_PINKIE]	= KEYCODE_GAMEPAD_Z;
		mButtonMap[BTN_BASE]	= KEYCODE_GAMEPAD_TL;
		mButtonMap[BTN_BASE2]	= KEYCODE_GAMEPAD_TR;
		mButtonMap[BTN_BASE3]	= KEYCODE_GAMEPAD_TL2;
		mButtonMap[BTN_BASE4]	= KEYCODE_GAMEPAD_TR2;
		mButtonMap[BTN_BASE5]	= KEYCODE_GAMEPAD_THUMBL;
		mButtonMap[BTN_BASE6]	= KEYCODE_GAMEPAD_THUMBR;
		mButtonMap[BTN_A]	= KEYCODE_GAMEPAD_A;
		mButtonMap[BTN_B]	= KEYCODE_GAMEPAD_B;
		mButtonMap[BTN_C]	= KEYCODE_GAMEPAD_C;
		mButtonMap[BTN_X]	= KEYCODE_GAMEPAD_X;
		mButtonMap[BTN_Y]	= KEYCODE_GAMEPAD_Y;
		mButtonMap[BTN_Z]	= KEYCODE_GAMEPAD_Z;
		mButtonMap[BTN_TL]	= KEYCODE_GAMEPAD_TL;
		mButtonMap[BTN_TR]	= KEYCODE_GAMEPAD_TR;
		mButtonMap[BTN_TL2]	= KEYCODE_GAMEPAD_TL2;
		mButtonMap[BTN_TR2]	= KEYCODE_GAMEPAD_TR2;
		mButtonMap[BTN_SELECT]	= KEYCODE_GAMEPAD_SELECT;
		mButtonMap[BTN_START]	= KEYCODE_GAMEPAD_START;
		mButtonMap[BTN_MODE]	= KEYCODE_GAMEPAD_MODE;
		mButtonMap[BTN_THUMBL]	= KEYCODE_GAMEPAD_THUMBL;
		mButtonMap[BTN_THUMBR]	= KEYCODE_GAMEPAD_THUMBR;
	}
	mMinX = 0;
	mMaxX = 0;
	mMinY = 0;
	mMaxY = 0;

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
			logfe ("Couldn't grab %s.\n", mDeviceName.c_str ());
			mGrabed = false;
		}
		else
		{
			mGrabed = true;
		}
	}
	mModifiers = 0;

	// Update device info
	struct input_info info;
	get_device_info (mFd, &info, false);
	mInfo.pid = info.id.product;
	mInfo.vid = info.id.vendor;
	mInfo.version = info.id.version;
	mInfo.bustype = info.id.bustype;

	mRouton = IsRoutonKeyboard(info.name);
	mHasPointer = info.num_rels || info.num_abs;
	mHasKey = info.num_keys || info.num_ext_keys;

	mDriver->AddDevice (mDeviceName, mInfo);

	logfi ("Device %s reopened.\n", mDeviceName.c_str ());


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
	if (keysym >= KEY_F1 && keysym <= KEY_F10)
		return KEYCODE_F1 + keysym - KEY_F1;

	return 0;
}

bool LinuxInputInterface::HandleKeyEvent (struct input_event & linux_event,
					  int &modifiers, Event & event)
{
     if (linux_event.code == BTN_TOUCH || linux_event.code == BTN_TOOL_FINGER)
	  linux_event.code = BTN_MOUSE;

	if ((linux_event.code >= BTN_MOUSE && linux_event.code < BTN_JOYSTICK) ||
	    linux_event.code == BTN_TOUCH)
	{
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
		if (!keycode && mButtonMap.find(linux_event.code) != mButtonMap.end())
			keycode = mButtonMap[linux_event.code];

		if (GetEnvOption("SEXY_LINUX_INPUT_DEBUG", false))
			logfd("keyCode: 0x%x => 0x%x", linux_event.code, keycode);

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

		if (GetEnvOption("SEXY_LINUX_INPUT_DEBUG", false))
			logfd ("keycode: %s\n", GetKeyNameFromCode((KeyCode)keycode).c_str());

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

bool LinuxInputInterface::HandleRelEvent (struct input_event & linux_event,
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

bool LinuxInputInterface::HandleAbsEvent (struct input_event & linux_event,
					  Event & event)
{
	if (!mHasJoystick)
		return false;

	AxisInfoMap::iterator it = mAxisInfoMap.find(linux_event.code);
	if (it == mAxisInfoMap.end())
		return false;

	LinuxAxisInfo& info = it->second;
	float value;

	if (info.devMaximum != info.devMinimum)
	{
		const float* coef = info.coef;
		value = (linux_event.value + coef[0]) * coef[1] + coef[2];
	}
	else
	{
		if (linux_event.value < 0)
			value = -1.0f;
		else if (linux_event.value > 0)
			value = 1.0f;
		else
			value = 0.0f;
	}
	event.type = EVENT_AXIS_MOVED;
	event.flags = 0;
	event.u.axis.axis = linux_event.code;
	event.u.axis.value = value;
	event.u.axis.flat = info.flat;
	event.u.axis.fuzz = info.fuzz;
	event.u.axis.minimum = info.minimum;
	event.u.axis.maximum = info.maximum;

	if (GetEnvOption("SEXY_LINUX_INPUT_DEBUG", false))
		logfd("LinuxInput:%p%d: AxisMoved[0x02%x]: value: %f raw value: 0x%x(%d:%d)",
		      this, mId, event.u.axis.axis, value, linux_event.value,
		      info.devMinimum, info.devMaximum);

	return true;
}

bool LinuxInputInterface::HandleEvent (struct input_event& linux_event,
				       int &modifiers, Event &event)
{
	switch (linux_event.type) {
	case EV_KEY:
		return HandleKeyEvent (linux_event, modifiers, event);
		break;
	case EV_REL:
		return HandleRelEvent (linux_event, event);
	case EV_ABS:
		return HandleAbsEvent (linux_event, event);
	default:
		break;
	}
	return false;
}

void  LinuxInputInterface::HandleEvents(struct input_event* linux_event, int nevents)
{
	Event event;

	memset (&event, 0, sizeof (event));
	for (int i = 0; i < nevents; i++)
	{
		if (GetEnvOption("SEXY_LINUX_INPUT_DEBUG", false))
			logfd ("LinuxInput:%p:%d: input_event: type %d code %d value: %d\n",
			       this, mId, linux_event[i].type, linux_event[i].code,
			       linux_event[i].value);

		HandleEvent (linux_event[i], mModifiers, event);
		if (event.type == EVENT_NONE)
			continue;

		PostEvent (event);
		memset (&event, 0, sizeof (event));
	}
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

		FD_ZERO (&set);
		FD_SET (fd, &set);

		struct timeval timeout;

		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		status = select (fd + 1, &set, NULL, NULL, &timeout);
		if (status < 0 && errno != EINTR)
		{
			logfi ("Device disconnected(hotpluged ? %s).\n",
			       driver->mHotpluged ? "yes" : "no");

			driver->CloseDevice ();
			goto disconnected;
		}
		if (status < 0)
			continue;
		if (driver->mDone)
			break;
		if (!FD_ISSET (fd, &set))
			continue;

		ssize_t readlen;
		readlen = read (fd, linux_event, sizeof (linux_event));

		if (driver->mDone)
			break;

		if (readlen < 0 && errno != EINTR)
		{
			logfi ("Device disconnected (hotpluged? %s).\n",
				driver->mHotpluged ? "yes" : "no");
			driver->CloseDevice ();
			goto disconnected;
		}

		if (readlen <= 0)
			continue;

		readlen /= sizeof (linux_event[0]);
		driver->HandleEvents(linux_event, readlen);
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

bool LinuxInputInterface::IsGrabbed ()
{
	return mGrabed;
}

bool LinuxInputInterface::Grab (bool grab)
{
	int ret = ioctl (mFd, EVIOCGRAB, grab == true ? 1 : 0);
	if (ret == 0)
		mGrabed = grab;
	return ret == 0;
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
