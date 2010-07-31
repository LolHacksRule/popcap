#ifndef  __OPENALMANAGER_H__
#define __OPENALMANAGER_H__

#include <map>

#ifdef SEXY_IOS
#include <AudioToolbox/AudioToolbox.h>
#endif

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace Sexy {

	class OpenALManager {
	 public:
		OpenALManager();
		~OpenALManager();


	 public:
		bool initialize();
		void uninitialize();

		int allocSource();
		void freeSource(int id);

	 private:
		typedef std::map<ALuint, bool> ALSources;

		bool mInitialized;
		ALSources mSources;

		ALCcontext* mContext;
		ALCdevice* mDevice;

#ifdef SEXY_IOS
		void haltOpenALSession();
		void resumeOpenALSession();
		static void interruptionListenerCallback (void *inUserData,
							  UInt32 interruptionState);
#endif
	};
}
#endif
