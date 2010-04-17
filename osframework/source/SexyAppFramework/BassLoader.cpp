#include "BassLoader.h"
#include <stdlib.h>

using namespace Sexy;

BASS_INSTANCE* Sexy::gBass = NULL;
static long gBassLoadCount = 0;


// Channel attributes
#define BASS_ATTRIB_FREQ			1
#define BASS_ATTRIB_VOL				2
#define BASS_ATTRIB_PAN				3
#define BASS_ATTRIB_EAXMIX			4
#define BASS_ATTRIB_MUSIC_AMPLIFY	0x100
#define BASS_ATTRIB_MUSIC_PANSEP	0x101
#define BASS_ATTRIB_MUSIC_PSCALER	0x102
#define BASS_ATTRIB_MUSIC_BPM		0x103
#define BASS_ATTRIB_MUSIC_SPEED		0x104
#define BASS_ATTRIB_MUSIC_VOL_GLOBAL 0x105
#define BASS_ATTRIB_MUSIC_VOL_CHAN	0x200 // + channel #


// BASS_ChannelGetLength/GetPosition/SetPosition modes
#define BASS_POS_BYTE			0		// byte position
#define BASS_POS_MUSIC_ORDER	1		// order.row position, MAKELONG(order,row)
#define BASS_POS_DECODE			0x10000000 // flag: get the decoding (not playing) position

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool CheckBassFunction(void* theFunc, const char *theName)
{
	if (theFunc==0)
	{
		char aBuf[1024];
		sprintf(aBuf,"%s function not found in bass.dll",theName);
		MessageBoxA(NULL,aBuf,"Error",MB_OK | MB_ICONERROR);
	}

	return theFunc != 0;
}


static BOOL WINAPI
BASS24_ChannelSetAttributes(DWORD handle, int freq, int volume, int pan)
{
	if (!gBass->BASS_ChannelSetAttribute(handle, BASS_ATTRIB_FREQ, (float)freq))
		return FALSE;
	if (!gBass->BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, volume / 100.0f))
		return FALSE;
	if (!gBass->BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, (float)pan))
		return FALSE;
	return TRUE;
}

static BOOL WINAPI
BASS24_ChannelGetAttributes(DWORD handle, DWORD* freq, DWORD* volume, int* pan)
{
	float val;

	if (!gBass->BASS_ChannelGetAttribute(handle, BASS_ATTRIB_FREQ, &val))
		return FALSE;
	*freq = val;

	if (!gBass->BASS_ChannelGetAttribute(handle, BASS_ATTRIB_VOL, &val))
		return FALSE;
	*volume = val * 100;

	if (!gBass->BASS_ChannelGetAttribute(handle, BASS_ATTRIB_PAN, &val))
		return FALSE;
	*pan = val;

	return TRUE;
}


static BOOL WINAPI
BASS24_ChannelSetPosition(DWORD handle, QWORD pos)
{
	return gBass->BASS_ChannelSetPosition2(handle, pos, BASS_POS_BYTE);
}

static QWORD WINAPI
BASS24_ChannelGetPosition(DWORD handle)
{
	return gBass->BASS_ChannelGetPosition2(handle, BASS_POS_BYTE);
}

static DWORD WINAPI
BASS24_MusicGetOrders(HMUSIC handle)
{
	return 0;
}

static DWORD WINAPI
BASS24_MusicGetOrderPosition(HMUSIC handle)
{
	return gBass->BASS_ChannelGetPosition2(handle, BASS_POS_MUSIC_ORDER);
}

static DWORD WINAPI
BASS24_MusicGetAttribute(DWORD handle, DWORD attrib)
{
	return 0;
}

static void WINAPI
BASS24_MusicSetAttribute(DWORD handle, DWORD attrib,DWORD value)
{
}

static 	BOOL WINAPI
BASS24_ChannelSetFlags(DWORD handle, DWORD flags)
{
	return TRUE;
}

