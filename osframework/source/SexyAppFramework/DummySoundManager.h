#ifndef __DUMMYSOUNDMANAGER_H__
#define __DUMMYSOUNDMANAGER_H__

#include "Common.h"
#include "SoundManager.h"

namespace Sexy
{

class DummySoundInstance;

class DummySoundManager : public SoundManager
{
	friend class DummySoundInstance;
	friend class DummySoundMusicInterface;

protected:
	std::string				mSourceFileNames[MAX_SOURCE_SOUNDS];
	ulong					mSourceDataSizes[MAX_SOURCE_SOUNDS];
	double					mBaseVolumes[MAX_SOURCE_SOUNDS];
	int						mBasePans[MAX_SOURCE_SOUNDS];
	DummySoundInstance*			mPlayingSounds[MAX_CHANNELS];
	double					mMasterVolume;
	DWORD					mLastReleaseTick;

protected:
	int						FindFreeChannel();
	int						VolumeToDB(double theVolume);
	bool					WriteWAV(unsigned int theSfxID, const std::string& theFilename, const std::string& theDepFile);
	void					ReleaseFreeChannels();

public:
	bool					mHaveFMod;

	DummySoundManager(void);
	virtual ~DummySoundManager();

	virtual bool			Initialized();

	virtual bool			LoadSound(unsigned int theSfxID, const std::string& theFilename);
	virtual int				LoadSound(const std::string& theFilename);
	virtual void			ReleaseSound(unsigned int theSfxID);

	virtual void			SetVolume(double theVolume);
	virtual bool			SetBaseVolume(unsigned int theSfxID, double theBaseVolume);
	virtual bool			SetBasePan(unsigned int theSfxID, int theBasePan);

	virtual SoundInstance*	GetSoundInstance(unsigned int theSfxID);

	virtual void			ReleaseSounds();
	virtual void			ReleaseChannels();

	virtual double			GetMasterVolume();
	virtual void			SetMasterVolume(double theVolume);

	virtual void			Flush();

	virtual void			SetCooperativeWindow(HWND theHWnd, bool isWindowed);
	virtual void			StopAllSounds();
	virtual int				GetFreeSoundId();
	virtual int				GetNumSounds();
};

}

#endif //__DSOUNDMANAGER_H__
