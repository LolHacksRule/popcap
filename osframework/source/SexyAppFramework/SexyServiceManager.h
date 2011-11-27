#ifndef __SEXY_SERVICE_MANAGER_H__
#define __SEXY_SERVICE_MANAGER_H__

#include "SexyAppFramework/SexySocket.h"
#include "SexyAppFramework/SexyThread.h"

#include <map>
#include <string>

namespace Sexy {

	struct ServiceInfo
	{
		std::string mName;
		std::string mType;
		std::string mDesc;
		std::string mAddr;
		std::string mPort;
	};

	class ServiceManager
	{
	 private:
		ServiceManager();
		ServiceManager(const ServiceManager&);
		~ServiceManager();

	 public:
		static ServiceManager& getInstance();

	        bool initialize();
		void release();
		bool registerService(const ServiceInfo& info);
		void unregisterService(const ServiceInfo& info);
		void setName(const std::string& name);

	 private:
		static void  serverProc(void*);
		void  server();

		void processEchoPacket(char*			buf,
				       int			buflen,
				       const std::string&	addr,
				       unsigned short		port);

		void processQueryPacket(char*			buf,
					int			buflen,
					const std::string&	addr,
					unsigned short		port);


		void processQueryInfoPacket(char*		buf,
					    int			buflen,
					    const std::string&	addr,
					    unsigned short	port);

		void processPacket(char*		buf,
				   int			buflen,
				   const std::string&	addr,
				   unsigned short	port);

	 private:
		UDPSocket* mSock;
		UDPSocket* mUniSock;
		bool       mValid;
		bool       mInitialized;
		bool       mDone;
		Thread     mThread;
		unsigned   mCookie;

		typedef std::map<std::string, ServiceInfo> ServiceInfoMap;
		ServiceInfoMap mServiceInfoMap;

		std::string  mName;
	};
}

#endif
