#include "Common.h"
#include "OpenALSoundManager.h"
#include "OpenALSoundInstance.h"
#include "MusicInterface.h"
#include "Debug.h"
#include "SexyTimer.h"

#include <fcntl.h>
#include <math.h>

#include "../PakLib/PakInterface.h"

using namespace Sexy;

#define USE_OGG_LIB

#ifdef USE_OGG_LIB
#include "ogg/ivorbiscodec.h"
#include "ogg/ivorbisfile.h"
#endif

OpenALSoundManager::OpenALSoundManager()
{
	mLastReleaseTick = 0;
	mManager = new OpenALManager();

	int i;

	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mSourceSounds[i] = NULL;
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
	}

	for (i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i] = 0;

	mMasterVolume = 1.0;

	mInitialized = mManager->initialize();
}

OpenALSoundManager::~OpenALSoundManager()
{
	ReleaseChannels();
	ReleaseSounds();

	delete mManager;
}

int OpenALSoundManager::FindFreeChannel()
{
	DWORD aTick = GetTickCount();
	if (aTick-mLastReleaseTick > 1000)
	{
		ReleaseFreeChannels();
		mLastReleaseTick = aTick;
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

bool OpenALSoundManager::Initialized()
{
	return mInitialized;
}

int OpenALSoundManager::VolumeToDB(double theVolume)
{
	int aVol = (int) ((log10(1 + theVolume*9) - 1.0) * 2333);
	if (aVol < -2000)
		aVol = -10000;

	return aVol;
}

void OpenALSoundManager::SetVolume(double theVolume)
{
	mMasterVolume = theVolume;

	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != NULL)
			mPlayingSounds[i]->RehupVolume();
}

bool OpenALSoundManager::LoadWAVSound(unsigned int theSfxID, const std::string& theFilename)
{
	int aDataSize;

	PFILE* fp;

	fp = p_fopen(theFilename.c_str(), "rb");

	if (!fp)
		return false;

	char aChunkType[5];
	aChunkType[4] = '\0';
	ulong aChunkSize;

	p_fread(aChunkType, 1, 4, fp);
	if (!strcmp(aChunkType, "RIFF") == 0)
	{
		p_fclose(fp);
		return false;
	}

	p_fread(&aChunkSize, 4, 1, fp);

	p_fread(aChunkType, 1, 4, fp);
	if (!strcmp(aChunkType, "WAVE") == 0)
	{
		p_fclose(fp);
		return false;
	}

	ushort aBitCount = 16;
	ushort aChannelCount = 1;
	ulong aSampleRate = 22050;
	uchar anXor = 0;

	while (!p_feof(fp))
	{
		p_fread(aChunkType, 1, 4, fp);
		if (p_fread(&aChunkSize, 4, 1, fp) == 0)
		{
			p_fclose(fp);
			return false;
		}

		int aCurPos = p_ftell(fp);

		if (strcmp(aChunkType, "fmt ") == 0)
		{
			ushort aFormatTag;
			ulong aBytesPerSec;
			ushort aBlockAlign;

			p_fread(&aFormatTag, 2, 1, fp);
			p_fread(&aChannelCount, 2, 1, fp);
			p_fread(&aSampleRate, 4, 1, fp);
			p_fread(&aBytesPerSec, 4, 1, fp);
			p_fread(&aBlockAlign, 2, 1, fp);
			p_fread(&aBitCount, 2, 1, fp);

			if (aFormatTag != 1)
			{
				p_fclose(fp);
				return false;
			}
		}
		else if (strcmp(aChunkType, "dep ") == 0)
		{
			char aStr[256];
			ushort aStrLen;

			p_fread(&aStrLen, 2, 1, fp);
			if (aStrLen > 255)
				aStrLen = 255;
			p_fread(aStr, 1, aStrLen, fp);
			aStr[aStrLen] = '\0';

			uchar aSavedFileTime[8];
			p_fread(&aSavedFileTime, sizeof(aSavedFileTime), 1, fp);
		}
		else if (strcmp(aChunkType, "xor ") == 0)
		{
			p_fread(&anXor, 1, 1, fp);
		}
		else if (strcmp(aChunkType, "data") == 0)
		{
			aDataSize = aChunkSize;
			mSourceDataSizes[theSfxID] = aChunkSize;

			ALenum format = 0;
			if (aChannelCount == 1)
			{
				if (aBitCount == 8)
					format = AL_FORMAT_MONO8;
				else if (aBitCount == 16)
					format = AL_FORMAT_MONO16;
			}
			else if (aChannelCount == 2)
			{
				if (aBitCount == 8)
					format = AL_FORMAT_STEREO8;
				else if (aBitCount == 16)
					format = AL_FORMAT_STEREO16;
			}
			if (format == 0)
			{
				p_fclose(fp);
				return false;
			}
			ALuint buffer;

			alGetError();
			alGenBuffers(1, &buffer);
			if (alGetError() != AL_NO_ERROR)
			{
				p_fclose(fp);
				return false;
			}

			uchar *data = new uchar[aDataSize];

			int aReadSize = p_fread(data, 1, aDataSize, fp);
			p_fclose(fp);

			for (int i = 0; i < aDataSize; i++)
				((uchar*) data)[i] ^= anXor;

			if (aReadSize != aDataSize)
			{
				alDeleteBuffers(1, &buffer);
				delete [] data;
				return false;
			}

			alBufferData(buffer, format, data, aDataSize, aSampleRate);
			if (alGetError() != AL_NO_ERROR)
			{
				alDeleteBuffers(1, &buffer);
				delete [] data;
				return false;
			}

			delete [] data;
			mSourceSounds[theSfxID] = buffer;
			mHasSourceSounds[theSfxID] = true;
			return true;
		}

		p_fseek(fp, aCurPos+aChunkSize, SEEK_SET);
	}

	return false;
}

#ifdef USE_OGG_LIB

static int p_fseek64_wrap(PFILE *f,ogg_int64_t off,int whence)
{
	if (!f)
		return -1;

	return p_fseek(f, (long)off, whence);
}

int ov_pak_open(PFILE *f,OggVorbis_File *vf,char *initial,long ibytes)
{
	ov_callbacks callbacks = {
		(size_t (*)(void *, size_t, size_t, void *))  p_fread,
		(int (*)(void *, ogg_int64_t, int))           p_fseek64_wrap,
		(int (*)(void *))                             p_fclose,
		(long (*)(void *))                            p_ftell
	};

	return ov_open_callbacks((void *)f, vf, initial, ibytes, callbacks);
}

bool OpenALSoundManager::LoadOGGSound(unsigned int theSfxID, const std::string& theFilename)
{
	OggVorbis_File vf;
	int current_section;

	PFILE *aFile = p_fopen(theFilename.c_str(),"rb");
	if (!aFile)
		return false;

	if (ov_pak_open(aFile, &vf, NULL, 0) < 0)
	{
		p_fclose(aFile);
		return false;
	}

	vorbis_info *anInfo = ov_info(&vf,-1);

	int frameSize = anInfo->channels * 16 / 8;
	int aLenBytes = (int) (ov_pcm_total(&vf,-1) * frameSize);

	mSourceDataSizes[theSfxID] = aLenBytes;

	ALenum format = 0;
	if (anInfo->channels == 1)
		format = AL_FORMAT_MONO16;
	else if (anInfo->channels == 2)
		format = AL_FORMAT_STEREO16;
	if (format == 0)
	{
		ov_clear(&vf);
		return false;
	}

	ALuint buffer;

	alGetError();
	alGenBuffers(1, &buffer);
	if (alGetError() != AL_NO_ERROR)
	{
		ov_clear(&vf);
		return false;
	}
	printf("OpenAL: allocate buffer: %u\n", buffer);

	char *aBuf = new char[aLenBytes];
	char *aPtr = aBuf;
	int aNumBytes = aLenBytes;
	while(aNumBytes > 0)
	{
		long ret = ov_read(&vf, aPtr, aNumBytes, &current_section);
		if (ret == 0)
			break;
		else if (ret < 0)
			break;
		else
		{
			aPtr += ret;
			aNumBytes -= ret;
		}
	}

	alGetError();
	alBufferData(buffer, format, aBuf, aLenBytes, anInfo->rate);
	if (aNumBytes != 0 || alGetError() != AL_NO_ERROR)
	{
		alDeleteBuffers(1, &buffer);
		delete [] aBuf;
		ov_clear(&vf);
		return false;
	}

	delete [] aBuf;
	ov_clear(&vf);

	mSourceSounds[theSfxID] = buffer;
	mHasSourceSounds[theSfxID] = true;
	return true;
}
#else
bool OpenALSoundManager::LoadOGGSound(unsigned int theSfxID, const std::string& theFilename)
{
	return false;
}
#endif


bool OpenALSoundManager::LoadAUSound(unsigned int theSfxID, const std::string& theFilename)
{
	PFILE* fp;

	fp = p_fopen(theFilename.c_str(), "rb");
	if (!fp)
		return false;

	char aHeaderId[5];
	aHeaderId[4] = '\0';
	p_fread(aHeaderId, 1, 4, fp);
	if (!strcmp(aHeaderId, ".snd") == 0)
	{
		p_fclose(fp);
		return false;
	}

	ulong aHeaderSize;
	p_fread(&aHeaderSize, 4, 1, fp);
	aHeaderSize = LONG_BIGE_TO_NATIVE(aHeaderSize);

	ulong aDataSize;
	p_fread(&aDataSize, 4, 1, fp);
	aDataSize = LONG_BIGE_TO_NATIVE(aDataSize);

	ulong anEncoding;
	p_fread(&anEncoding, 4, 1, fp);
	anEncoding = LONG_BIGE_TO_NATIVE(anEncoding);

	ulong aSampleRate;
	p_fread(&aSampleRate, 4, 1, fp);
	aSampleRate = LONG_BIGE_TO_NATIVE(aSampleRate);

	ulong aChannelCount;
	p_fread(&aChannelCount, 4, 1, fp);
	aChannelCount = LONG_BIGE_TO_NATIVE(aChannelCount);

	p_fseek(fp, aHeaderSize, SEEK_SET);

	bool ulaw = false;

	ulong aSrcBitCount = 8;
	ulong aBitCount = 16;
	switch (anEncoding)
	{
	case 1:
		aSrcBitCount = 8;
		aBitCount = 16;
		ulaw = true;
		break;
	case 2:
		aSrcBitCount = 8;
		aBitCount = 8;
		break;

	/*
	Support these formats?

	case 3:
		aBitCount = 16;
		break;
	case 4:
		aBitCount = 24;
		break;
	case 5:
		aBitCount = 32;
		break;*/

	default:
		return false;
	}


	ulong aDestSize = aDataSize * aBitCount/aSrcBitCount;
	mSourceDataSizes[theSfxID] = aDestSize;

	ALenum format = 0;
	if (aChannelCount == 1)
		format = AL_FORMAT_MONO16;
	else if (aChannelCount == 2)
		format = AL_FORMAT_STEREO16;
	if (format == 0)
	{
		p_fclose(fp);
		return false;
	}

	ALuint buffer;

	alGetError();
	alGenBuffers(1, &buffer);
	if (alGetError() != AL_NO_ERROR)
	{
		p_fclose(fp);
		return false;
	}

	uchar* aBuf = new uchar[aDestSize];
	uchar* aSrcBuffer = new uchar[aDataSize];

	int aReadSize = p_fread(aSrcBuffer, 1, aDataSize, fp);
	p_fclose(fp);

	if (ulaw)
	{
		short* aDestBuffer = (short*) aBuf;

		for (ulong i = 0; i < aDataSize; i++)
		{
			int ch = aSrcBuffer[i];

			int sign = (ch < 128) ? -1 : 1;
			ch = ch | 0x80;
			if (ch > 239)
				ch = ((0xF0 | 15) - ch) * 2;
			else if (ch > 223)
				ch = (((0xE0 | 15) - ch) * 4) + 32;
			else if (ch > 207)
				ch = (((0xD0 | 15) - ch) * 8) + 96;
			else if (ch > 191)
				ch = (((0xC0 | 15) - ch) * 16) + 224;
			else if (ch > 175)
				ch = (((0xB0 | 15) - ch) * 32) + 480;
			else if (ch > 159)
				ch = (((0xA0 | 15) - ch) * 64) + 992;
			else if (ch > 143)
				ch = (((0x90 | 15) - ch) * 128) + 2016;
			else if (ch > 128)
				ch = (((0x80 | 15) - ch) * 256) + 4064;
			else
				ch = 0xff;

			aDestBuffer[i] = sign * ch * 4;
		}
	}
	else
		memcpy(aBuf, aSrcBuffer, aDataSize);

	delete [] aSrcBuffer;
	delete [] aBuf;

	if (aReadSize != aDataSize)
		return false;

	alGetError();
	alBufferData(buffer, format, aBuf, aDestSize, aSampleRate);
	if (alGetError() != AL_NO_ERROR)
	{
		alDeleteBuffers(1, &buffer);
		delete [] aBuf;
		return false;
	}

	delete [] aBuf;

	mSourceSounds[theSfxID] = buffer;
	mHasSourceSounds[theSfxID] = true;
	return true;
}

bool OpenALSoundManager::LoadSound(unsigned int theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);

	// ignore it
	if (!mInitialized)
		return true;

	mSourceFileNames[theSfxID] = theFilename;

	std::string aFilename = theFilename;

	if (LoadWAVSound(theSfxID, aFilename + ".wav"))
		return true;

#ifdef USE_OGG_LIB
	if (LoadOGGSound(theSfxID, aFilename + ".ogg"))
		return true;
#endif

	if (aFilename.find(".au") >= 0 && LoadAUSound(theSfxID, aFilename))
		return true;

	if (LoadAUSound(theSfxID, aFilename + ".au"))
		return true;

	return false;
}

