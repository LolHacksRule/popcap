#ifndef __OPENALSOUNDINSTANCE_H__
#define __OPENALSOUNDINSTANCE_H__

#include "SoundInstance.h"
#include "OpenALManager.h"

namespace Sexy
{

class OpenALSoundManager;

class OpenALSoundInstance : public SoundInstance
{
	friend class OpenALSoundManager;

protected:
	OpenALSoundManager*			mSoundManagerP;
	int                                     mSoundSource;
	ALuint                                  mSoundBuffer;
	bool					mAutoRelease;
	bool					mHasPlayed;
	bool					mReleased;

	int					mBasePan;
	double					mBaseVolume;
	double                                  mBasePitch;
	double                                  mNumSteps;

	int					mPan;
	double					mVolume;

	DWORD					mDefaultFrequency;

protected:
	void					RehupVolume();
	void					RehupPan();

public:
	OpenALSoundInstance(OpenALSoundManager* theSoundManager,
			    ALuint theSoundBuffer);
	virtual ~OpenALSoundInstance();

	virtual void			Release();

	virtual void			SetBaseVolume(double theBaseVolume);
	virtual void			SetBasePan(int theBasePan);

	virtual void			SetVolume(double theVolume);
	virtual void			SetPan(int thePosition);
	virtual void			AdjustPitch(double theNumSteps);

	virtual bool			Play(bool looping, bool autoRelease);
	virtual void			Stop();
	virtual bool			IsPlaying();
	virtual bool			IsReleased();
	virtual double			GetVolume();

};

}

#endif //__DSOUNDINSTANCE_H__
