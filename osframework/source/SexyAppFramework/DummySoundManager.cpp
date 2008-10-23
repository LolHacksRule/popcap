#include "DummySoundManager.h"
#include "DummySoundInstance.h"
#include "Debug.h"

#include <fcntl.h>
#include <math.h>

using namespace Sexy;

#define USE_OGG_LIB


DummySoundManager::DummySoundManager()
{
	mLastReleaseTick = 0;

	int i;

	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
	}

	for (i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i] = NULL;

	mMasterVolume = 1.0;
}

DummySoundManager::~DummySoundManager()
{
	ReleaseChannels();
	ReleaseSounds();
}

int	DummySoundManager::FindFreeChannel()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] == NULL)
			return i;

		if (mPlayingSounds[i]->IsReleased())
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
			return i;
		}
	}

	return -1;
}

bool DummySoundManager::Initialized()
{
	return false;
}

int DummySoundManager::VolumeToDB(double theVolume)
{
	int aVol = (int) ((log10(1 + theVolume*9) - 1.0) * 2333);
	if (aVol < -2000)
		aVol = -10000;

	return aVol;
}

void DummySoundManager::SetVolume(double theVolume)
{
	mMasterVolume = theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
			mPlayingSounds[i]->RehupVolume();
}

bool DummySoundManager::LoadSound(unsigned int theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);
	mSourceFileNames[theSfxID] = theFilename;

	return true;
}

int DummySoundManager::LoadSound(const std::string& theFilename)
{
	int i;
	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceFileNames[i] == theFilename)
			return i;

	for (i = MAX_SOURCE_SOUNDS - 1; i >= 0; i--)
	{
		if (mSourceFileNames[i].length() == 0)
		{
			if (!LoadSound(i, theFilename))
				return -1;
			else
				return i;
		}
	}
	return -1;
}

void DummySoundManager::ReleaseSound(unsigned int theSfxID)
{
}

int DummySoundManager::GetFreeSoundId()
{
	return -1;
}

int DummySoundManager::GetNumSounds()
{
	int aCount = 0;
	return aCount;
}

bool DummySoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = theBaseVolume;
	return true;
}

bool DummySoundManager::SetBasePan(unsigned int theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}

SoundInstance* DummySoundManager::GetSoundInstance(unsigned int theSfxID)
{
	if (theSfxID > MAX_SOURCE_SOUNDS)
		return NULL;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return NULL;


	if (mSourceFileNames[theSfxID].length() == 0)
		return NULL;

	mPlayingSounds[aFreeChannel] = new DummySoundInstance(this);
	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

void DummySoundManager::ReleaseSounds()
{
}

void DummySoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
}

void DummySoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL && mPlayingSounds[i]->IsReleased())
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
}

void DummySoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
}


double DummySoundManager::GetMasterVolume()
{
}

void DummySoundManager::SetMasterVolume(double theVolume)
{
}

void DummySoundManager::Flush()
{
}

void DummySoundManager::SetCooperativeWindow(HWND theHWnd, bool isWindowed)
{
}
#undef SOUND_FLAGS
