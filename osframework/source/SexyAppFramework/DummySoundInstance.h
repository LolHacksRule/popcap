#ifndef __DummySOUNDINSTANCE_H__
#define __DummySOUNDINSTANCE_H__

#include "SoundInstance.h"

namespace Sexy
{

class DummySoundManager;

class DummySoundInstance : public SoundInstance
{
	friend class DummySoundManager;

protected:
	DummySoundManager*			mSoundManagerP;
	bool					mAutoRelease;
	bool					mHasPlayed;
	bool					mReleased;

	int					mBasePan;
	double					mBaseVolume;

	int					mPan;
	double					mVolume;

	DWORD					mDefaultFrequency;

protected:
	void					RehupVolume();
	void					RehupPan();

public:
	DummySoundInstance(DummySoundManager* theSoundManager);
	virtual ~DummySoundInstance();
	virtual void			Release();

	virtual void			SetBaseVolume(double theBaseVolume);
	virtual void			SetBasePan(int theBasePan);

	virtual void			SetVolume(double theVolume);
	virtual void			SetPan(int thePosition); //-hundredth db to +hundredth db = left to right
	virtual void			AdjustPitch(double theNumSteps);

	virtual bool			Play(bool looping, bool autoRelease);
	virtual void			Stop();
	virtual bool			IsPlaying();
	virtual bool			IsReleased();
	virtual double			GetVolume();

};

}

#endif //__DSOUNDINSTANCE_H__
