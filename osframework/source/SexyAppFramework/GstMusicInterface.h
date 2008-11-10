#ifndef __GSTMUSICINTERFACE_H__
#define __GSTMUSICINTERFACE_H__

#include "Common.h"
#include "MusicInterface.h"
#include "GstSoundInstance.h"

#include <map>

namespace Sexy
{

class GstMusicInfo
{
public:
        GstSoundInstance*  mPlayer;

public:
	GstMusicInfo ();
};

typedef std::map<int, GstMusicInfo> GstMusicMap;

class GstMusicInterface : public MusicInterface
{
public:

public:
	GstMusicInterface();
	virtual ~GstMusicInterface();

	virtual bool			LoadMusic(int theSongId, const std::string& theFileName);
	virtual void			PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false);
	virtual void			StopMusic(int theSongId);
	virtual void			PauseMusic(int theSongId);
	virtual void			ResumeMusic(int theSongId);
	virtual void			StopAllMusic();

	virtual void			UnloadMusic(int theSongId);
	virtual void			UnloadAllMusic();
	virtual void			PauseAllMusic();
	virtual void			ResumeAllMusic();

	virtual void			FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false);
	virtual void			FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004);
	virtual void			FadeOutAll(bool stopSong = true, double theSpeed = 0.004);
	virtual void			SetSongVolume(int theSongId, double theVolume);
	virtual void			SetSongMaxVolume(int theSongId, double theMaxVolume);
	virtual bool			IsPlaying(int theSongId);

	virtual void			SetVolume(double theVolume);
	virtual void			SetMusicAmplify(int theSongId, double theAmp);
	virtual void			Update();

private:
        GstMusicMap                             mMusicMap;
};

}

#endif //__MUSICINTERFACE_H__
