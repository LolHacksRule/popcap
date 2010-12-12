#include "GameAudio.h"
#include "GameLauncher.h"

extern "C" int
awAudioInit(int sampleRate, int channels, int bits)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        return launcher->audioInit(sampleRate, channels, bits);
}

extern "C" int
awAudioUninit(void)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        return launcher->audioUninit();
}

extern "C" int
awAudioSetReadCallback(void (*callback)(void*), void* data)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        return launcher->audioSetReadCallback(callback, data);
}

extern "C" int
awAudioWrite(const void* data, int len)
{
        GameLauncher* launcher = GameLauncher::getInstance();

        if (launcher->getState() < GAME_STATE_INITED)
            return -1;

        launcher->audioWriteData((void*)data, len);
        return len;
}


