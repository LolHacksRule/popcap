#include "SexyLogManager.h"
#include "AutoCrit.h"
#include "MultiplexLogListener.h"
#include "DefaultLogListener.h"
#include "SimpleUdpLogListener.h"
#include "TcpLogListener.h"
#include "Common.h"

#include <ctype.h>

#if defined(ANDROID) || defined(__ANDROID__)
#include "AndroidLogListener.h"
#endif

using namespace Sexy;

#ifndef SEXY_LOG
#ifdef SEXY_DEBUG
#define SEXY_LOG "default;tcp"
#else
#define SEXY_LOG ""
#endif
#endif

LogManager LogManager::msLogManager;

LogManager::LogManager()
{
	const char* target = GetEnv("SEXY_LOG");
	mDefaultTarget = target ? target : SEXY_LOG;

	mVerboseLevel = LOG_INFO;

	// SEXY_LOG_LEVEL={0,1,2,3}
	// SEXY_LOG_LEVEL={debug,info,warn,error}
	// SEXY_LOG_LEVEL="*:error abc:debug"
	const char* level = GetEnv("SEXY_LOG_LEVEL");
	if (level && *level)
	{
		std::string lvl(level);
		std::vector<std::string> tags;

		Split(level, " ", tags);
		for (size_t i = 0; i < tags.size(); i++)
		{
			std::vector<std::string> kv;

			Split(tags[i], ":", kv);
			if (kv.size() != 2)
				continue;

			LogLevel logLvl = mVerboseLevel;
			if (!logLevelFromName(kv[0].c_str(), logLvl))
				logLvl = LogLevel(atoi(kv[0].c_str()));

			if (kv[0] == "*")
				mVerboseLevel = logLvl;
			else
				mTagLevelMap[kv[0]] = logLvl;
		}

		if (tags.size() == 1 && lvl.find(':') == std::string::npos &&
		    !logLevelFromName(level, mVerboseLevel))
			mVerboseLevel =
				LogLevel(GetEnvIntOption("SEXY_LOG_LEVEL",
							 mVerboseLevel));
	}
	mName = "default";
	mDefaultTag = "default";
	mDefaultFormat = LogFormat(LOG_FORMAT_TAG | LOG_FORMAT_PID | LOG_FORMAT_TIMESTAMP);
	mDefaultListener = new MultiplexLogListener();
	mListener = 0;
	mSetupCount = 0;
}

LogManager::~LogManager()
{
	close();
}

LogManager& LogManager::getInstance()
{
	return msLogManager;
}

void LogManager::close()
{
	if (!mListener && !mDefaultListener)
		return;

	log(LOG_DEBUG, "log", "Closing log, byte...");
	setListener(0);

	delete mDefaultListener;
	mDefaultListener = 0;
}

void LogManager::resetToPlatformListener()
{
	// Reset to the defualt listener
	resetListener();

	// Delete the old the default listener
	mListener = 0;
	delete mDefaultListener;

	// Setup a new default listener with the default listener only
	mDefaultListener = new MultiplexLogListener();
	mDefaultListener->addListener(createListener("default"));
	resetListener();
}

void LogManager::release()
{
	msLogManager.resetToPlatformListener();
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
	else if (target == "stdout")
	{
		listener = new DefaultLogListener();
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
	if (mListener || mSetupCount)
		return;

	{
		AutoCrit anAutoCrit(mCritSect);

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

LogLevel LogManager::getVerboseLevel(const std::string& tag)
{
	TagLevelMap::iterator it = mTagLevelMap.find(tag);
	if (it != mTagLevelMap.end())
		return it->second;
	return mVerboseLevel;
}

void LogManager::log(LogLevel lvl, const std::string& tag, const std::string& s)
{
	setupDefaultListener();
	if (!mListener)
		return;

	const std::string& realTag = tag.empty() ? mDefaultTag : tag;
	mListener->log(lvl, realTag, s);
}
