#include "TcpLogListener.h"
#include "SexyLogManager.h"
#include "SexyTimer.h"
#include "Common.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(WIN32) || defined(_WIN32)
#define getpid() ((int)GetCurrentProcessId())
#endif

#include <stdio.h>
#include <errno.h>
#include <assert.h>

using namespace Sexy;

#define LOG_TAG "log"

TcpLogListener::TcpLogListener(const std::string& target) :
	mHost(""), mPort("11035")
{
	mMaxLogRecordSize = GetEnvIntOption("SEXY_TCP_LOG_BUFFER_SIZE", 1024 * 1024);
	mLogRecordSeq = 0;
	mLogRecordSize = 0;
	mDone = true;

	std::string s = target;

	if (s.substr(0, 6) == "tcp://")
	{
		s = s.substr(6);
		std::vector<std::string> tuple;
		Split(s, ":", tuple);

		if (tuple.size() > 1)
			mPort = tuple[1];
		if (tuple.size() > 0)
			mHost = tuple[0];
	}

	mDone = true;
	for (;;)
	{
		mSock = new TCPServerSocket();
		if (!mSock->hasError() &&
		    mSock->setLocalAddressAndPort(mHost, atoi(mPort.c_str())) &&
		    mSock->setListen(5))
		{
			mPort = StrFormat("%d", mSock->getLocalPort());

		        logtfi(LOG_TAG,
			       "Listening on %s:%d\n",
			       mHost.c_str(), mSock->getLocalPort());
			break;
		}
		delete mSock;
		mSock = 0;

		if (mPort == "0")
			break;
		else
			mPort = "0";
	}
	if (!mSock)
		return;

	mServiceInfo.mName = "sexytcplog";
	mServiceInfo.mDesc = "The log service for SexyAppFramework";
	mServiceInfo.mType = "tcp";
	mServiceInfo.mAddr = mSock->getLocalAddress();
	mServiceInfo.mPort = mPort;

	ServiceManager& mgr = ServiceManager::getInstance();
	mgr.registerService(mServiceInfo);

	mDone = false;
	mThread = Thread::Create(serverProc, this);
}

TcpLogListener::~TcpLogListener()
{
	if (mSock)
	{
		mDone = true;
		mThread.Join();

		ServiceManager& mgr = ServiceManager::getInstance();
		mgr.unregisterService(mServiceInfo);
	}

	ClientMap::iterator it;
	for (it = mClientMap.begin(); it != mClientMap.end();)
	{
		it->second.close();
		mClientMap.erase(it++);
	}
}

void TcpLogListener::serverProc(void* arg)
{
	TcpLogListener* tcpLogListener = (TcpLogListener*)arg;
	tcpLogListener->server();
}

void TcpLogListener::addClient(TCPSocket* sock)
{
	mClientMap.insert(ClientMap::value_type(sock->getSocket(), TcpLogClient()));
	TcpLogClient& client = mClientMap[sock->getSocket()];
	client.setSocket(sock);
	client.mSeq = 0;
	client.mWouldBlock = false;
}

static inline char* writel(char *buf, unsigned int l)
{
	buf[0] = l >> 24;
	buf[1] = (l >> 16) & 0xff;
	buf[2] = (l >>  8) & 0xff;
	buf[3] = (l >>  0) & 0xff;

	return buf + 4;
}

static inline char* writes(char *buf, unsigned short s)
{
	buf[0] = (s >>  8) & 0xff;
	buf[1] = (s >>  0) & 0xff;

	return buf + 2;
}

bool TcpLogListener::sendRecord(TcpLogRecord* record, TcpLogClient& client)
{
	assert (client.mSock != 0);

	// 4b tag 'LGBD'
	// 4b package len
	// 4b pid
	// 4b timestamp
	// 2b log level
	// 2b tag length
	// 4b msg length
	char header[4 + 4 + 4 + 4 + 2 + 2 + 4];
	char* ptr = header;

	ptr[0] = 'L';
	ptr[1] = 'G';
	ptr[2] = 'B';
	ptr[3] = 'D';
	ptr += 4;

	ptr = writel(ptr, sizeof(header) + record->tag.length() + record->msg.length());
	ptr = writel(ptr, record->pid);
	ptr = writel(ptr, record->timestamp);
	ptr = writes(ptr, record->lvl);
	ptr = writes(ptr, record->tag.length());
	ptr = writel(ptr, record->msg.length());
	if (!client.mSock->send(header, sizeof(header)))
		return false;
	if (!client.mSock->send(record->tag.data(), record->tag.length()))
		return false;
	if (!client.mSock->send(record->msg.data(), record->msg.length()))
		return false;
	return true;
}

