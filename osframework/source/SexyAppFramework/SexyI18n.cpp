#include "Common.h"
#include "SexyI18n.h"
#include "SexyLang.h"
#include "CritSect.h"
#include "AutoCrit.h"
#include "XMLParser.h"

#include <assert.h>

#include <map>
#include <string>

namespace Sexy {

// msgid -> msgstr
typedef std::map<std::string, std::string> MsgStrMap;
// domain -> MsgStrMap
typedef std::map<std::string, MsgStrMap> PoFileMap;
class I18nManager
{
private:
	I18nManager();
	~I18nManager();

public:
	static I18nManager* GetManager();
	void setLocale(const char *locale);
	const char* setDomain(const char *domain);
	void bindTextDomain (const std::string &domain,
			     const std::string &dir);
	const char* tr(const char *domain,
		       const char *s);

private:
	PoFileMap mPoMap;
	std::string mLocale;
	std::string mDomain;
	bool mValid;
	CritSect mCritSect;
};

I18nManager* I18nManager::GetManager()
{
	static I18nManager aManager;
	return &aManager;
}

I18nManager::I18nManager()
{
	mValid = true;
}

I18nManager::~I18nManager()
{
	mValid = false;
}

void I18nManager::setLocale(const char *locale)
{
	if (!mValid)
		return;

	if (locale)
	{
		if (!*locale)
			mLocale = SexyGetLocaleName("LC_MESSAGES");
		else
			mLocale = locale;
	}
	else
	{
		mLocale = "";
	}

	// for example zh_CN.UTF-8
	std::string::size_type pos = mLocale.find('.');
	if (pos != std::string::npos)
		mLocale = mLocale.substr(0, pos);
}

const char* I18nManager::setDomain(const char *domain)
{
	if (!mValid)
		return 0;

	if (domain)
		mDomain = domain;
	else
		mDomain = "";

	return mDomain.c_str();
}

static bool parseMessage(XMLParser &parser,
			 XMLElement &theXMLElement,
			 MsgStrMap &map)
{
	std::string msgid;
	std::string msgstr;
	std::string obsolete;

	XMLParamMap::iterator anItr;
	anItr = theXMLElement.mAttributes.find(_S("obsolete"));
	if (anItr != theXMLElement.mAttributes.end())
		obsolete = theXMLElement.mAttributes[_S("obsolete")];

	for (;;)
	{
		XMLElement aXMLElement;
		if (!parser.NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == _S("occurrence"))
			{
				// ELEMENT
				if (!parser.NextElement(&aXMLElement))
					return false;

				// END
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return false;
			}
			else if (aXMLElement.mValue == _S("flag"))
			{
				// ELEMENT
				if (!parser.NextElement(&aXMLElement))
					return false;

				// END
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return false;
			}
			else if (aXMLElement.mValue == _S("msgid"))
			{
				// CDATA
				if (!parser.NextElement(&aXMLElement))
					return false;

				msgid = aXMLElement.mInstruction;
				//printf ("msgid: %s\n", msgstr.c_str());

				// END
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return false;
			}
			else if (aXMLElement.mValue == _S("msgstr_plural"))
			{
				// CDATA
				if (!parser.NextElement(&aXMLElement))
					return false;

				// END
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return false;
			}
			else if (aXMLElement.mValue == _S("msgstr"))
			{
				// CDATA
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (msgstr.empty())
					msgstr = aXMLElement.mInstruction;
				//printf ("msgstr: %s\n", msgstr.c_str());

				// END
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			//printf ("msgid: %s\nmsgstr: %s\n", msgid.c_str(), msgstr.c_str());
			if (!msgstr.empty() && obsolete != "true")
				map.insert (std::pair<std::string, std::string>(msgid, msgstr));
			return true;
		}
	}
	return false;
}

static bool parseMessages(XMLParser &parser,
			  XMLElement &theXMLElement,
			  MsgStrMap &map)
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!parser.NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == _S("message"))
			{
				if (!parseMessage(parser, aXMLElement, map))
					return false;
			}
			else if (aXMLElement.mValue == _S("header"))
			{
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_CDATA)
					return false;

				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			if (aXMLElement.mValue == _S("po"))
				return true;
		}
	}

	return true;
}

static bool parseXml(const std::string &path, MsgStrMap &map)
{
	XMLParser parser;

	if (!parser.OpenFile(path))
		return false;

	XMLElement aXMLElement;
	while (!parser.HasFailed())
	{
		if (!parser.NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == _S("po"))
				if (parseMessages(parser, aXMLElement, map))
					return true;
		}
	}

	printf ("Failed to parse %s: %s at line %d.\n",
		path.c_str(), parser.GetErrorText().c_str(),
		parser.GetCurrentLineNum());
	return false;
}

void I18nManager::bindTextDomain (const std::string &domain,
				  const std::string &dir)
{
	if (!mValid)
		return;

	if (mLocale.empty())
		return;

	std::string path = dir + "/" + mLocale + "/" + domain + ".xml";
	MsgStrMap map;
	if (!parseXml(path, map))
	{
		printf ("Failed to parse: %s\n", path.c_str());
		return;
	}

	AutoCrit aAutoCrit(mCritSect);
	if (mPoMap.find(domain) != mPoMap.end())
		return;
	mPoMap.insert(std::pair<std::string, MsgStrMap>(domain, map));
}

const char* I18nManager::tr(const char *sdomain,
			    const char *s)
{
	if (!mValid)
		return s;

	AutoCrit aAutoCrit(mCritSect);
	std::string domain = sdomain ? sdomain : mDomain;
	PoFileMap::iterator it = mPoMap.find(domain);
	if (it == mPoMap.end())
		return s;
	MsgStrMap::iterator mit = it->second.find(std::string(s));
	if (mit != it->second.end())
		return mit->second.c_str();
	return s;
}

const char* tr(const char *s)
{
	return I18nManager::GetManager()->tr(0, s);
}

const std::string tr(const std::string &s)
{
	return std::string(I18nManager::GetManager()->tr(0, s.c_str()));
}

const char* dtr(const char *domain, const char *s)
{
	if (!domain || !s)
		return 0;
	return I18nManager::GetManager()->tr(domain, s);
}

const std::string dtr(const std::string &domain, const std::string &s)
{
	return std::string(I18nManager::GetManager()->tr(domain.c_str(),
							 s.c_str()));
}

void setLocale(const char *locale)
{
	I18nManager::GetManager()->setLocale(locale);
}

const char* textDomain(const char *domain)
{
	assert (domain);
	return I18nManager::GetManager()->setDomain(domain);
}

void bindTextDomain(const char *domain, const char *dir)
{
	assert (domain && dir);
	I18nManager::GetManager()->bindTextDomain(std::string(domain),
						  std::string(dir));
}

void bindText(const char *domain, const char *dir)
{
	assert (domain && dir);
	I18nManager::GetManager()->bindTextDomain(std::string(domain),
						  std::string(dir));
	I18nManager::GetManager()->setDomain(domain);
}

}

