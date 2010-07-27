// XMLWriter.h: interface for the XMLWriter class by James Poag
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XML_WRITER__)
#define __XML_WRITER__

#ifdef _MSC_VER
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include <stack>
#include "XMLParser.h"

namespace Sexy
{
	class XMLWriter
	{
	protected:
		std::string				mFileName;
		std::string				mErrorText;
		int					mLineNum;
		FILE*					mFile;
		bool					mHasFailed;
		bool					mAllowComments;
		bool					mOpenAttributes;

		std::stack<std::string>	                mSectionStack;
		std::stack<std::string>                 mWarningStack;

	protected:
		bool					CheckFileOpen();
		bool					ValidateElementNodeName(const std::string& theNodeName);
		void					Fail(const std::string& theErrorText);
		void					Warn(const std::string& theWarning);
		void					Init();

	public:
		XMLWriter();
		virtual ~XMLWriter();

		static	bool				AddAttribute(XMLElement* theElement, const std::string& aAttributeKey, const std::string& aAttributeValue);
		bool					WriteAttribute(const std::string& aAttributeKey, const std::string& aAttributeValue);
		bool					WriteAttribute(const std::string& aAttributeKey, const float& aAttributeValue);
		bool					WriteAttribute(const std::string& aAttributeKey, const int& aAttributeValue);
		void					Comment(const std::string& theComment);
		bool					StartElement(const std::string &theElementName);
		bool					StartElement(XMLElement *theElement);
		bool					StopElement();
		bool					OpenFile(const std::string& theFilename);
		bool					CloseFile();
		std::string				GetErrorText();
		int					GetCurrentLineNum();
		std::string				GetFileName();

		inline void				AllowComments(bool doAllow) { mAllowComments = doAllow; }

		bool					HasFailed();
	};
};


#endif
