#include "DefaultLogListener.h"
#include "SexyLogManager.h"
#include "Common.h"

#include <stdio.h>

using namespace Sexy;

void DefaultLogListener::log(LogLevel lvl, const std::string& tag, const std::string& s)
{
	if (LogManager::getInstance().getVerboseLevel() > lvl)
		return;

	if (s.empty())
		return;

	std::string msg = formatMessage(lvl, tag, s);
	if (msg.empty())
		return;

	inlineRTrim(msg);
	printf("%s\n", msg.c_str());
}