int OpenALSoundManager::LoadSound(const std::string& theFilename)
{
	int i;
	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceFileNames[i] == theFilename)
			return i;

	for (i = MAX_SOURCE_SOUNDS-1; i >= 0; i--)
	{
		if (!mHasSourceSounds[i])
		{
			if (!LoadSound(i, theFilename))
				return -1;
			else
				return i;
		}
	}

	return -1;
}

void OpenALSoundManager::ReleaseSound(unsigned int theSfxID)
{
	if (mHasSourceSounds[theSfxID])
	{
		alDeleteSources(1, &mSourceSounds[theSfxID]);
		mSourceSounds[theSfxID] = NULL;
		mSourceFileNames[theSfxID] = "";
		mHasSourceSounds[theSfxID] = false;
	}
}

int OpenALSoundManager::GetFreeSoundId()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		if (!mHasSourceSounds[i])
			return i;
	}

	return -1;
}

int OpenALSoundManager::GetNumSounds()
{
	int aCount = 0;
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		if (mHasSourceSounds[i])
			aCount++;
	}

	return aCount;
}

bool OpenALSoundManager::SetBaseVolume(unsigned int theSfxID, double theBaseVolume)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = theBaseVolume;
	return true;
}

bool OpenALSoundManager::SetBasePan(unsigned int theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}

