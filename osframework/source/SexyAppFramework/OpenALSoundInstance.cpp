#include "OpenALSoundInstance.h"
#include "OpenALSoundManager.h"

#include <math.h>

using namespace Sexy;

OpenALSoundInstance::OpenALSoundInstance(OpenALSoundManager* theSoundManager,
					 ALuint theSourceSound)
{
	mSoundManagerP = theSoundManager;
	mReleased = false;
	mAutoRelease = false;
	mHasPlayed = false;
	mSoundBuffer = theSourceSound;
	mSoundSource = -1;

	mBaseVolume = 1.0;
	mBasePan = 0;

	mBasePitch = 1.0;
	mNumSteps = 1.0;

	mVolume = 1.0;
	mPan = 0;

	RehupVolume();
}

OpenALSoundInstance::~OpenALSoundInstance()
{
	Release();
}

void OpenALSoundInstance::RehupVolume()
{
	float aVolume = mBaseVolume * mVolume * mSoundManagerP->mMasterVolume;

	if (mSoundSource < 0)
		return;

	alSourcef((ALuint)mSoundSource, AL_GAIN, aVolume);
}

void OpenALSoundInstance::RehupPan()
{
	float aPan = mBasePan + mPan;
}

void OpenALSoundInstance::Release()
{
	Stop();
	mReleased = true;
}

void OpenALSoundInstance::SetVolume(double theVolume) // 0 = max
{
	mVolume = theVolume;
	RehupVolume();
}

void OpenALSoundInstance::SetPan(int thePosition) //-db to =db = left to right
{
	mPan = thePosition;
	RehupPan();
}

void OpenALSoundInstance::SetBaseVolume(double theBaseVolume)
{
	mBaseVolume = theBaseVolume;
	RehupVolume();
}

void OpenALSoundInstance::SetBasePan(int theBasePan)
{
	mBasePan = theBasePan;
	RehupPan();
}

bool OpenALSoundInstance::Play(bool looping, bool autoRelease)
{
	Stop();

	mHasPlayed = true;
	mAutoRelease = autoRelease;

	mSoundSource = mSoundManagerP->GetOpenALManager()->allocSource();
	if (mSoundSource < 0)
		return -1;

	ALuint source = mSoundSource;

	alGetError();
	alSourcei(source, AL_BUFFER, mSoundBuffer);
	if (alGetError() != AL_NO_ERROR)
		printf("OpenAL: bad buffer: %u\n", mSoundBuffer);

	alSourcef(source, AL_PITCH, 1.0);
	alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	alGetError();

	RehupVolume();

	alSourcePlayv(1, &source);
	alGetError();

	//printf("OpenAL: playing source: %d buffer: %u\n", mSoundSource, mSoundBuffer);

	return true;
}

void OpenALSoundInstance::Stop()
{
	mAutoRelease = false;

	ALuint source = mSoundSource;
	alSourceStopv(1, &source);
	alGetError();

	mSoundManagerP->GetOpenALManager()->freeSource(mSoundSource);
	mSoundSource = -1;
}

//#include "DirectXErrorString.h"
void OpenALSoundInstance::AdjustPitch(double theNumSteps)
{
	if (mSoundSource < 0)
		return;

	mNumSteps = theNumSteps;

	float pitch = pow(1.0594630943592952645618252949463, mBasePitch * theNumSteps);
	alSourcef((ALuint)mSoundSource, AL_PITCH, pitch);
	alGetError();
}

bool OpenALSoundInstance::IsPlaying()
{
	if (!mHasPlayed)
		return false;

	if (mSoundSource < 0)
		return false;

	ALint state;
	alGetSourcei((ALuint)mSoundSource, AL_SOURCE_STATE, &state);

	if (state == AL_PLAYING)
		return true;
	return false;
}

bool OpenALSoundInstance::IsReleased()
{
	if ((!mReleased) && (mAutoRelease) && (mHasPlayed) && (!IsPlaying()))
		Release();

	return mReleased;
}

double OpenALSoundInstance::GetVolume()
{
	return mVolume;
}

