#ifndef __ANDROID_LOG_LISTENER_H__
#define __ANDROID_LOG_LISTENER_H__

#include "SexyLogListener.h"

namespace Sexy {

	class AndroidLogListener : public LogListener {
	 public:
		AndroidLogListener() {}
		~AndroidLogListener() {}

		virtual void log(LogLevel lvl, const std::string& tag, const std::string& s);
	};

}

#endif