SoundInstance* OpenALSoundManager::GetSoundInstance(unsigned int theSfxID)
{
	if (theSfxID > MAX_SOURCE_SOUNDS)
		return NULL;

	if (!mInitialized)
		return NULL;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return NULL;

	if (!mHasSourceSounds[theSfxID])
			return NULL;

	mPlayingSounds[aFreeChannel] = new OpenALSoundInstance(this, mSourceSounds[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

void OpenALSoundManager::ReleaseSounds()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		if (mHasSourceSounds[i])
		{
			alDeleteBuffers(1, &mSourceSounds[i]);
			mSourceSounds[i] = 0;
			mHasSourceSounds[i] = false;
		}
	}
}

void OpenALSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != NULL)
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
	}
}

void OpenALSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != NULL && mPlayingSounds[i]->IsReleased())
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = NULL;
		}
	}
}

void OpenALSoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != NULL)
		{
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
	}
}


double OpenALSoundManager::GetMasterVolume()
{
	return 1.0;
}

void OpenALSoundManager::SetMasterVolume(double theVolume)
{
}

void OpenALSoundManager::Flush()
{
}

void OpenALSoundManager::SetCooperativeWindow(HWND theHWnd, bool isWindowed)
{
}

OpenALManager* OpenALSoundManager::GetOpenALManager()
{
	return mManager;
}

class OpenALSoundDriver: public SoundDriver {
public:
	OpenALSoundDriver ()
		: SoundDriver("OpenALSound", 20),
		  mInitialized (false)
	{
	}

	void Init ()
	{
		if (mInitialized)
			return;

		mInitialized = true;
	}

	SoundManager* Create (SexyAppBase * theApp)
	{
		Init ();
		return new OpenALSoundManager ();
	}

	MusicInterface* CreateMusicInterface (SexyAppBase * theApp)
	{
		Init ();
		return new MusicInterface ();
	}
private:
	bool	 mInitialized;
};

static OpenALSoundDriver aOpenALSoundDriver;
SoundDriverRegistor aOpenALSoundDriverRegistor (&aOpenALSoundDriver);
SoundDriver* GetOpenALSoundDriver()
{
	return &aOpenALSoundDriver;
}
