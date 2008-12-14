#include "DummySoundInstance.h"
#include "DummySoundManager.h"

using namespace Sexy;

DummySoundInstance::DummySoundInstance(DummySoundManager* theSoundManager)
{
	mSoundManagerP = theSoundManager;
	mReleased = false;
	mAutoRelease = false;
	mHasPlayed = false;
	mBaseVolume = 1.0;
	mBasePan = 0;

	mVolume = 1.0;
	mPan = 0;

	mDefaultFrequency = 44100;
	RehupVolume();
}

DummySoundInstance::~DummySoundInstance()
{
}

void DummySoundInstance::RehupVolume()
{
}

void DummySoundInstance::RehupPan()
{
}

void DummySoundInstance::Release()
{
	Stop();
	mReleased = true;
}

void DummySoundInstance::SetVolume(double theVolume) // 0 = max
{
	mVolume = theVolume;
	RehupVolume();
}

void DummySoundInstance::SetPan(int thePosition) //-db to =db = left to right
{
	mPan = thePosition;
	RehupPan();
}

void DummySoundInstance::SetBaseVolume(double theBaseVolume)
{
	mBaseVolume = theBaseVolume;
	RehupVolume();
}

void DummySoundInstance::SetBasePan(int theBasePan)
{
	mBasePan = theBasePan;
	RehupPan();
}

bool DummySoundInstance::Play(bool looping, bool autoRelease)
{
	Stop();

	mHasPlayed = true;
	mAutoRelease = autoRelease;

	return true;
}

void DummySoundInstance::Stop()
{
}

void DummySoundInstance::AdjustPitch(double theNumSteps)
{
}

bool DummySoundInstance::IsPlaying()
{
	if (!mHasPlayed)
		return false;

	return false;
}

bool DummySoundInstance::IsReleased()
{
	if ((!mReleased) && (mAutoRelease) && (mHasPlayed) && (!IsPlaying()))
		Release();

	return mReleased;
}

double DummySoundInstance::GetVolume()
{
	return mVolume;
}
