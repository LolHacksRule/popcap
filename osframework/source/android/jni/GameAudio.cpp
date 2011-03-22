#include "GameAudio.h"
#include "GameLauncher.h"

extern "C" int
AGAudioInit(int sampleRate, int channels, int bits)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        return launcher->audioInit(sampleRate, channels, bits);
}

extern "C" int
AGAudioUninit(void)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        return launcher->audioUninit();
}

extern "C" int
AGAudioSetReadCallback(void (*callback)(void*), void* data)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        return launcher->audioSetReadCallback(callback, data);
}

extern "C" int
AGAudioWrite(const void* data, int len)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        if (launcher->getState() < GAME_STATE_INITED)
            return -1;

        launcher->audioWriteData((void*)data, len);
        return len;
}


