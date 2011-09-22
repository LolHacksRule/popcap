#ifndef __SEXY_LOG_LISTENER_H__
#define __SEXY_LOG_LISTENER_H__

#include "SexyLog.h"

namespace Sexy {

	enum LogFormat {
		LOG_FORMAT_DEFAULT    = -1,
		LOG_FORMAT_NONE       = 0,
		LOG_FORMAT_TAG        = 1 << 0,
		LOG_FORMAT_TIMESTAMP  = 1 << 1,
		LOG_FORMAT_PID        = 1 << 2,
	};

	class LogListener {
	 public:
		LogListener() { mFormat = LOG_FORMAT_DEFAULT; }
		virtual ~LogListener() {}

		void setFormat(LogFormat format) { mFormat = format; }
		LogFormat getFormat() { return mFormat; }

		virtual std::string formatMessage(LogLevel lvl, const std::string& tag, const std::string& s);
		virtual void log(LogLevel lvl, const std::string& tag, const std::string& s) = 0;

	 protected:
		LogFormat mFormat;
	};

	class NullLogListener : public LogListener {
	 public:
		NullLogListener() {}
		~NullLogListener() {}

		virtual void log(LogLevel lvl, const std::string& tag, const std::string& s) {}
	};

}

#endif
