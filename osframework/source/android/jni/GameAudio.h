#ifndef __GAME_AUDIO_H__
#define __GAME_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

int
AGAudioInit(int sampleRate, int channels, int bits);

int
AGAudioSetReadCallback(void (*callback)(void*), void* data);

int
AGAudioWrite(const void* data, int len);

int
AGAudioUninit(void);

#ifdef __cplusplus
}
#endif

#endif
