#ifndef __SEXY_LOG_MANAGER_H__
#define __SEXY_LOG_MANAGER_H__

#include "SexyLogListener.h"

namespace Sexy {

	class LogManager {
	 private:
		LogManager();
		~LogManager();

	 public:
		static LogManager& getInstance();
		static void release();

		void silent();
		void setListener(LogListener* listener);
		void log(LogLevel lvl, const std::string& tag, const std::string& s);
		void setVerboseLevel(LogLevel lvl);
		LogLevel getVerboseLevel();
		std::string getDefaultTag();
		void setDefaultTag(const std::string& tag);
		LogFormat getDefaultFormat();
		void setDefaultFormat(LogFormat format);

	 private:
		void setupDefaultListener();
		void resetListener();

	 private:
		LogListener* mListener;
		LogListener* mDefaultListener;
		LogLevel mVerboseLevel;
		std::string mDefaultTag;
		LogFormat mDefaultFormat;

		static LogManager msLogManager;
	};

}

#endif
