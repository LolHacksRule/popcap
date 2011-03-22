#ifndef __GAME_LAUNCHER_H__
#define __GAME_LAUNCHER_H__

#include <string>
#include <vector>
#include <list>

#include <jni.h>

extern "C" {
  typedef int (*GameInitProc)(void);
  typedef int (*GameRenderProc)(void);
  typedef int (*GamePauseProc)(void);
  typedef int (*GameResumeProc)(void);
  typedef int (*GameUninitProc)(void);
  typedef void (*GameAudioReadCallback)(void*);
  struct AGEvent;
  typedef void (*AGEventListener)(const struct AGEvent* event,
				  void*                 data);
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
  void readAudioData();

  void release();

  GameState   getState() const;
  const char* getSourceDir() const;
  const char* getDataDir() const;
  const char* getFilesDir() const;
  int         getViewWidth() const;
  int         getViewHeight() const;
  bool        gameLoaded() const;
  JNIEnv*     getJNIEnv();

  int         audioInit(int sampleRate,
                        int channels,
                        int bits);
  int         audioUninit();

  int         audioSetReadCallback(GameAudioReadCallback callback,
                                   void* data = 0);
  void        audioWriteData(void* data, size_t size);

  void        queueKeyEvent(int     down,
			    long    time,
			    int     keycode,
			    int     keychar);

  void        queuePointerEvent(int     id,
				int     action,
				long    time,
				int     flags,
				float   x,
				float   y,
				float   pressure);

  void        addEventListener(AGEventListener listener,
			       void* data);

 private:
  void        setupEnv();
  bool        loadGame();
  void        unloadGame();
  void        dispatchEvent(AGEvent &event);

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
  GameAudioReadCallback mAudioReadCallback;
  void*                 mAudioReadCallbackData;

  struct EventListener {
    AGEventListener callback;
    void*           data;
  };
  std::list<EventListener> mListeners;
};

#endif
