#include "SexyLog.h"
#include "SexyLogManager.h"
#include "Common.h"

#include <stdarg.h>

namespace Sexy {

static const char* LogNames[] = {
	"verbose", "debug", "info", "warn", "error"
};
const char* logLevelName(LogLevel level)
{
	if (level >= LOG_VERBOSE && level < LOG_LEVEL_MAX)
		return LogNames[level];

	return "unknown";
}

bool logLevelFromName(const char* name, LogLevel& level)
{
	if (!name || !*name)
		return false;

	std::string s = name;
	inlineLower(s);
	for (size_t i = 0; i < sizeof(LogNames) / sizeof(LogNames[0]); i++)
	{
		if (s == LogNames[i])
		{
			level = LogLevel(i);
			return true;
		}
	}
	return false;
}

void log(LogLevel lvl, const std::string& tag, const std::string& s)
{
	LogManager& mgr = LogManager::getInstance();
	mgr.log(lvl, tag, s);
}

void logf(LogLevel lvl, const std::string& tag, const char* fmt, ...)
{
	LogManager& mgr = LogManager::getInstance();

	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	mgr.log(lvl, tag, s);
}

void logf(LogLevel lvl, const char* fmt, ...)
{
	LogManager& mgr = LogManager::getInstance();

	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	std::string tag;

	mgr.log(lvl, tag, s);
}

void log(LogLevel lvl, const std::string& s)
{
	std::string tag;
	log(lvl, tag, s);
}

void logv(const std::string& tag, const std::string& s)
{
	log(LOG_VERBOSE, tag, s);
}

void logtfv(const std::string& tag, const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	log(LOG_VERBOSE, tag, s);
}

void logfv(const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	std::string tag;
	log(LOG_VERBOSE, tag, s);
}

void logd(const std::string& tag, const std::string& s)
{
	log(LOG_DEBUG, tag, s);
}

void logtfd(const std::string& tag, const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	log(LOG_DEBUG, tag, s);
}

void logfd(const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	std::string tag;
	log(LOG_DEBUG, tag, s);
}

void logi(const std::string& tag, const std::string& s)
{
	log(LOG_INFO, tag, s);
}

void logtfi(const std::string& tag, const char* fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	log(LOG_INFO, tag, s);
}

void logfi(const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	std::string tag;

	log(LOG_INFO, tag, s);
}

void logw(const std::string& tag, const std::string& s)
{
	log(LOG_WARN, tag, s);
}

void logtfw(const std::string& tag, const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	log(LOG_ERROR, tag, s);
}

void logfw(const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	std::string tag;
	log(LOG_WARN, tag, s);
}

void loge(const std::string& tag, const std::string& s)
{
	log(LOG_WARN, tag, s);
}

void logtfe(const std::string& tag, const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	log(LOG_ERROR, tag, s);
}

void logfe(const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	std::string s = vformat(fmt, argList);
	va_end(argList);

	std::string tag;
	log(LOG_ERROR, tag, s);
}

}
