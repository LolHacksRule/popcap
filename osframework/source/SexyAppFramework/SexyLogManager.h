#ifndef __SEXY_LOG_MANAGER_H__
#define __SEXY_LOG_MANAGER_H__

#include "SexyLogListener.h"
#include "CritSect.h"

namespace Sexy {

	class MultiplexLogListener;
	class LogManager {
	 private:
		LogManager();
		~LogManager();

	 public:
		static LogManager& getInstance();
		static void release();

		void close();
		void silent();
		void setListener(LogListener* listener);
		void log(LogLevel lvl, const std::string& tag, const std::string& s);
		void setVerboseLevel(LogLevel lvl);
		LogLevel getVerboseLevel();
		LogLevel getVerboseLevel(const std::string& tag);
		std::string getDefaultTag();
		void setDefaultTag(const std::string& tag);
		std::string getName();
		void setName(const std::string& name);
		LogFormat getDefaultFormat();
		void setDefaultFormat(LogFormat format);

	 private:
		void setupDefaultListener();
		LogListener* createListener(const std::string& target);
		void resetListener();
		void resetToPlatformListener();

	 private:
		CritSect mCritSect;
		LogListener* mListener;
		MultiplexLogListener* mDefaultListener;
		int mSetupCount;
		LogLevel mVerboseLevel;
		std::string mDefaultTag;
		LogFormat mDefaultFormat;
		std::string mDefaultTarget;
		std::string mName;

		typedef std::map<std::string, LogLevel> TagLevelMap;
		TagLevelMap mTagLevelMap;

		static LogManager msLogManager;
	};

}

#endif
