#include "GameLauncher.h"
#include "GameView.h"

#include <algorithm>
#include <vector>

#include <jni.h>
#include <android/log.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define  LOG_TAG    "libGameLauncher"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static GameLauncher* gameLauncher = 0;
static pthread_key_t vmTlsKey;
static pthread_t     mainThread;
static JavaVM*       mainVM;

static void
DettachJavaVM(void* arg)
{
    JavaVM* vm = (JavaVM*)arg;

    LOGI("Detaching thread %ld from JavaVM", (long)pthread_self());

    if (vm)
        vm->DetachCurrentThread();
}

GameLauncher* GameLauncher::getInstance()
{
    if (gameLauncher)
        return gameLauncher;
    gameLauncher = new GameLauncher();
    pthread_key_create(&vmTlsKey, DettachJavaVM);

    return gameLauncher;
}

GameLauncher::GameLauncher() :
    mHandler(0), mView(0), mWidth(0), mHeight(0),
    mState(GAME_STATE_INVALID), mEnv(0), mVM(0)
{
    mDirty = false;
    mAudioReadCallback = 0;
}

bool GameLauncher::gameLoaded() const
{
    return mHandler != 0;
}

bool GameLauncher::init(JNIEnv*     env,
                        const char *sourcedir,
                        const char *datadir,
                        const char *filesdir,
			jobject     view,
                        int         width,
                        int         height)
{
    if (gameLoaded())
    {
	mWidth = width;
	mHeight = height;
        return false;
    }

    LOGI("Application data directory: %s\n", datadir);
    LOGI("OpenGLView size: %dx%d\n", width, height);

    chdir(filesdir);

    mainThread = pthread_self();
    mEnv = env;
    env->GetJavaVM(&mVM);
    mainVM = mVM;
    pthread_setspecific(vmTlsKey, env);

    mSourceDir = std::string(sourcedir);
    mDataDir = std::string(datadir);
    mFilesDir = std::string(filesdir);
    mView = (jobject)env->NewGlobalRef(view);
    mWidth = width;
    mHeight = height;
    mAudioReadCallback = 0;
    if (!loadGame())
        return false;

    char buf[128];

    snprintf(buf, sizeof(buf), "ANDROID_GLVIEW_WIDTH=%d", width);
    putenv(buf);
    snprintf(buf, sizeof(buf), "ANDROID_GLVIEW_HEIGHT=%d", height);
    putenv(buf);

    std::string path;

    path = std::string("ANDROID_SOURCE_DIR=") + mSourceDir;
    putenv(path.c_str());

    path = std::string("ANDROID_DATA_DIR=") + mFilesDir;
    putenv(path.c_str());

    path = std::string("ANDROID_FILES_DIR=") + mFilesDir;
    putenv(path.c_str());

    mInitProc();
    mState = GAME_STATE_INITED;
    return true;
}

