#include "SimpleUdpLogListener.h"
#include "SexyLogManager.h"
#include "Common.h"

#include <stdio.h>

using namespace Sexy;

SimpleUdpLogListener::SimpleUdpLogListener(const std::string& target) :
	mHost("localhost"), mPort("11035")
{
	std::string s = target;

	if (s.substr(0, 6) != "udp://")
		return;

	s = s.substr(6);
	std::vector<std::string> tuple;
	Split(s, ":", tuple);

	if (tuple.size() > 1)
		mPort = tuple[1];
	if (tuple.size() > 0)
		mHost = tuple[0];
}

void SimpleUdpLogListener::log(LogLevel lvl, const std::string& tag, const std::string& s)
{
	if (mHost.empty() || mPort.empty() || s.empty())
		return;

	std::string msg = formatMessage(lvl, tag, s);
	if (msg.empty())
		return;

	inlineRTrim(msg);

	msg += "\n";
	int left = msg.length();
	const char* data = msg.data();
	int port = atoi(mPort.c_str());
	while (left)
	{
		size_t bytes = std::min(left, 1500);
		if (mSock.sendTo(data, bytes, mHost, port) < 0)
			return;
		data += bytes;
		left -= bytes;
	}
}
