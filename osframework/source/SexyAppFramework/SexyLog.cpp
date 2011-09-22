#include "SexyLog.h"
#include "SexyLogManager.h"
#include "Common.h"

#include <stdarg.h>

namespace Sexy {

const char* logLevelName(LogLevel level)
{
	const char* names[] = {
		"debug", "info", "error"
	};
	if (level >= LOG_DEBUG && level < LOG_LEVEL_MAX)
		return names[level];

	return "unknown";
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

void log(LogLevel lvl, const std::string& s)
{
	std::string tag;
	log(lvl, tag, s);
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

void loge(const std::string& tag, const std::string& s)
{
	log(LOG_ERROR, tag, s);
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
