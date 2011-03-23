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

#include "GameLauncher.h"

#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#define  LOG_TAG    "libGameLauncher/JNI"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

static GameLauncher* launcher = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

JNIEXPORT void JNICALL Java_org_jinghua_GameJni_init(JNIEnv * env, jclass obj,
						   jstring sourcedir,
						   jstring datadir,
						   jstring filesdir,
						   jobject view,
						   jint width, jint height)
{
    const char *sdir = env->GetStringUTFChars(sourcedir, 0);
    const char *ddir = env->GetStringUTFChars(datadir, 0);
    const char *fdir = env->GetStringUTFChars(filesdir, 0);

    pthread_mutex_lock(&mutex);

    launcher = GameLauncher::getInstance();
    LOGI("Init(%s, %s, %s, %d, %d)", sdir, ddir, fdir, width, height);
    launcher->init(env, sdir, ddir, fdir, view, width, height);

    pthread_mutex_unlock(&mutex);

    env->ReleaseStringUTFChars(sourcedir, sdir);
    env->ReleaseStringUTFChars(datadir, ddir);
    env->ReleaseStringUTFChars(filesdir, fdir);
}

JNIEXPORT jboolean JNICALL Java_org_jinghua_GameJni_render(JNIEnv * env, jclass obj)
{
    jboolean ret = JNI_FALSE;

    pthread_mutex_lock(&mutex);

    if (launcher)
        ret = launcher->render();

    pthread_mutex_unlock(&mutex);

    return ret;
}

JNIEXPORT void JNICALL Java_org_jinghua_GameJni_pause(JNIEnv * env, jclass obj)
{
    if (launcher)
        launcher->pause();
}

JNIEXPORT void JNICALL Java_org_jinghua_GameJni_resume(JNIEnv * env, jclass obj)
{
    if (launcher)
        launcher->resume();
}

JNIEXPORT void JNICALL Java_org_jinghua_GameJni_readAudioData(JNIEnv * env, jclass obj)
{
    if (launcher)
        launcher->readAudioData();
}

JNIEXPORT void JNICALL Java_org_jinghua_GameJni_queueKeyEvent(JNIEnv * env,
							      jclass   obj,
							      jint     down,
							      jlong    time,
							      jint     keycode,
							      jint     keychar)
{
    if (launcher)
	launcher->queueKeyEvent(down, time, keycode, keychar);
}

JNIEXPORT void JNICALL Java_org_jinghua_GameJni_queuePointerEvent(JNIEnv * env,
								  jclass   obj,
								  jint     id,
								  jint     action,
								  jlong    time,
								  jint     flags,
								  jfloat   x,
								  jfloat   y,
								  jfloat   pressure)
{
    if (launcher)
	launcher->queuePointerEvent(id, action, time, flags, x, y, pressure);
}

JNIEXPORT void JNICALL Java_org_jinghua_GameJni_uninit(JNIEnv * env, jclass obj)
{
    pthread_mutex_lock(&mutex);

    if (launcher)
        launcher->release();
    launcher = 0;

    pthread_mutex_unlock(&mutex);
}

}
