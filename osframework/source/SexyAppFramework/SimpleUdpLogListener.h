#ifndef __SIMPLE_UDP_LOG_LISTENER_H__
#define __SIMPLE_UDP_LOG_LISTENER_H__

#include "SexyLogListener.h"
#include "SexySocket.h"

#include <string>

namespace Sexy {

	class SimpleUdpLogListener : public LogListener {
	 public:
		SimpleUdpLogListener(const std::string& target = "");
		~SimpleUdpLogListener() {}

		virtual void log(LogLevel lvl, const std::string& tag, const std::string& s);

	 private:
		UDPSocket mSock;
		std::string mHost;
		std::string mPort;
	};

}

#endif
