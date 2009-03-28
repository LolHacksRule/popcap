#ifndef XMLREGISTRYINTERFACE_H
#define XMLREGISTRYINTERFACE_H

#include "RegistryInterface.h"
#include <map>

namespace Sexy
{
	class XMLRegistryInterface: public RegistryInterface
	{
	public:
		XMLRegistryInterface(SexyAppBase * theApp);
		~XMLRegistryInterface();

	public:
		virtual bool Write(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength);
		virtual bool ReadKey(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength, HKEY theKey);
		virtual bool GetSubKeys(const std::string& theKeyName, StringVector* theSubKeys);
		virtual bool EraseKey(const SexyString& theKeyName);
		virtual void EraseValue(const SexyString& theValueName);
		virtual void Flush();

	private:
		void         Load();

		bool                             mLoaded;
		std::map<SexyString, SexyString> mRegistry;
		std::string                      mRegKey;
	};
}

#endif
