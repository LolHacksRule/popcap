#include "SexyLogManager.h"
#include "AutoCrit.h"
#include "MultiplexLogListener.h"
#include "DefaultLogListener.h"
#include "SimpleUdpLogListener.h"
#include "TcpLogListener.h"
#include "Common.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include "AndroidLogListener.h"
#endif

using namespace Sexy;

#ifndef SEXY_LOG
#define SEXY_LOG ""
#endif

LogManager LogManager::msLogManager;

LogManager::LogManager()
{
	const char* target = GetEnv("SEXY_LOG");
	mDefaultTarget = target ? target : SEXY_LOG;

	mVerboseLevel = LOG_INFO;

	const char* level = GetEnv("SEXY_LOG_LEVEL");
	if (level && !logLevelFromName(level, mVerboseLevel))
		mVerboseLevel = LogLevel(GetEnvIntOption("SEXY_LOG_LEVEL", mVerboseLevel));
	mName = "default";
	mDefaultTag = "default";
	mDefaultFormat = LogFormat(LOG_FORMAT_TAG | LOG_FORMAT_PID | LOG_FORMAT_TIMESTAMP);
	mDefaultListener = new MultiplexLogListener();
	mListener = 0;
	mSetupCount = 0;
}

LogManager::~LogManager()
{
	setListener(0);

	delete mDefaultListener;
}

LogManager& LogManager::getInstance()
{
	return msLogManager;
}

void LogManager::release()
{
	msLogManager.resetListener();
}

LogListener* LogManager::createListener(const std::string& target)
{
	LogListener* listener = 0;

	if (target == "udp" ||
	    target.substr(0, 6) == "udp://")
	{
		listener = new SimpleUdpLogListener(target);
	}
	else if (target == "tcp" ||
	    target.substr(0, 6) == "tcp://")
	{
		listener = new TcpLogListener(target);
	}
	else if (target == "default")
	{
#if defined(ANDROID) || defined(__ANDROID__)
		listener = new AndroidLogListener();
#else
		listener = new DefaultLogListener();
#endif
	}

	return listener;
}

void LogManager::setupDefaultListener()
{
	if (mListener)
		return;

	{
		AutoCrit anAutoCrit(mCritSect);

		if (mSetupCount)
			return;

		mSetupCount++;

		std::vector<std::string> targets;

		Split(mDefaultTarget, ";", targets);
		for (size_t i = 0; i < targets.size(); i++)
			mDefaultListener->addListener(createListener(targets[i]));
		if (!mDefaultListener->hasListener())
			mDefaultListener->addListener(createListener("default"));

		if (!mListener)
			mListener = mDefaultListener;
		mSetupCount--;
	}

	std::string name = logLevelName(mVerboseLevel);
	inlineUpper(name);
	log(LOG_INFO, "log", std::string("Verbose log level: ") + name);
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
	AutoCrit anAutoCrit(mCritSect);

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

std::string LogManager::getName()
{
	return mName;
}

void LogManager::setName(const std::string& name)
{
	mName = name;
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
