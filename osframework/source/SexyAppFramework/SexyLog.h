#ifndef  __SEXY_LOG_H__
#define  __SEXY_LOG_H__

#include <string>

namespace Sexy {

	enum LogLevel {
		LOG_DEBUG,
		LOG_INFO,
		LOG_ERROR,
		LOG_LEVEL_MAX
	};

	const char* logLevelName(LogLevel level);
	void logf(LogLevel lvl, const std::string& tag, const char* fmt, ...);
	void log(LogLevel lvl, const std::string& tag, const std::string& s);
	void log(LogLevel lvl, const std::string& s);
	void log(LogLevel lvl, const char* fmt, ...);
	void logd(const std::string& tag, const std::string& s);
	void logtfd(const std::string& tag, const char* fmt, ...);
	void logfd(const char* fmt, ...);
	void logi(const std::string& tag, const std::string& s);
	void logtfi(const std::string& tag, const char* fmt, ...);
	void logfi(const char* fmt, ...);
	void loge(const std::string& tag, const std::string& s);
	void logtfe(const std::string& tag, const char* fmt, ...);
	void logfe(const char* fmt, ...);
};

#endif
