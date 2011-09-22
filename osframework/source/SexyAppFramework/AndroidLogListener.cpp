#include "AndroidLogListener.h"

#if defined(ANDROID) || defined(__ANDROID__)

#include <android/log.h>

using namespace Sexy;

void AndroidLogListener::log(LogLevel lvl, const std::string& tag, const std::string& s)
{
    int level = ANDROID_LOG_INFO;

    switch (lvl) {
    case LOG_DEBUG:
	level = ANDROID_LOG_DEBUG;
	break;

    case LOG_INFO:
	level = ANDROID_LOG_INFO;
	break;

    case LOG_ERROR:
	level = ANDROID_LOG_ERROR;
	break;

    default:
	level = ANDROID_LOG_INFO;
	break;
    }

    __android_log_print(level, tag.c_str(), s.c_str());
}

#endif

