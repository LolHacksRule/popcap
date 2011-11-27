#ifndef  __SEXY_LOG_H__
#define  __SEXY_LOG_H__

#include <string>

namespace Sexy {

	enum LogLevel {
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARN,
		LOG_ERROR,
		LOG_LEVEL_MAX
	};

	const char* logLevelName(LogLevel level);
	bool logLevelFromName(const char* name, LogLevel& level);

	void logf(LogLevel lvl, const std::string& tag, const char* fmt, ...);
	void logf(LogLevel lvl, const char* fmt, ...);
	void log(LogLevel lvl, const std::string& tag, const std::string& s);
	void log(LogLevel lvl, const std::string& s);

	void logd(const std::string& tag, const std::string& s);
	void logtfd(const std::string& tag, const char* fmt, ...);
	void logfd(const char* fmt, ...);

	void logi(const std::string& tag, const std::string& s);
	void logtfi(const std::string& tag, const char* fmt, ...);
	void logfi(const char* fmt, ...);

	void logw(const std::string& tag, const std::string& s);
	void logtfw(const std::string& tag, const char* fmt, ...);
	void logfw(const char* fmt, ...);

	void loge(const std::string& tag, const std::string& s);
	void logtfe(const std::string& tag, const char* fmt, ...);
	void logfe(const char* fmt, ...);
};

#endif
