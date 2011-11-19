#include "InputDriverFactory.h"

using namespace Sexy;

InputDriver::InputDriver (const std::string theName,
			  int		    thePriority)
	: Driver(theName, thePriority)
{
}

InputDriver::~InputDriver ()
{
}

InputDriverFactory::InputDriverFactory ()
	: DriverFactory ()
{
}

InputDriverFactory::~InputDriverFactory ()
{
}

namespace Sexy {

class StaticInputDriverFactory
{
public:
	struct StaticData {
		InputDriverFactory* mFactory;
		bool mDone;
	};

	StaticInputDriverFactory(StaticData* data)
	{
		mData = data;
	}

	InputDriverFactory* Get(StaticData* data)
	{
		if (data->mDone)
			return 0;

		if (data->mFactory)
			return data->mFactory;

		data->mFactory = new InputDriverFactory;
		return data->mFactory;
	}

	~StaticInputDriverFactory()
	{
		if (!mData)
			return;

		mData->mDone = true;
		if (mData->mFactory)
			delete mData->mFactory;
	}

private:
	StaticData* mData;
};

static StaticInputDriverFactory::StaticData aData;
static StaticInputDriverFactory inputDriverFactory(&aData);

}

InputDriverFactory* InputDriverFactory::GetInputDriverFactory ()
{
	return inputDriverFactory.Get(&aData);
}

/* This is a hack that preventing gcc from striping drivers out of
 * binary.
 */
extern InputDriver* GetLinuxInputDriver();
extern InputDriver* GetUdpInputDriver();
extern InputDriver* GetSMInputDriver();
extern InputDriver* GetModuleInputDriver();

typedef InputDriver* (* InputDriverGetter)();
InputDriverGetter InputDriverGetters []= {
#ifdef SEXY_LINUX_INPUT_DRIVER
	GetLinuxInputDriver,
#endif
#ifdef SEXY_SM_INPUT_DRIVER
	GetSMInputDriver,
#endif
#ifdef SEXY_UDP_INPUT_DRIVER
	GetUdpInputDriver,
#endif
#ifdef SEXY_MODULE_INPUT_DRIVER
	GetModuleInputDriver,
#endif
	NULL
};

void InputDriverFactory::Load(void)
{
	int i = 0;
	for (i = 0; InputDriverGetters[i]; i++)
		InputDriverGetters[i]();
}

