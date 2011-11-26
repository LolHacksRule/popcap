#ifndef __TCP_LOG_LISTENER_H__
#define __TCP_LOG_LISTENER_H__

#include "SexyLogListener.h"
#include "SexyServiceManager.h"
#include "SexySocket.h"
#include "SexyThread.h"
#include "AutoCrit.h"

#include <string>

namespace Sexy {

	struct TcpLogRecord {
		LogLevel	lvl;
		std::string	tag;
		std::string	msg;
		int             pid;
		DWORD           timestamp;

		size_t size()
		{
			return sizeof(TcpLogRecord) + tag.length() + msg.length();
		}
	};

	struct TcpLogClient {
		TCPSocket*    mSock;
		int64         mSeq;
		bool          mWouldBlock;

		void setSeq(int64 seq)
		{
			mSeq = seq;
		}

		void setSocket(TCPSocket* sock)
		{
			mSock = sock;
		}

		void close()
		{
			delete mSock;
			mSock = 0;
		}
	};

	class TcpLogListener : public LogListener {
	 public:
		TcpLogListener(const std::string& target = "");
		~TcpLogListener();

		virtual void log(LogLevel lvl, const std::string& tag, const std::string& s);

	 private:
		static void serverProc(void* arg);

		void server();
		void addClient(TCPSocket* socket);
		bool sendRecord(TcpLogRecord* record, TcpLogClient& client);

	 private:
		TCPServerSocket *mSock;
		Thread           mThread;
		CritSect         mCritSect;
		bool             mDone;

		typedef std::map<int, TcpLogClient> ClientMap;
		ClientMap        mClientMap;

		typedef std::map<int64, TcpLogRecord> TcpLogRecordMap;
		TcpLogRecordMap  mLogRecords;
		int64            mLogRecordSeq;
		size_t           mMaxLogRecordSize;
		size_t           mLogRecordSize;

		std::string	 mHost;
		std::string	 mPort;

		ServiceInfo      mServiceInfo;
	};

}

#endif