static BOOL WINAPI
BASS24_ChannelSlideAttributes(DWORD handle, int freq, int volume, int pan, DWORD time)
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BASS_INSTANCE::BASS_INSTANCE(const char *dllName)
{
	mOk = true;
	mModule = LoadLibrary(dllName);
	if (!mModule)
	{
		mOk = false;
		return;
	}

#define GETPROC(_x) if (!CheckBassFunction(*((void **)&_x) = (void*)GetProcAddress(mModule, #_x),#_x)) { mOk = false; return; }

	GETPROC(BASS_Init);
	GETPROC(BASS_Free);
	GETPROC(BASS_Stop);
	GETPROC(BASS_Start);

	*((void**) &BASS_SetGlobalVolumes) = (void*) GetProcAddress(mModule, "BASS_SetGlobalVolumes");
	*((void**) &BASS_SetVolume) = (void*) GetProcAddress(mModule, "BASS_SetVolume");

	if ((BASS_SetVolume == NULL) && (BASS_SetGlobalVolumes == NULL))
	{
		MessageBoxA(NULL,"Neither BASS_SetGlobalVolumes or BASS_SetVolume found in bass.dll","Error",MB_OK | MB_ICONERROR);
		mOk = false;
		return;
	}

	*((void**) &BASS_SetConfig) = (void*) GetProcAddress(mModule, "BASS_SetConfig");
	*((void**) &BASS_GetConfig) = (void*) GetProcAddress(mModule, "BASS_GetConfig");

	GETPROC(BASS_GetVolume);
	GETPROC(BASS_GetInfo);

	GETPROC(BASS_GetVersion);

	mVersion = BASS_GetVersion();

	GETPROC(BASS_ChannelStop);
	GETPROC(BASS_ChannelPlay);
	GETPROC(BASS_ChannelPause);
	if (mVersion < 0x204)
	{
		GETPROC(BASS_ChannelSetAttributes);
		GETPROC(BASS_ChannelGetAttributes);
	}
	else
	{

		GETPROC(BASS_ChannelSetAttribute);
		GETPROC(BASS_ChannelGetAttribute);
		BASS_ChannelGetAttributes = BASS24_ChannelGetAttributes;
		BASS_ChannelSetAttributes = BASS24_ChannelSetAttributes;
	}

	if (mVersion  < 0x204)
	{
		GETPROC(BASS_ChannelSetPosition);
		GETPROC(BASS_ChannelGetPosition);
	}
	else
	{
		*((void**) &BASS_ChannelSetPosition2) = (void*) GetProcAddress(mModule, "BASS_ChannelSetPosition");
		*((void**) &BASS_ChannelGetPosition2) = (void*) GetProcAddress(mModule, "BASS_ChannelGetPosition");
		BASS_ChannelSetPosition = BASS24_ChannelSetPosition;
		BASS_ChannelGetPosition = BASS24_ChannelGetPosition;
	}

	GETPROC(BASS_ChannelIsActive);

	if (mVersion < 0x204)
	{
		GETPROC(BASS_ChannelSetFlags);
		GETPROC(BASS_ChannelSlideAttributes);
	}
	else
	{
		BASS_ChannelSetFlags = BASS24_ChannelSetFlags;
		BASS_ChannelSlideAttributes = BASS24_ChannelSlideAttributes;
	}

	GETPROC(BASS_ChannelIsSliding);
	GETPROC(BASS_ChannelGetLevel);
	GETPROC(BASS_ChannelSetSync);
	GETPROC(BASS_ChannelRemoveSync);
	GETPROC(BASS_ChannelGetData);

	// supported by BASS 1.1 and higher. Only work if the user has DX8 or higher though.
	GETPROC(BASS_FXSetParameters);
	GETPROC(BASS_FXGetParameters);
	GETPROC(BASS_ChannelSetFX);
	GETPROC(BASS_ChannelRemoveFX);

	GETPROC(BASS_MusicLoad);
	GETPROC(BASS_MusicFree);
	if (mVersion < 0x204)
	{
		GETPROC(BASS_MusicGetAttribute);
		GETPROC(BASS_MusicSetAttribute);
		GETPROC(BASS_MusicGetOrders);
		GETPROC(BASS_MusicGetOrderPosition);
	}
	else
	{
		
		BASS_MusicGetOrders = BASS24_MusicGetOrders;
		BASS_MusicGetOrderPosition = BASS24_MusicGetOrderPosition;
		BASS_MusicGetAttribute = BASS24_MusicGetAttribute;
		BASS_MusicSetAttribute = BASS24_MusicSetAttribute;
	}

	GETPROC(BASS_StreamCreateFile);
	GETPROC(BASS_StreamFree);

	GETPROC(BASS_SampleLoad);
	GETPROC(BASS_SampleFree);
	GETPROC(BASS_SampleSetInfo);
	GETPROC(BASS_SampleGetInfo);
	GETPROC(BASS_SampleGetChannel);
	GETPROC(BASS_SampleStop);

	GETPROC(BASS_ErrorGetCode);

	mVersion2 = BASS_SetConfig != NULL;
	if (mVersion2)
	{
		// Version 2 has different BASS_Init params
		*((void**) &BASS_Init2) = (void*) BASS_Init;
		BASS_Init = NULL;

		*((void**) &BASS_MusicLoad2) = (void*) BASS_MusicLoad;
		BASS_MusicLoad = NULL;

		// The following are only supported in 2.2 and higher
		*((void**) &BASS_PluginLoad) = (void*) GetProcAddress(mModule, "BASS_PluginLoad");
		*((void**) &BASS_ChannelGetLength) = (void*) GetProcAddress(mModule, "BASS_ChannelGetLength");

		// 2.1 and higher only
		*((void**) &BASS_ChannelPreBuf) = (void*) GetProcAddress(mModule, "BASS_ChannelPreBuf");
	}
	else
	{
		BASS_PluginLoad = NULL;
		BASS_ChannelPreBuf = NULL;
	}

#undef GETPROC
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BASS_INSTANCE::~BASS_INSTANCE()
{
    if (mModule)
        FreeLibrary(mModule);
}


BOOL BASS_INSTANCE::BASS_MusicSetAmplify(HMUSIC handle, DWORD amp)
{
	BASS_MusicSetAttribute(handle, BASS_MUSIC_ATTRIB_AMPLIFY, amp);
	return true;
}


BOOL BASS_INSTANCE::BASS_MusicPlay(HMUSIC handle)
{
	return BASS_ChannelPlay(handle, true);
}


BOOL BASS_INSTANCE::BASS_MusicPlayEx(HMUSIC handle, DWORD pos, int flags, BOOL reset)
{
	int anOffset = MAKEMUSICPOS(pos,0);

	BASS_ChannelStop(handle);
	BASS_ChannelSetPosition(handle, anOffset);
	BASS_ChannelSetFlags(handle, flags);

	return BASS_ChannelPlay(handle, false/*reset*/);
}


BOOL BASS_INSTANCE::BASS_ChannelResume(DWORD handle)
{
	return BASS_ChannelPlay(handle, false);
}

BOOL BASS_INSTANCE::BASS_StreamPlay(HSTREAM handle, BOOL flush, DWORD flags)
{
	BASS_ChannelSetFlags(handle, flags);
	return BASS_ChannelPlay(handle, flush);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool Sexy::LoadBassDLL()
{
	static bool ok = true;
	if (!ok)
		return false;

	InterlockedIncrement(&gBassLoadCount);
	if (gBass!=NULL)
		return true;

	gBass = new BASS_INSTANCE(".\\bass.dll");
	if (gBass->mModule==NULL || !gBass->mOk)
	{
		if (!gBass->mModule && false)
			MessageBoxA(NULL,"Can't find bass.dll." ,"Error",MB_OK | MB_ICONERROR);
		delete gBass;
		gBass = 0;
		ok = false;
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Sexy::FreeBassDLL()
{
	if (gBass!=NULL)
	{
		if (InterlockedDecrement(&gBassLoadCount) <= 0)
		{
			delete gBass;
			gBass = NULL;
		}
	}
}
