/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.jinghua;

import org.jinghua.GameJni;

import java.nio.ByteBuffer;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

public class GameAudio {
    static String TAG = "GameAudio";

    private static GameAudio mInst = new GameAudio();

    public static GameAudio getInstance() {
        return mInst;
    }

    public boolean initAudio(int sampleRate, int channels,
                             int bits)
    {
        if (mAudioTrack != null)
            return false;

        if (sampleRate != 44100)
            return false;
        if (channels != 2)
            return false;
        if (bits != 16)
            return false;

        int channelCfg = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        int bufferSize = AudioTrack.getMinBufferSize(sampleRate,
                                                     channelCfg,
                                                     audioFormat);

	Log.d(TAG, String.format("sampleRate %d channels %d bits %d bufferSize %d",
				 sampleRate, channels, bits, bufferSize));
        mAudioTrack = new GameAudioTrack(AudioManager.STREAM_MUSIC,
                                         sampleRate,
                                         channelCfg,
                                         audioFormat,
                                         bufferSize,
                                         AudioTrack.MODE_STREAM);
        mAudioTrack.play();
        mAudioTrack.setPlaybackPositionUpdateListener(new AudioTrack.OnPlaybackPositionUpdateListener()
            {
                @Override
                public void onMarkerReached(AudioTrack track) {
                }

                @Override
                public void onPeriodicNotification(AudioTrack track) {
                    GameJni.readAudioData();
                }
            });
        mAudioTrack.setPositionNotificationPeriod(1024);

        return true;
    }

    public void uninitAudio()
    {
        if (mAudioTrack == null)
            return;

        mAudioTrack.stop();
        mAudioTrack.release();
        mAudioTrack = null;
    }

    public void writeData(ByteBuffer data, int offset, int len)
    {
        if (mAudioTrack == null)
            return;

        data.position(offset);
        data.get(mAudioData, 0, len);
        mAudioTrack.write(mAudioData, 0, len);
    }

    public void pause()
    {
        if (mAudioTrack != null)
            mAudioTrack.pause();
    }

    public void resume()
    {
        if (mAudioTrack != null)
            mAudioTrack.play();
    }

    private GameAudioTrack mAudioTrack;
    byte[] mAudioData = new byte[8192];
}

class GameAudioTrack extends AudioTrack
{
    public GameAudioTrack(int streamType, int sampleRateInHz,
                          int channelConfig, int audioFormat,
                          int bufferSizeInBytes, int mode)
        throws IllegalArgumentException {
        super(streamType, sampleRateInHz, channelConfig, audioFormat,
              bufferSizeInBytes, mode);

        if(audioFormat == AudioFormat.ENCODING_PCM_16BIT)
            mFrameSize = 2 * this.getChannelCount();
        else
            mFrameSize = this.getChannelCount();
    }

    @Override
    public void play() throws IllegalStateException {
        super.play();
        initBuffer();
    }

    public void initBuffer()
    {
        byte[] audioData = new byte[getNativeFrameCount() * mFrameSize];
        write(audioData, 0, audioData.length);
    }

    private int mFrameSize;
}