#ifndef __DEFAULT_LOG_LISTENER_H__
#define __DEFAULT_LOG_LISTENER_H__

#include "SexyLogListener.h"

namespace Sexy {

	class DefaultLogListener : public LogListener {
	 public:
		DefaultLogListener() {}
		~DefaultLogListener() {}

		virtual void log(LogLevel lvl, const std::string& tag, const std::string& s);
	};

}

#endif