void TcpLogListener::server()
{
	TcpLogRecord record;

	while (!mDone && mSock)
	{
		int sock = mSock->getSocket();
		fd_set rfds;
		fd_set wfds;
		fd_set efds;

		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		FD_ZERO(&efds);
		FD_SET(sock, &efds);
		FD_ZERO(&wfds);

		int maxsock = sock;
		ClientMap::iterator it;
		for (it = mClientMap.begin(); it != mClientMap.end(); ++it)
		{
			TcpLogClient& client = it->second;

			if (it->first > maxsock)
				maxsock = it->first;

			if (client.mWouldBlock && client.mSeq < mLogRecordSeq)
				FD_SET(it->first, &wfds);
			FD_SET(it->first, &efds);
		}

		struct timeval tv;
		tv.tv_sec = 0;
		if (mClientMap.empty())
			tv.tv_usec = 10000;
		else
			tv.tv_usec = 1000;

		int ret = select(maxsock + 1, &rfds, &wfds, &efds, &tv);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret < 0)
			break;

		if (ret && FD_ISSET(sock, &efds))
			break;

		if (ret && FD_ISSET(sock, &rfds))
		{
			TCPSocket* clientSock = mSock->accept();
			if (clientSock)
			{
				addClient(clientSock);
				logtfd(LOG_TAG, "New client: %d.\n",
				       clientSock->getSocket());
			}
		}

		for (it = mClientMap.begin(); it != mClientMap.end();)
		{
			TcpLogClient& client = it->second;

			if (ret && FD_ISSET(it->first, &efds))
			{
				logtfd(LOG_TAG, "Removing client: %d\n",
				       client.mSock->getSocket());
				client.close();
				mClientMap.erase(it++);
			}

			if (ret && FD_ISSET(it->first, &rfds))
			{
				// verify password ?
			}

			if (!client.mWouldBlock || (ret && FD_ISSET(it->first, &wfds)))
			{
				int64 seq;
				{
					AutoCrit anAutoCrit(mCritSect);

					TcpLogRecordMap::iterator logIt;
					logIt = mLogRecords.upper_bound(client.mSeq);
					if (logIt == mLogRecords.end())
					{
						++it;
						continue;
					}

					seq = logIt->first;
					record = logIt->second;
				}

				if (!sendRecord(&record, client))
				{
					logtfd(LOG_TAG, "Removing client: %d.\n",
					       client.mSock->getSocket());
					client.close();
					mClientMap.erase(it++);
					continue;
				}

				client.mSeq = seq;
			}

			++it;
		}
	}
}

void TcpLogListener::log(LogLevel lvl, const std::string& tag, const std::string& s)
{
	if (mPort.empty() || !mSock || s.empty())
		return;

	AutoCrit anAutoCrit(mCritSect);
	int64 seq = mLogRecordSeq++;

	mLogRecords.insert(TcpLogRecordMap::value_type(seq, TcpLogRecord()));
	TcpLogRecord& record = mLogRecords[seq];
	record.lvl = lvl;
	record.tag = tag;
	record.msg = s;
	record.pid = getpid();
	record.timestamp = GetTickCount();
	inlineRTrim(record.msg);

	size_t size = record.size();
	while (mMaxLogRecordSize && mLogRecordSize + size > mMaxLogRecordSize &&
	       !mLogRecords.empty())
	{
		TcpLogRecordMap::iterator it = mLogRecords.begin();

		assert (mLogRecordSize > it->second.size());

		mLogRecordSize -= it->second.size();
		mLogRecords.erase(it);
	}
	mLogRecordSize += size;
	//printf("TcpLogListener: logsize %zd\n", mLogRecordSize);
}
