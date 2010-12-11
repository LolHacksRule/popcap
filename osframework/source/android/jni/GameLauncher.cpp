#include "GameLauncher.h"

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
    mHandler(0), mWidth(0), mHeight(0),
    mState(GAME_STATE_INVALID), mEnv(0), mVM(0)
{
}

bool GameLauncher::gameLoaded() const
{
    return mHandler != 0;
}

bool GameLauncher::init(JNIEnv*     env,
			const char *sourcedir,
			const char *datadir,
			const char *filesdir,
			int         width,
			int         height)
{
    if (gameLoaded())
	return false;

    LOGI("Application data directory: %s\n", datadir);
    LOGI("OpenGLView size: %dx%d\n", width, height);

    mainThread = pthread_self();
    mEnv = env;
    env->GetJavaVM(&mVM);
    mainVM = mVM;
    pthread_setspecific(vmTlsKey, env);

    mSourceDir = std::string(sourcedir);
    mDataDir = std::string(datadir);
    mFilesDir = std::string(filesdir);
    mWidth = width;
    mHeight = height;
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

    chdir(filesdir);

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
    mPauseProc = (GamePauseProc)dlsym(mHandler, "GameResume");
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

void GameLauncher::pause()
{
    if (!gameLoaded() || mState < GAME_STATE_INITED)
	return;

    if (mPauseProc)
	mPauseProc();
    mState = GAME_STATE_PAUSED;
}

void GameLauncher::resume()
{
    if (!gameLoaded() || mState < GAME_STATE_INITED)
	return;

    if (mResumeProc)
	mResumeProc();
    mState = GAME_STATE_RUNNING;
}

bool GameLauncher::render()
{
    if (!gameLoaded() || mState < GAME_STATE_INITED)
	return false;

    mState = GAME_STATE_RUNNING;
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

void GameLauncher::release()
{
    if (gameLoaded())
    {
	uninit();
	unloadGame();
	mVM = 0;
	mEnv = 0;
	mainVM = 0;
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
