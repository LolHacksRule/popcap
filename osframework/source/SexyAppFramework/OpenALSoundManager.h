#ifndef __OPENALSOUNDMANAGER_H__
#define __OPENALSOUNDMANAGER_H__

#include "SoundManager.h"
#include "SoundDriverFactory.h"

#include "OpenALManager.h"

namespace Sexy
{

class OpenALSoundInstance;

class OpenALSoundManager : public SoundManager
{
	friend class OpenALSoundInstance;
	friend class OpenALSoundMusicInterface;

protected:
	OpenALManager*                          mManager;
	bool                                    mHasSourceSounds[MAX_SOURCE_SOUNDS];
	ALuint              		        mSourceSounds[MAX_SOURCE_SOUNDS];
	std::string				mSourceFileNames[MAX_SOURCE_SOUNDS];
	int              		        mPrimaryBuffer;
	ulong					mSourceDataSizes[MAX_SOURCE_SOUNDS];
	double					mBaseVolumes[MAX_SOURCE_SOUNDS];
	int					mBasePans[MAX_SOURCE_SOUNDS];
	OpenALSoundInstance*			mPlayingSounds[MAX_CHANNELS];
	double					mMasterVolume;
	DWORD					mLastReleaseTick;

	bool                                    mInitialized;

protected:
	int					FindFreeChannel();
	int					VolumeToDB(double theVolume);
	bool					LoadOGGSound(unsigned int theSfxID,
							     const std::string& theFilename);
	bool					LoadWAVSound(unsigned int theSfxID,
							     const std::string& theFilename);
	bool					LoadAUSound(unsigned int theSfxID,
							    const std::string& theFilename);
	void			 		ReleaseFreeChannels();
	OpenALManager*                          GetOpenALManager();

public:
	OpenALSoundManager();
	virtual ~OpenALSoundManager();

	virtual bool			Initialized();

	virtual bool			LoadSound(unsigned int theSfxID,
						  const std::string& theFilename);
	virtual int			LoadSound(const std::string& theFilename);
	virtual void			ReleaseSound(unsigned int theSfxID);

	virtual void			SetVolume(double theVolume);
	virtual bool			SetBaseVolume(unsigned int theSfxID, double theBaseVolume);
	virtual bool			SetBasePan(unsigned int theSfxID, int theBasePan);

	virtual SoundInstance*	        GetSoundInstance(unsigned int theSfxID);

	virtual void			ReleaseSounds();
	virtual void			ReleaseChannels();

	virtual double			GetMasterVolume();
	virtual void			SetMasterVolume(double theVolume);

	virtual void			Flush();

	virtual void			SetCooperativeWindow(HWND theHWnd, bool isWindowed);
	virtual void			StopAllSounds();
	virtual int			GetFreeSoundId();
	virtual int			GetNumSounds();
};

}

#endif //__OPENALSOUNDMANAGER_H__
