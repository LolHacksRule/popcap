#include "GstSoundManager.h"
#include "GstSoundInstance.h"
#include "Debug.h"
#include "SoundDriverFactory.h"
#include "GstMusicInterface.h"
#include "GstPakSrc.h"

#include <fcntl.h>
#include <math.h>

using namespace Sexy;

GstSoundManager::GstSoundManager()
{
	g_get_current_time (&mLastReleaseTime);

	int i;

	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
	}

	for (i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i] = NULL;

	mMasterVolume = 1.0;

	GThread* thread;
	mLoop = g_main_loop_new (NULL, FALSE);
	thread = g_thread_create (MainLoop, mLoop, TRUE, NULL);
}

GstSoundManager::~GstSoundManager()
{
	if (mLoop)
		g_main_loop_quit (mLoop);
	g_main_loop_unref (mLoop);

	ReleaseChannels();
	ReleaseSounds();
}

void* GstSoundManager::MainLoop (void* data)
{
	GMainLoop * loop = (GMainLoop *)data;

	g_main_loop_run (loop);
	return NULL;
}

int GstSoundManager::FindFreeChannel()
{
	GTimeVal now;

	g_get_current_time (&now);

	gulong diff = (now.tv_sec - mLastReleaseTime.tv_sec) * 1000;
	diff += now.tv_usec - mLastReleaseTime.tv_usec;
	if (ABS (diff) > 1000)
	{
		ReleaseFreeChannels ();
		mLastReleaseTime = now;
	}

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

bool GstSoundManager::Initialized()
{
	return true;
}

int GstSoundManager::VolumeToDB(double theVolume)
{
	int aVol = (int) ((log10(1 + theVolume * 9) - 1.0) * 2333);
	if (aVol < -2000)
		aVol = -10000;

	return aVol;
}

void GstSoundManager::SetVolume(double theVolume)
{
	mMasterVolume = theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
			mPlayingSounds[i]->RehupVolume();
}

bool GstSoundManager::LoadSound(unsigned int theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);
	mSourceFileNames[theSfxID] = theFilename;

	return true;
}

int GstSoundManager::LoadSound(const std::string& theFilename)
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

void GstSoundManager::ReleaseSound(unsigned int theSfxID)
{
	if (mSourceFileNames[theSfxID].length() != 0)
		mSourceFileNames[theSfxID] = "";
}

int GstSoundManager::GetFreeSoundId()
{
	int i;
	for (i = MAX_SOURCE_SOUNDS - 1; i >= 0; i--)
	{
		if (mSourceFileNames[i].length() == 0)
			return i;
	}
	return -1;
}

int GstSoundManager::GetNumSounds()
{
	int aCount = 0;
	int i;

	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceFileNames[i].length () != 0)
			aCount++;
	return aCount;
}

bool GstSoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = theBaseVolume;
	return true;
}

bool GstSoundManager::SetBasePan(unsigned int theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}

SoundInstance* GstSoundManager::GetSoundInstance(unsigned int theSfxID)
{
	if (theSfxID > MAX_SOURCE_SOUNDS)
		return NULL;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return NULL;


	if (mSourceFileNames[theSfxID].length() == 0)
		return NULL;

	mPlayingSounds[aFreeChannel] =
		new GstSoundInstance(this,
				     mSourceFileNames[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

void GstSoundManager::ReleaseSounds()
{
	int i;

	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
		mSourceFileNames[i].clear ();
}

void GstSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
}

void GstSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL && mPlayingSounds[i]->IsReleased())
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
}

void GstSoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
		{
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
}


double GstSoundManager::GetMasterVolume()
{
	return mMasterVolume;
}

void GstSoundManager::SetMasterVolume(double theVolume)
{
	mMasterVolume = theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != NULL)
		{
			double volume = mPlayingSounds[i]->GetVolume ();
			mPlayingSounds[i]->SetVolume(volume);
		}
	}
}

void GstSoundManager::Flush()
{
}

void GstSoundManager::SetCooperativeWindow(HWND theHWnd, bool isWindowed)
{
}

class GstSoundDriver: public SoundDriver {
public:
	GstSoundDriver ()
		: SoundDriver("Gst", 0),
		  mInitialized (false)
	{
	}

	void Init ()
	{
		if (mInitialized)
			return;

		gst_init (NULL, NULL);
		gst_pak_src_plugin_register ();

		mInitialized = true;
	}

	SoundManager* Create (SexyAppBase * theApp)
	{
		Init ();
		return new GstSoundManager ();
        }

	MusicInterface* CreateMusicInterface (SexyAppBase * theApp)
	{
		Init ();
		return new GstMusicInterface ();
        }
private:
	bool     mInitialized;
};

static GstSoundDriver aGstSoundDriver;
SoundDriverRegistor aGstSoundDriverRegistor (&aGstSoundDriver);
SoundDriver* GetGstSoundDriver()
{
	return &aGstSoundDriver;
}
