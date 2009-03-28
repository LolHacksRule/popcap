#ifndef Win32REGISTRYINTERFACE_H
#define Win32REGISTRYINTERFACE_H

#ifdef WIN32

#include "RegistryInterface.h"
#include <map>

namespace Sexy
{
	class Win32RegistryInterface: public RegistryInterface
	{
	public:
		Win32RegistryInterface(SexyAppBase * theApp);
		~Win32RegistryInterface();

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

#endif
