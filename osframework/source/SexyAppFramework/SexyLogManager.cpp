#include "SexyLogManager.h"
#include "DefaultLogListener.h"
#include "Common.h"
#include "SimpleUdpLogListener.h"
#include "TcpLogListener.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include "AndroidLogListener.h"
#endif

using namespace Sexy;

LogManager LogManager::msLogManager;

LogManager::LogManager()
{
	const char* target = GetEnv("SEXY_LOG");
	mDefaultTarget = target ? target : "";
	mVerboseLevel = LogLevel(GetEnvIntOption("SEXY_LOG_LEVEL", LOG_INFO));
	mDefaultTag = "default";
	mDefaultFormat = LogFormat(LOG_FORMAT_TAG | LOG_FORMAT_PID | LOG_FORMAT_TIMESTAMP);
}

LogManager::~LogManager()
{
	setListener(0);
}

LogManager& LogManager::getInstance()
{
	return msLogManager;
}

void LogManager::release()
{
	msLogManager.resetListener();
}

void LogManager::setupDefaultListener()
{
	if (mListener)
		return;

	if (mDefaultTarget == "udp" ||
	    mDefaultTarget.substr(0, 6) == "udp://")
	{
		mDefaultListener = new SimpleUdpLogListener(mDefaultTarget);
	}
	else if (mDefaultTarget == "tcp" ||
	    mDefaultTarget.substr(0, 6) == "tcp://")
	{
		mDefaultListener = new TcpLogListener(mDefaultTarget);
	}
	else
	{
#if defined(ANDROID) || defined(__ANDROID__)
		mDefaultListener = new AndroidLogListener();
#else
		mDefaultListener = new DefaultLogListener();
#endif
	}
	resetListener();
}

void LogManager::resetListener()
{
	setListener(mDefaultListener);
}

void LogManager::silent()
{
	setListener(new NullLogListener());
}

void LogManager::setListener(LogListener *listener)
{
	if (mListener != mDefaultListener)
		delete mListener;
	mListener = listener;
}

void LogManager::setVerboseLevel(LogLevel lvl)
{
	mVerboseLevel = lvl;
}

LogLevel LogManager::getVerboseLevel()
{
	return mVerboseLevel;
}

std::string LogManager::getDefaultTag()
{
	return mDefaultTag;
}

void LogManager::setDefaultTag(const std::string& tag)
{
	mDefaultTag = tag;
}

LogFormat LogManager::getDefaultFormat()
{
	return mDefaultFormat;
}

void LogManager::setDefaultFormat(LogFormat format)
{
	mDefaultFormat = format;
}

void LogManager::log(LogLevel lvl, const std::string& tag, const std::string& s)
{
	setupDefaultListener();
	if (!mListener)
		return;

	const std::string& realTag = tag.empty() ? mDefaultTag : tag;
	mListener->log(lvl, realTag, s);
}
