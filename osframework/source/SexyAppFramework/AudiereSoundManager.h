/* Original Audiere Sound Manager  by Rheenen 2005 */

#ifndef __AudiereSoundMANAGER_H__
#define __AudiereSoundMANAGER_H__

#include "SoundManager.h"
#include "audiere.h"

using namespace audiere;

namespace Sexy
{

class AudiereSoundInstance;
class AudiereSoundInfo
{
public:
	int             m_stream_length;
	int             m_channel_count;
	int             m_sample_rate;
	SampleFormat    m_sample_format;

	unsigned char * m_buffer;

public:
	AudiereSoundInfo();
	~AudiereSoundInfo();
	void Reset();
};

class AudiereSoundManager : public SoundManager
{
	friend class AudiereSoundInstance;

protected:
	SampleSourcePtr			        mSourceSounds[MAX_SOURCE_SOUNDS];
	AudiereSoundInfo			mSourceInfos[MAX_SOURCE_SOUNDS];
	float					mBaseVolumes[MAX_SOURCE_SOUNDS];
	int					mBasePans[MAX_SOURCE_SOUNDS];
	float					mBasePitches[MAX_SOURCE_SOUNDS];
	AudiereSoundInstance*	                mPlayingSounds[MAX_CHANNELS];
	float					mMasterVolume;
	DWORD                                   mLastReleaseTick;

protected:
	int					FindFreeChannel();
	void					ReleaseFreeChannels();

public:
	AudioDevicePtr			        mDevice;

public:
	AudiereSoundManager();
	virtual ~AudiereSoundManager();

	virtual bool			Initialized();

	virtual bool			LoadSound(unsigned int theSfxID, const std::string& theFilename);
	virtual int			LoadSound(const std::string& theFilename);
	virtual void			ReleaseSound(unsigned int theSfxID);
	virtual int			GetFreeSoundId();
	virtual int			GetNumSounds();

	virtual void			SetVolume(double theVolume);
	virtual bool			SetBaseVolume(unsigned int theSfxID, double theBaseVolume);
	virtual bool			SetBasePan(unsigned int theSfxID, int theBasePan);
	virtual bool			SetBasePitch(unsigned int theSfxID, float theBasePitch);

	virtual SoundInstance*	        GetSoundInstance(unsigned int theSfxID);

	virtual void			ReleaseSounds();
	virtual void			ReleaseChannels();

	virtual double			GetMasterVolume();
	virtual void			SetMasterVolume(double theVolume);

	virtual void			Flush();

	virtual void			SetCooperativeWindow(HWND theHWnd, bool isWindowed);
	virtual void			StopAllSounds();
};

}

#endif //__AudiereSoundMANAGER_H__

