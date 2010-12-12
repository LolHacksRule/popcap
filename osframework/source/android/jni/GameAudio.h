#ifndef __GAME_AUDIO_H__
#define __GAME_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

int
awAudioInit(int sampleRate, int channels, int bits);

int
awAudioSetReadCallback(void (*callback)(void*), void* data);

int
awAudioWrite(const void* data, int len);

int
awAudioUninit(void);

#ifdef __cplusplus
}
#endif

#endif
