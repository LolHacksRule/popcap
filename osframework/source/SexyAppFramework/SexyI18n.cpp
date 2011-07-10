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

const std::string msgctxtDel = "<|||>";

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
	bool loadTransForLang(MsgDomain &msgDomain,
			      const std::string &domain,
			      const std::string &lang);
	void reloadTrans();
	const char* setLocale(const char *locale);
	const char* setDomain(const char *domain);
	void bindTextDomain (const std::string &domain,
			     const std::string &dir);
	const char* tr(const char *domain,
		       const char *ctx,
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

	AutoCrit aAutoCrit(mCritSect);

	std::string oldLocale = mLocale;
	if (!locale[0])
		mLocale = SexyGetLocaleName("LC_MESSAGES");
	else
		mLocale = locale;

	if (mLocale == "C" || mLocale == "POSIX")
		mLocale = "en_US";

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

	AutoCrit aAutoCrit(mCritSect);
	mDomain = sdomain;
	return mDomain.c_str();
}

static bool parseMessage(XMLParser &parser,
			 XMLElement &theXMLElement,
			 MsgStrMap &map)
{
	std::string msgid;
	std::string msgctxt;
	std::string msgstr;
	std::string obsolete;

	XMLParamMap::iterator anItr;
	anItr = theXMLElement.mAttributes.find("obsolete");
	if (anItr != theXMLElement.mAttributes.end())
		obsolete = theXMLElement.mAttributes["obsolete"];

	for (;;)
	{
		XMLElement aXMLElement;
		if (!parser.NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "occurrence")
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
			else if (aXMLElement.mValue == "flag")
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
			else if (aXMLElement.mValue == "msgid")
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
			else if (aXMLElement.mValue == "msgctxt")
			{
				// CDATA
				if (!parser.NextElement(&aXMLElement))
					return false;

				msgctxt = aXMLElement.mInstruction;
				//printf ("msgctxt: %s\n", msgctxt.c_str());

				// END
				if (!parser.NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return false;
			}
			else if (aXMLElement.mValue == "msgstr_plural")
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
			else if (aXMLElement.mValue == "msgstr")
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
			if (!msgstr.empty() && obsolete != "true") {
				if (!msgctxt.empty())
					msgid = msgctxt + msgctxtDel + msgid;
				map.insert (MsgStrMap::value_type(msgid, msgstr));
			}
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
			if (aXMLElement.mValue == "message")
			{
				if (!parseMessage(parser, aXMLElement, map))
					return false;
			}
			else if (aXMLElement.mValue == "header")
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
			if (aXMLElement.mValue == "po")
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
			if (aXMLElement.mValue == "po")
				if (parseMessages(parser, aXMLElement, map))
					return true;
		}
	}

	printf ("Failed to parse %s: %s at line %d.\n",
		path.c_str(), parser.GetErrorText().c_str(),
		parser.GetCurrentLineNum());
	return false;
}

bool I18nManager::loadTransForLang(MsgDomain &msgDomain,
				   const std::string &domain,
				   const std::string &lang)
{
	// reset the current translation
	msgDomain.mCurTrans = 0;

	// check if it's already loaded
	MsgTransMap::iterator it = msgDomain.mMap.find(lang);
	if (it != msgDomain.mMap.end() && !it->second.mMap.empty()) {
		msgDomain.mCurTrans = &it->second;
		return true;
	}

	// try to load it
	msgDomain.mMap.insert(MsgTransMap::value_type(lang, MsgTrans()));

	MsgTrans &trans = msgDomain.mMap.find(lang)->second;
	std::string path = msgDomain.mPath + "/" + lang + "/" + domain + ".xml";
	if (!parseXml(path, trans.mMap)) {
		trans.mMap.clear();
		return false;
	}
	msgDomain.mCurTrans = &trans;
	return true;
}

bool I18nManager::loadTrans(const std::string &domain)
{
	DomainMap::iterator it = mMap.find(domain);

	if (it == mMap.end())
		return true;

	MsgDomain &msgDomain = it->second;

	// load the current translation
	if (loadTransForLang(msgDomain, domain, mLocale))
		return true;

	// try to load a alternative translation
	// for example, load zh translation if the current lang is zh_CN
	size_t pos = mLocale.find('_');
	if (pos != std::string::npos &&
	    loadTransForLang(msgDomain, domain, mLocale.substr(0, pos)))
		return true;

	return false;
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

	AutoCrit aAutoCrit(mCritSect);
	DomainMap::iterator it = mMap.find(domain);
	if (it == mMap.end())
		mMap.insert(DomainMap::value_type(domain,
						  MsgDomain(dir)));
	if (mLocale.empty())
		return;
	loadTrans(domain);
}

const char* I18nManager::tr(const char *sdomain,
			    const char *ctx,
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
	std::string msgid;
	if (ctx)
		msgid = std::string(ctx) + msgctxtDel + std::string(s);
	else
		msgid = std::string(s);
	MsgStrMap::iterator mit = trans->mMap.find(msgid);
	if (mit != trans->mMap.end())
		return mit->second.c_str();
	return s;
}

const char* tr(const char *s)
{
	if (!s)
		return s;
	return I18nManager::GetManager()->tr(0, 0, s);
}

std::string tr(const std::string &s)
{
	return std::string(I18nManager::GetManager()->tr(0, 0, s.c_str()));
}

const char* tr(const char *ctx, const char *s)
{
	return I18nManager::GetManager()->tr(0, ctx, s);
}

std::string tr(const std::string &ctx, const std::string &s)
{
	return std::string(I18nManager::GetManager()->
			   tr(0, ctx.c_str(), s.c_str()));
}

const char* dtr(const char *domain, const char *s)
{
	if (!s)
		return s;
	return I18nManager::GetManager()->tr(domain, 0, s);
}

std::string dtr(const std::string &domain, const std::string &s)
{
	return std::string(I18nManager::GetManager()->tr(domain.c_str(),
							 0, s.c_str()));
}

const char* dtr(const char *domain, const char *ctx, const char *s)
{
	if (!s)
		return s;
	return I18nManager::GetManager()->tr(domain, ctx, s);
}

std::string dtr(const std::string &domain, const std::string &ctx,
		const std::string &s)
{
	return std::string(I18nManager::GetManager()->
			   tr(domain.c_str(), ctx.c_str(), s.c_str()));
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

