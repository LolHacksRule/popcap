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

struct MsgTrans {
	MsgStrMap mMap;
};

// lang -> translation
typedef std::map<std::string, MsgTrans> MsgTransMap;

// domain
struct MsgDomain {
	MsgDomain(const std::string &path) :
		mPath(path), mCurTrans(0)
	{
	}

	std::string mPath;
	MsgTransMap mMap;
	MsgTrans *mCurTrans;
};

// domain -> MsgDomain
typedef std::map<std::string, MsgDomain> DomainMap;
class I18nManager
{
private:
	I18nManager();
	~I18nManager();

public:
	static I18nManager* GetManager();
	bool loadTrans(const std::string &domain);
	void reloadTrans();
	const char* setLocale(const char *locale);
	const char* setDomain(const char *domain);
	void bindTextDomain (const std::string &domain,
			     const std::string &dir);
	const char* tr(const char *domain,
		       const char *s);

private:
	DomainMap mMap;
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

const char* I18nManager::setLocale(const char *locale)
{
	if (!mValid)
		return 0;

	if (!locale)
		return mLocale.c_str();

	std::string oldLocale = mLocale;
	if (!locale[0])
		mLocale = SexyGetLocaleName("LC_MESSAGES");
	else
		mLocale = locale;

	// for example zh_CN.UTF-8
	std::string::size_type pos = mLocale.find('.');
	if (pos != std::string::npos)
		mLocale = mLocale.substr(0, pos);

	// reload translations
	if (oldLocale != mLocale)
		reloadTrans();

	return mLocale.c_str();
}

const char* I18nManager::setDomain(const char *sdomain)
{
	if (!mValid)
		return 0;

	if (!sdomain)
		return mDomain.c_str();

	std::string domain = mDomain;

	AutoCrit aAutoCrit(mCritSect);
	mDomain = sdomain;
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
				map.insert (MsgStrMap::value_type(msgid, msgstr));
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

bool I18nManager::loadTrans(const std::string &domain)
{
	DomainMap::iterator it = mMap.find(domain);

	if (it == mMap.end())
		return true;

	MsgDomain &msgDomain = it->second;
	if (msgDomain.mMap.find(mLocale) != msgDomain.mMap.end()) {
		msgDomain.mCurTrans = &msgDomain.mMap.find(mLocale)->second;
		return true;
	}

	msgDomain.mMap.insert(MsgTransMap::value_type(mLocale, MsgTrans()));
	MsgTrans &trans = msgDomain.mMap.find(mLocale)->second;

	std::string path = msgDomain.mPath + "/" + mLocale + "/" + domain + ".xml";
	if (!parseXml(path, trans.mMap)) {
		printf ("Failed to parse: %s\n", path.c_str());
		trans.mMap.clear();
		msgDomain.mCurTrans = 0;
		return false;
	}
	msgDomain.mCurTrans = &trans;

	return true;
}

void I18nManager::reloadTrans()
{
	if (mLocale.empty())
		return;

	DomainMap::iterator it;

	for (it = mMap.begin(); it != mMap.end(); ++it)
		loadTrans(it->first);
}

void I18nManager::bindTextDomain (const std::string &domain,
				  const std::string &dir)
{
	if (!mValid)
		return;

	if (mLocale.empty())
		return;

	AutoCrit aAutoCrit(mCritSect);
	DomainMap::iterator it = mMap.find(domain);
	if (it == mMap.end())
		mMap.insert(DomainMap::value_type(domain,
						  MsgDomain(dir)));
	loadTrans(domain);
}

const char* I18nManager::tr(const char *sdomain,
			    const char *s)
{
	if (!mValid)
		return s;

	if (mLocale.empty())
		return s;

	AutoCrit aAutoCrit(mCritSect);
	std::string domain = sdomain ? sdomain : mDomain;
	DomainMap::iterator it = mMap.find(domain);
	if (it == mMap.end())
		return s;

	MsgDomain &msgDomain = it->second;
	if (!msgDomain.mCurTrans)
		return s;

	MsgTrans *trans = msgDomain.mCurTrans;
	MsgStrMap::iterator mit = trans->mMap.find(std::string(s));
	if (mit != trans->mMap.end())
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

const char* setLocale(const char *locale)
{
	return I18nManager::GetManager()->setLocale(locale);
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

