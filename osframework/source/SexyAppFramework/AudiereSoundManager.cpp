/* Original Audiere Sound Manager  by Rheenen 2005 */

#include "SoundDriverFactory.h"
#include "AudiereMusicInterface.h"
#include "AudiereSoundManager.h"
#include "AudiereSoundInstance.h"
#include "AudiereMusicInterface.h"
#include "AudiereLoader.h"
#include "audiere.h"

using namespace Sexy;
using namespace audiere;

AudiereSoundManager::AudiereSoundManager()
{
	mDevice = getAudiereDevice();

	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mSourceSounds[i] = NULL;
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
		mBasePitches[i] = 1;
	}

	for (int i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i] = NULL;

	mMasterVolume = 1.0;
}

AudiereSoundManager::~AudiereSoundManager()
{
	ReleaseChannels();
	ReleaseSounds();
}

int	AudiereSoundManager::FindFreeChannel()
{
#if 0
	Uint32 aTick = SDL_GetTicks();
	if (aTick-mLastReleaseTick > 1000)
	{
		ReleaseFreeChannels();
		mLastReleaseTick = aTick;
	}
#endif
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] == NULL)
			return i;

		if (mPlayingSounds[i]->IsReleased())
		{
			mPlayingSounds[i] = NULL;
			return i;
		}
	}

	return -1;
}

bool AudiereSoundManager::Initialized()
{
	return (mDevice.get() != NULL);
}

void AudiereSoundManager::SetVolume(double theVolume)
{
	mMasterVolume = (float)theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
			mPlayingSounds[i]->RehupVolume();
}

int AudiereSoundManager::LoadSound(const std::string& theFilename) {
	int id = GetFreeSoundId();

	if (id != -1) {
		if (LoadSound(id, theFilename)) {
			return id;
		}
	}
	return -1;
}

bool AudiereSoundManager::LoadSound(unsigned int theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);

	// sounds just	won't play, but this is not treated as a failure condition
	if (!mDevice)
		return true;

	std::string aFilename = theFilename;

	int aLastDotPos = aFilename.rfind('.');
	int aLastSlashPos = std::max((int) aFilename.rfind('\\'), (int) aFilename.rfind('/'));
	if (aLastSlashPos < 0)
		aLastSlashPos = 0;

	mSourceSounds[theSfxID] = OpenSampleSource(aFilename.c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = OpenSampleSource((aFilename + ".wav").c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = OpenSampleSource((aFilename + ".ogg").c_str());

	if (!mSourceSounds[theSfxID])
		mSourceSounds[theSfxID] = OpenSampleSource((aFilename + ".mp3").c_str());

	return (mSourceSounds[theSfxID]);
}

void AudiereSoundManager::ReleaseSound(unsigned int theSfxID)
{
	mSourceSounds[theSfxID] = NULL;
}

bool AudiereSoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = (float)theBaseVolume;
	return true;
}

bool AudiereSoundManager::SetBasePan(unsigned int theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}

bool AudiereSoundManager::SetBasePitch(unsigned int theSfxID, float theBasePitch)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePitches[theSfxID] = theBasePitch;
	return true;
}

SoundInstance* AudiereSoundManager::GetSoundInstance(unsigned int theSfxID)
{
	if (theSfxID > MAX_SOURCE_SOUNDS)
		return NULL;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return NULL;


	if (!mDevice)
	{
		mPlayingSounds[aFreeChannel] = new
			AudiereSoundInstance(this, NULL);
	}
	else
	{
		if (!mSourceSounds[theSfxID])
			return NULL;
		mPlayingSounds[aFreeChannel] = new
			AudiereSoundInstance(this, mSourceSounds[theSfxID]);
	}

	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);
	mPlayingSounds[aFreeChannel]->AdjustBasePitch(mBasePitches[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

void AudiereSoundManager::ReleaseSounds()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceSounds[i])
			mSourceSounds[i] = NULL;
}

void AudiereSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i])
		{
			mPlayingSounds[i]->Release();
			mPlayingSounds[i] = NULL;
		}
}

void AudiereSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] && mPlayingSounds[i]->IsReleased())
			mPlayingSounds[i] = NULL;
}

void AudiereSoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
}


double AudiereSoundManager::GetMasterVolume()
{
	return 1.0;
}

void AudiereSoundManager::SetMasterVolume(double theVolume)
{
	mMasterVolume = theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != NULL)
		{
			double volume = mPlayingSounds[i]->GetVolume ();
			mPlayingSounds[i]->SetVolume(volume);
		}
	}
}

void AudiereSoundManager::Flush()
{
}

void AudiereSoundManager::SetCooperativeWindow(HWND theHWnd, bool isWindowed)
{
}

int AudiereSoundManager::GetFreeSoundId()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; ++i)
		if (!mSourceSounds[i])
			return i;
	return -1;
}

int AudiereSoundManager::GetNumSounds()
{
	int nr_sounds = 0;

	for (int i = 0; i < MAX_SOURCE_SOUNDS; ++i)
		if (mSourceSounds[i])
			++nr_sounds;
	return nr_sounds;
}

class AudiereSoundDriver: public SoundDriver {
public:
	AudiereSoundDriver ()
		: SoundDriver("AudiereInterface", 0),
		  mInitialized (false)
	{
	}

	void Init ()
	{
		if (mInitialized)
			return;

		mInitialized = true;
	}

	SoundManager* Create (SexyAppBase * theApp)
	{
		Init ();
		return new AudiereSoundManager ();
        }

	MusicInterface* CreateMusicInterface (SexyAppBase * theApp)
	{
		Init ();
		return new AudiereMusicInterface ();
        }
private:
	bool     mInitialized;
};

static AudiereSoundDriver aAudiereSoundDriver;
SoundDriverRegistor aAudiereSoundDriverRegistor (&aAudiereSoundDriver);
SoundDriver* GetAudiereSoundDriver()
{
	return &aAudiereSoundDriver;
}
