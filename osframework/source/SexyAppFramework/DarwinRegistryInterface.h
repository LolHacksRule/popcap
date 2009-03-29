#ifndef DARWINREGISTRYINTERFACE_H
#define DARWINREGISTRYINTERFACE_H

#include "RegistryInterface.h"

namespace Sexy
{
	class DarwinRegistryInterface: public RegistryInterface
	{
	public:
		DarwinRegistryInterface(SexyAppBase * theApp);

	public:
		// Registry access methods
		virtual bool Write(const std::string& theValueName, ulong theType, const uchar* theValue, ulong theLength);
		virtual bool Read(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength);
		virtual bool ReadKey(const std::string& theValueName, ulong* theType, uchar* theValue, ulong* theLength, HKEY theKey);

		virtual bool GetSubKeys(const std::string& theKeyName, StringVector* theSubKeys);
		virtual bool ReadString(const std::string& theValueName, std::string* theString);
		virtual bool ReadInteger(const std::string& theValueName, int* theValue);
		virtual bool ReadBoolean(const std::string& theValueName, bool* theValue);
		virtual bool ReadData(const std::string& theValueName, uchar* theValue, ulong* theLength);
		virtual bool WriteString(const std::string& theValueName, const std::string& theString);
		virtual bool WriteInteger(const std::string& theValueName, int theValue);
		virtual bool WriteBoolean(const std::string& theValueName, bool theValue);
		virtual bool WriteData(const std::string& theValueName, const uchar* theValue, ulong theLength);
		virtual bool EraseKey(const SexyString& theKeyName);
		virtual void EraseValue(const SexyString& theValueName);

		virtual void Flush(void);
	};

}

#endif
