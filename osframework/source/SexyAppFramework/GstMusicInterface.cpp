#include "GstMusicInterface.h"
#include "GstSoundInstance.h"
#include "SexyAppBase.h"

using namespace Sexy;

GstMusicInfo::GstMusicInfo ()
{
	mPlayer = 0;
	mVolume = 1.0;
}

GstMusicInterface::GstMusicInterface()
{
	mVolume = 1.0;
	gst_init (NULL, NULL);
}

GstMusicInterface::~GstMusicInterface()
{
	UnloadAllMusic ();
}

bool GstMusicInterface::LoadMusic(int theSongId, const std::string& theFileName)
{
	printf ("sound id %d file %s\n", theSongId, theFileName.c_str ());

	GstMusicInfo aMusicInfo;
	aMusicInfo.mPlayer = new GstSoundInstance (0, theFileName);
	mMusicMap.insert(GstMusicMap::value_type(theSongId, aMusicInfo));
	return true;
}

void GstMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
	printf ("sound id %d offset %dnoloop %d\n", theSongId, theOffset, noLoop);

	GstMusicMap::iterator anIt;

	anIt= mMusicMap.find (theSongId);
	if (anIt == mMusicMap.end ())
		return;

	GstMusicInfo* aMusicInfo = &anIt->second;
	aMusicInfo->mPlayer->Play (!noLoop, false);
}

void GstMusicInterface::StopMusic(int theSongId)
{
	GstMusicMap::iterator anIt;

	anIt= mMusicMap.find (theSongId);
	if (anIt == mMusicMap.end ())
		return;

	GstMusicInfo* aMusicInfo = &anIt->second;
	aMusicInfo->mPlayer->Stop ();
}

void GstMusicInterface::PauseMusic(int theSongId)
{
	GstMusicMap::iterator anIt;

	anIt= mMusicMap.find (theSongId);
	if (anIt == mMusicMap.end ())
		return;

	GstMusicInfo* aMusicInfo = &anIt->second;
	aMusicInfo->mPlayer->Pause ();
}

void GstMusicInterface::ResumeMusic(int theSongId)
{
	GstMusicMap::iterator anIt;

	anIt= mMusicMap.find (theSongId);
	if (anIt == mMusicMap.end ())
		return;

	GstMusicInfo* aMusicInfo = &anIt->second;
	aMusicInfo->mPlayer->Resume ();
}

void GstMusicInterface::StopAllMusic()
{
	GstMusicMap::iterator anIt;

	for (anIt = mMusicMap.begin (); anIt != mMusicMap.end (); ++anIt)
	{
		GstMusicInfo* aMusicInfo = &anIt->second;

                if (aMusicInfo->mPlayer)
			aMusicInfo->mPlayer->Stop ();
        }
}

void GstMusicInterface::UnloadMusic(int theSongId)
{
	GstMusicMap::iterator anIt;

	anIt= mMusicMap.find (theSongId);
	if (anIt == mMusicMap.end ())
		return;

	GstMusicInfo* aMusicInfo = &anIt->second;

	if (aMusicInfo->mPlayer)
		delete aMusicInfo->mPlayer;
	mMusicMap.erase (anIt);
}

void GstMusicInterface::UnloadAllMusic()
{
	GstMusicMap::iterator anIt;

	for (anIt = mMusicMap.begin (); anIt != mMusicMap.end (); ++anIt)
	{
		GstMusicInfo* aMusicInfo = &anIt->second;

                if (aMusicInfo->mPlayer)
			delete aMusicInfo->mPlayer;
        }

	mMusicMap.clear();
}

void GstMusicInterface::PauseAllMusic()
{
	GstMusicMap::iterator anIt;

	for (anIt = mMusicMap.begin (); anIt != mMusicMap.end (); ++anIt)
	{
		GstMusicInfo* aMusicInfo = &anIt->second;

                if (aMusicInfo->mPlayer)
			aMusicInfo->mPlayer->Pause ();
        }
}

void GstMusicInterface::ResumeAllMusic()
{
	GstMusicMap::iterator anIt;

	for (anIt = mMusicMap.begin (); anIt != mMusicMap.end (); ++anIt)
	{
		GstMusicInfo* aMusicInfo = &anIt->second;

                if (aMusicInfo->mPlayer)
		{
			aMusicInfo->mPlayer->Resume ();
			aMusicInfo->mPlayer->SetVolume (mVolume * aMusicInfo->mVolume);
		}
        }
}

void GstMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
	PlayMusic (theSongId, theOffset, noLoop);
}

void GstMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
	StopMusic (theSongId);
}

void GstMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
	StopAllMusic ();
}

void GstMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
	GstMusicMap::iterator anIt;

	anIt= mMusicMap.find (theSongId);
	if (anIt == mMusicMap.end ())
		return;

	GstMusicInfo* aMusicInfo = &anIt->second;

	if (aMusicInfo->mPlayer)
	{
		aMusicInfo->mPlayer->SetVolume (theVolume);
		aMusicInfo->mVolume = theVolume;
	}
}

void GstMusicInterface::SetSongMaxVolume(int theSongId, double theMaxVolume)
{
}

bool GstMusicInterface::IsPlaying(int theSongId)
{
	GstMusicMap::iterator anIt;

	for (anIt = mMusicMap.begin (); anIt != mMusicMap.end(); ++anIt)
	{
		GstMusicInfo* aMusicInfo = &anIt->second;

                if (aMusicInfo->mPlayer &&
		    aMusicInfo->mPlayer->IsPlaying ())
			return true;
        }

	return false;
}

void GstMusicInterface::SetVolume(double theVolume)
{
	mVolume = theVolume;

	GstMusicMap::iterator anIt;

	for (anIt = mMusicMap.begin (); anIt != mMusicMap.end(); ++anIt)
	{
		GstMusicInfo* aMusicInfo = &anIt->second;

                if (aMusicInfo->mPlayer &&
		    aMusicInfo->mPlayer->IsPlaying ())
			aMusicInfo->mPlayer->SetVolume (mVolume * aMusicInfo->mVolume);
        }
}

void GstMusicInterface::SetMusicAmplify(int theSongId, double theAmp)
{
}

void GstMusicInterface::Update()
{
}
