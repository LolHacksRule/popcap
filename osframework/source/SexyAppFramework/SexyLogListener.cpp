#include "SexyLogListener.h"
#include "SexyLogManager.h"
#include "Common.h"

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#define getpid() ((int)GetProcessId())
#else
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace Sexy;

std::string LogListener::formatMessage(LogLevel lvl, const std::string& tag, const std::string& s)
{
	LogFormat format = mFormat;

	std::string prefix;

	if (format == LOG_FORMAT_DEFAULT)
		format = LogManager::getInstance().getDefaultFormat();

	if (format & LOG_FORMAT_PID)
		prefix += StrFormat("%u", (unsigned)getpid());
	if (format & LOG_FORMAT_TAG)
		prefix += prefix.empty() ? tag : std::string("/") + tag;
	if (format & LOG_FORMAT_TIMESTAMP)
		prefix += StrFormat(" %u", (unsigned)GetTickCount());
	return prefix + " " + s;
}

