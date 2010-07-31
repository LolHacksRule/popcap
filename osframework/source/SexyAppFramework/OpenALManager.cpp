#include "OpenALManager.h"
#include "SexyAppBase.h"
#include "InputManager.h"

using namespace Sexy;

OpenALManager::OpenALManager()
{
	mInitialized = false;
}


OpenALManager::~OpenALManager()
{
	uninitialize();
}

void OpenALManager::uninitialize()
{
	if (!mInitialized)
		return;

	for (ALSources::iterator it = mSources.begin();
	     it != mSources.end(); ++it)
		alDeleteSources(1, &it->first);

	alcMakeContextCurrent(0);

	// destroy the context
	alcDestroyContext(mContext);

	// close the device
	alcCloseDevice(mDevice);
}

bool OpenALManager::initialize()
{
	if (mInitialized)
		return true;


	mDevice = alcOpenDevice(NULL);
	if (!mDevice)
		return false;

	// use the device to make a context
	mContext = alcCreateContext(mDevice, NULL);
	// set my context to the currently active one
	alcMakeContextCurrent(mContext);

	alGetError();
	for (ALuint i = 0; i < 255; i++)
	{
		ALuint id;

		alGenSources(1, &id);
		if (alGetError() != AL_NO_ERROR)
			break;

		mSources.insert(ALSources::value_type(id, false));
	}

#ifdef SEXY_IOS
	UInt32 category = kAudioSessionCategory_AmbientSound;

	OSStatus result = AudioSessionInitialize(NULL, NULL,
						 interruptionListenerCallback, this);
	result = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory,
					 sizeof(category), &category);
	AudioSessionSetActive(true);
#endif

	mInitialized = true;
	return true;
}

int OpenALManager::allocSource()
{
	if (!mInitialized)
		return -1;

	for (ALSources::iterator it = mSources.begin();
	     it != mSources.end(); ++it)
	{
		if (!it->second)
		{
			it->second = true;
			return it->first;
		}
	}

	return -1;
}

void OpenALManager::freeSource(int id)
{
	if (id < 0)
		return;

	ALSources::iterator it = mSources.find((ALuint)id);
	if (it == mSources.end())
		return;
	it->second = false;
}

#ifdef SEXY_IOS
void OpenALManager::haltOpenALSession()
{
	Event evt;

	evt.type = EVENT_ACTIVE;
	evt.flags = 0;
	evt.u.active.active = 0;
	if (gSexyAppBase)
		gSexyAppBase->mInputManager->PushEvent(evt);

	AudioSessionSetActive(false);
 
	alcMakeContextCurrent(0);
	alcSuspendContext(mContext);
}

void OpenALManager::resumeOpenALSession()
{
	Event evt;

	evt.type = EVENT_ACTIVE;
	evt.flags = 0;
	evt.u.active.active = 1;
	if (gSexyAppBase)
		gSexyAppBase->mInputManager->PushEvent(evt);

	UInt32 category = kAudioSessionCategory_AmbientSound;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory,
				sizeof (category), &category );
 
	AudioSessionSetActive(true);
 
	alcMakeContextCurrent(mContext);
	alcProcessContext(mContext);
}

void OpenALManager::interruptionListenerCallback (void *inUserData, UInt32 interruptionState)
{
 
        // you could do this with a cast below, but I will keep it here to make it clearer
	OpenALManager *mgr = (OpenALManager *) inUserData;
 
	if (interruptionState == kAudioSessionBeginInterruption)
	{
		mgr->haltOpenALSession();
	}
	else if (interruptionState == kAudioSessionEndInterruption)
	{
		mgr->resumeOpenALSession();
	}
}
#endif
