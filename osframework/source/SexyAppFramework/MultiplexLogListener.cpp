#include "MultiplexLogListener.h"

#include <algorithm>

using namespace Sexy;

MultiplexLogListener::MultiplexLogListener()
{
}

MultiplexLogListener::~MultiplexLogListener()
{
	LogListenerList::iterator it = mListeners.begin();

	for (; it != mListeners.end(); ++it)
		delete *it;
	mListeners.clear();
}

bool MultiplexLogListener::hasListener()
{
	return !mListeners.empty();
}

void MultiplexLogListener::addListener(LogListener* listener)
{
	if (!listener)
		return;

	LogListenerList::iterator it =
		std::find(mListeners.begin(), mListeners.end(),
			  listener);
	if (it != mListeners.end())
		return;
	mListeners.push_back(listener);

}

void MultiplexLogListener::removeListener(LogListener* listener)
{
	if (!listener)
		return;

	LogListenerList::iterator it =
		std::find(mListeners.begin(), mListeners.end(),
			  listener);
	if (it == mListeners.end())
		return;
	mListeners.erase(it);
}

void MultiplexLogListener::log(LogLevel lvl, const std::string& tag, const std::string& s)
{
	LogListenerList::iterator it = mListeners.begin();

	for (; it != mListeners.end(); ++it)
		(*it)->log(lvl, tag, s);
}
