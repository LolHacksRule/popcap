#ifndef __GAME_LAUNCHER_H__
#define __GAME_LAUNCHER_H__

#include <string>

#include <jni.h>

extern "C" {
  typedef int (*GameInitProc)(void);
  typedef int (*GameRenderProc)(void);
  typedef int (*GamePauseProc)(void);
  typedef int (*GameResumeProc)(void);
  typedef int (*GameUninitProc)(void);
};

enum GameState {
  GAME_STATE_INVALID,
  GAME_STATE_LOADED,
  GAME_STATE_INITED,
  GAME_STATE_RUNNING,
  GAME_STATE_PAUSED
};

class GameLauncher {
 public:
  static GameLauncher* getInstance();

 private:
  GameLauncher();

 public:
  bool init(JNIEnv*     env,
	    const char *sourcedir,
	    const char *datadir,
	    const char *filesdir,
	    int         width,
	    int         height);

  void pause();
  void resume();
  bool render();
  void uninit();

  void release();

  GameState   getState() const;
  const char* getSourceDir() const;
  const char* getDataDir() const;
  const char* getFilesDir() const;
  int         getViewWidth() const;
  int         getViewHeight() const;
  bool        gameLoaded() const;
  JNIEnv*     getJNIEnv();

 private:
  void        setupEnv();
  bool        loadGame();
  void        unloadGame();

 private:
  void*            mHandler;
  int              mWidth;
  int              mHeight;
  GameState        mState;
  JNIEnv*          mEnv;
  JavaVM*          mVM;
  std::string      mSourceDir;
  std::string      mDataDir;
  std::string      mFilesDir;
  GameInitProc     mInitProc;
  GameRenderProc   mRenderProc;
  GamePauseProc    mPauseProc;
  GameResumeProc   mResumeProc;
  GameUninitProc   mUninitProc;
};

#endif