void split(const std::string& s, const std::string& delim, std::vector<std::string>& ret)
{
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);

    while (index != std::string::npos)
    {
        ret.push_back(s.substr(last, index-last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0)
    {
        ret.push_back(s.substr(last, index-last));
    }
}

void GameLauncher::setupEnv()
{
    std::string path;

    if (!mDataDir.empty())
        path = mDataDir + "/lib";
    else
        path.clear();

    const char* oldpath = getenv("LD_LIBRARY_PATH");
    std::string newpath;

    if (oldpath)
    {
        std::vector<std::string> paths;

        split(std::string(oldpath), ":", paths);
        if (!path.empty() &&
            std::find(paths.begin(),
                     paths.end(), path) == paths.end())
            paths.insert(paths.begin(), path);
        if (std::find(paths.begin(), paths.end(), ".") == paths.end())
            paths.insert(paths.begin(), std::string("."));
        for (size_t i = 0; i < paths.size(); i++)
        {
            if (i != 0)
                newpath += std::string(":");
            newpath += paths[i];
        }
        if (newpath == std::string(oldpath))
            return;
    }
    else
    {
        newpath = std::string(".:") + oldpath;
    }

    std::string ldpath = "LD_LIBRARY_PATH=" + newpath;
    putenv(ldpath.c_str());

    LOGI("Setting LD_LIBRARY_PATH to %s\n", newpath.c_str());
}

bool GameLauncher::loadGame()
{
    std::string path = mDataDir;
    std::string name = "libGameMain.so";

    setupEnv();
    if (!path.empty())
        path = path + "/lib/" + name;
    else
        path = name;

    LOGI("Loading game module: %s\n", path.c_str());
    mHandler = dlopen(path.c_str(), RTLD_LOCAL | RTLD_NOW);
    if (!mHandler)
    {
        LOGE("Couldn't load game module: %s\n", dlerror());
        return false;
    }

    mInitProc = (GameInitProc)dlsym(mHandler, "GameInit");
    mRenderProc = (GameRenderProc)dlsym(mHandler, "GameRender");
    mPauseProc = (GamePauseProc)dlsym(mHandler, "GamePause");
    mResumeProc = (GameResumeProc)dlsym(mHandler, "GameResume");
    mUninitProc = (GameUninitProc)dlsym(mHandler, "GameUninit");
    if (!mInitProc || !mUninitProc || !mRenderProc)
    {
        LOGE("Couldn't find some required symbols in game.");
        dlclose(mHandler);
        mHandler = 0;
        return false;
    }
    mState = GAME_STATE_LOADED;
    return true;
}

void GameLauncher::unloadGame()
{
    LOGI("Unloading game module.");
    if (mHandler)
        dlclose(mHandler);
    mHandler = 0;
}

bool GameLauncher::pause()
{
    LOGI("Pausing game module.");

    if (!gameLoaded() || mState < GAME_STATE_INITED)
        return false;

    if (!mPauseProc)
	return false;

    if (mPauseProc())
	return false;

    mState = GAME_STATE_PAUSED;
    LOGI("Paused");
    return true;
}

bool GameLauncher::resume()
{
    LOGI("Resuming game module.");

    if (!gameLoaded() || mState < GAME_STATE_INITED)
        return false;

    if (mResumeProc)
        mResumeProc();
    mState = GAME_STATE_RUNNING;
    mDirty = true;
    LOGI("Resumed");
    return true;
}

bool GameLauncher::render()
{
    if (!gameLoaded() || mState < GAME_STATE_INITED)
        return false;

    mState = GAME_STATE_RUNNING;
    if (mDirty)
    {
	mDirty = false;

	AGEvent evt;

	evt.type = AG_VIEW_CHANGED_EVENT;
	evt.flags = 0;
	evt.timestamp = 0;
	evt.u.view.width = mWidth;
	evt.u.view.height = mHeight;
	dispatchEvent(evt);
    }
    if (mRenderProc())
        return true;
    return false;
}

void GameLauncher::uninit()
{
    if (!gameLoaded())
        return;

    LOGI("Unintializing game...");
    mUninitProc();
    mState = GAME_STATE_LOADED;
    LOGI("Game unitialized...");
}

int GameLauncher::audioInit(int sampleRate,
                            int channels,
                            int bits)
{
    JNIEnv* env = 0;

    LOGI("Initializing audio player...");
    if (!mVM)
        return -1;

    mVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (!env)
        return -1;

    if (env->ExceptionOccurred())
        return -1;

    // Get the asset manager
    jclass cls = env->FindClass("org/jinghua/GameAudio");
    if (cls == NULL)
        return -1;

    // Find methods to be called
    jmethodID initAudio_method =
        env->GetMethodID(cls, "initAudio", "(III)Z");
    jmethodID getInstance_method =
        env->GetStaticMethodID(cls, "getInstance", "()Lorg/jinghua/GameAudio;");
    if (initAudio_method == NULL || getInstance_method == NULL)
    {
        env->DeleteLocalRef(cls);
        return -1;
    }

    // Get the instance of the manager
    jobject obj = env->CallStaticObjectMethod(cls, getInstance_method);
    if (obj == NULL)
    {
        env->DeleteLocalRef(cls);
        return -1;
    }

    // Call class method
    jboolean result = env->CallBooleanMethod(obj, initAudio_method,
                                             sampleRate, channels, bits);

    env->DeleteLocalRef(cls);

    LOGI("Audio Playre initialized.");

    return result == JNI_TRUE ? 0 : -1;
}

int GameLauncher::audioUninit()
{
    JNIEnv* env = 0;

    LOGI("Uninitializing audio player...");
    if (!mVM)
        return -1;

    mVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (!env)
        return -1;

    if (env->ExceptionOccurred())
        return -1;

    // Get the asset manager
    jclass cls = env->FindClass("org/jinghua/GameAudio");
    if (cls == NULL)
        return -1;

    // Find methods to be called
    jmethodID uninitAudio_method =
        env->GetMethodID(cls, "uninitAudio", "()V");
    jmethodID getInstance_method =
        env->GetStaticMethodID(cls, "getInstance", "()Lorg/jinghua/GameAudio;");
    if (uninitAudio_method == NULL || getInstance_method == NULL)
    {
        env->DeleteLocalRef(cls);
        return -1;
    }

    // Get the instance of the manager
    jobject obj = env->CallStaticObjectMethod(cls, getInstance_method);
    if (obj == NULL)
    {
        env->DeleteLocalRef(cls);
        return -1;
    }

    // Call class method
    env->CallVoidMethod(obj, uninitAudio_method);

    env->DeleteLocalRef(cls);

    LOGI("Audio Player uninitialized.");

    return 0;
}

void GameLauncher::audioWriteData(void* data, size_t size)
{
    JNIEnv* env = 0;

    //LOGI("Writing audio data...");
    if (!mVM)
        return;

    mVM->GetEnv((void**)&env, JNI_VERSION_1_4);

    if (!env)
        return;

    if (env->ExceptionOccurred())
        return;

    // Get the asset manager
    jclass cls = env->FindClass("org/jinghua/GameAudio");
    if (cls == NULL)
        return;

    // Find methods to be called
    jmethodID writeData_method =
        env->GetMethodID(cls, "writeData", "(Ljava/nio/ByteBuffer;II)V");
    jmethodID getInstance_method =
        env->GetStaticMethodID(cls, "getInstance", "()Lorg/jinghua/GameAudio;");
    if (writeData_method == NULL || getInstance_method == NULL)
    {
        env->DeleteLocalRef(cls);
        return;
    }

    // Get the instance of the manager
    jobject obj = env->CallStaticObjectMethod(cls, getInstance_method);
    if (obj == NULL)
    {
        env->DeleteLocalRef(cls);
        return;
    }

    jobject buf = env->NewDirectByteBuffer(data, size);

    // Call class method
    env->CallVoidMethod(obj, writeData_method, buf, 0, size);

    env->DeleteLocalRef(buf);
    env->DeleteLocalRef(cls);

    //LOGI("Writed audio data %p size %zd...", data, size);
}

int GameLauncher::audioSetReadCallback(GameAudioReadCallback callback,
                                       void*                 data)
{
    LOGI("Set audio read callback to %p:%p.", callback, data);
    mAudioReadCallback = callback;
    mAudioReadCallbackData = data;
    return 0;
}

void GameLauncher::readAudioData()
{
    if (!mAudioReadCallback)
        return;
    mAudioReadCallback(mAudioReadCallbackData);
}

void GameLauncher::dispatchEvent(AGEvent &event)
{
    std::list<EventListener>::iterator it = mListeners.begin();

    for (; it != mListeners.end(); ++it)
	it->callback(&event, it->data);
}

void GameLauncher::queueKeyEvent(int     down,
				 long    time,
				 int     keycode,
				 int     keychar)
{
    AGEvent evt;

    evt.type = down ? AG_KEY_DOWN_EVENT : AG_KEY_UP_EVENT;
    evt.flags = 0;
    evt.timestamp = time;
    evt.u.key.keyCode = keycode;
    evt.u.key.keyChar = keychar;
    dispatchEvent(evt);
}

void GameLauncher::queuePointerEvent(int     id,
				     int     action,
				     long    time,
				     int     flags,
				     float   x,
				     float   y,
				     float   pressure)
{
    AGEvent evt;

    switch (action)
    {
    case 0:
    case 5:
	evt.type = AG_POINTER_DOWN_EVENT;
	break;
    case 1:
    case 6:
	evt.type = AG_POINTER_UP_EVENT;
	break;
    case 2:
	evt.type = AG_POINTER_MOVE_EVENT;
	break;
    case 3:
	evt.type = AG_POINTER_CANCEL_EVENT;
	break;
    default:
	return;
    }

    evt.flags = flags;
    evt.timestamp = time;
    evt.u.pointer.id = id;
    evt.u.pointer.x = x;
    evt.u.pointer.y = y;
    evt.u.pointer.pressure = pressure;
    dispatchEvent(evt);
}

void GameLauncher::addEventListener(AGEventListener listener,
				    void* data)
{
    mListeners.push_back(EventListener());

    EventListener& back = mListeners.back();
    back.callback = listener;
    back.data = data;
}

void GameLauncher::viewSwapBuffers()
{
    LOGI("view swapBuffers...");

    JNIEnv* env = 0;

    if (!mVM || !mView)
        return;

    mVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (!env)
        return;

    if (env->ExceptionOccurred())
        return;

    // Get the asset manager
    jclass cls = env->FindClass("org/jinghua/GameView");
    if (cls == NULL)
        return;

    // Find methods to be called
    jmethodID swapBuffers_method = env->GetMethodID(cls, "swapBuffers", "()V");
    if (swapBuffers_method == NULL)
    {
        env->DeleteLocalRef(cls);
        return;
    }

    // Call class method
    env->CallVoidMethod(mView, swapBuffers_method);
    env->DeleteLocalRef(cls);

    LOGI("done");
}

void GameLauncher::viewUpdate()
{
    JNIEnv* env = 0;

    LOGI("updating view...");

    if (!mVM || !mView)
        return;

    mVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (!env)
        return;

    if (env->ExceptionOccurred())
        return;

    // Get the asset manager
    jclass cls = env->FindClass("org/jinghua/GameView");
    if (cls == NULL)
        return;

    // Find methods to be called
    jmethodID update_method =  env->GetMethodID(cls, "update", "()V");
    if (update_method == NULL)
    {
        env->DeleteLocalRef(cls);
        return;
    }

    // Call class method
    env->CallVoidMethod(mView, update_method);

    env->DeleteLocalRef(cls);

    LOGI("done");
}

void GameLauncher::viewShowKeyboard(int mode,
				    const char *title,
				    const char *hint,
				    const char *initial)
{
    JNIEnv* env = 0;

    LOGI("show keyboard...");
    if (!mVM || !mView)
        return;

    mVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (!env)
        return;

    if (env->ExceptionOccurred())
        return;

    jclass cls = env->FindClass("org/jinghua/GameView");
    if (cls == NULL)
        return;

    jmethodID method =
	env->GetMethodID(cls, "showKeyboard",
			 "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    if (method == NULL)
    {
        env->DeleteLocalRef(cls);
        return;
    }

    env->CallVoidMethod(mView, method, mode,
			env->NewStringUTF(title ? title : ""),
			env->NewStringUTF(hint ? hint : ""),
			env->NewStringUTF(initial ? initial : ""));

    env->DeleteLocalRef(cls);
}

void GameLauncher::viewHideKeyboard()
{
    LOGI("hide keyboard");
    JNIEnv* env = 0;

    if (!mVM || !mView)
        return;

    mVM->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (!env)
        return;

    if (env->ExceptionOccurred())
        return;

    jclass cls = env->FindClass("org/jinghua/GameView");
    if (cls == NULL)
        return;

    jmethodID method = env->GetMethodID(cls, "hideKeyboard", "()V");
    if (method == NULL)
    {
        env->DeleteLocalRef(cls);
        return;
    }

    env->CallVoidMethod(mView, method);

    env->DeleteLocalRef(cls);
}

void GameLauncher::textInput(const char *str)
{
    if (!str)
	mTextInput.clear();
    else
	mTextInput = str;

    AGEvent evt;

    evt.type = AG_TEXT_INPUT_EVENT;
    evt.flags = 0;
    evt.timestamp = 0;
    dispatchEvent(evt);
}

void GameLauncher::release()
{
    if (gameLoaded())
    {
        mAudioReadCallback = 0;
	mListeners.clear();
        audioUninit();
        uninit();
        unloadGame();
	if (mEnv && mView)
	    mEnv->DeleteLocalRef(mView);
        mVM = 0;
        mEnv = 0;
        mainVM = 0;
	mView = 0;
    }

    if (gameLauncher == this)
    {
        gameLauncher = 0;
        pthread_key_delete(vmTlsKey);
    }
    delete this;
}

const char* GameLauncher::getSourceDir() const
{
    return mSourceDir.c_str();
}

const char* GameLauncher::getDataDir() const
{
    return mDataDir.c_str();
}

const char* GameLauncher::getFilesDir() const
{
    return mFilesDir.c_str();
}

const char* GameLauncher::getTextInput() const
{
    return mTextInput.c_str();
}

int GameLauncher::getViewWidth() const
{
    return mWidth;
}

int GameLauncher::getViewHeight() const
{
    return mHeight;
}

JNIEnv* GameLauncher::getJNIEnv()
{
    if (!mVM)
        return 0;

    if (mainThread == pthread_self())
        return mEnv;

    return 0;
    JNIEnv* env = (JNIEnv*)pthread_getspecific(vmTlsKey);

    if (env)
    {
        if (env->ExceptionOccurred())
            env->ExceptionDescribe();
        return env;
    }

    mVM->AttachCurrentThread(&env, NULL);
    pthread_setspecific(vmTlsKey, env);
    LOGI("Attached thread %ld to JavaVM, evn %p", (long)pthread_self(), env);
    return env;
}

GameState GameLauncher::getState() const
{
    return mState;
}
