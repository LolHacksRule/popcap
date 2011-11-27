#ifndef __MULTIPLEX_LOG_LISTENER_H__
#define __MULTIPLEX_LOG_LISTENER_H__

#include "SexyLogListener.h"

#include <list>

namespace Sexy {

	class MultiplexLogListener : public LogListener {
	 public:
		MultiplexLogListener();
		virtual ~MultiplexLogListener();

		bool hasListener();
		void addListener(LogListener* listener);
		void removeListener(LogListener* listener);

		virtual void log(LogLevel lvl, const std::string& tag, const std::string& s);

	 private:
		typedef std::list<LogListener*> LogListenerList;
		LogListenerList mListeners;
	};
}

#endif
