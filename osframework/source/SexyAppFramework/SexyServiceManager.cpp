#include "SexyServiceManager.h"
#include "Common.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <assert.h>

using namespace Sexy;

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

static inline char* writestr(char *buf, const char* str, unsigned short len)
{
	buf = writes(buf, len);
	memcpy(buf, str, len);
	return buf + len;
}

static inline char* readl(char *buf, unsigned int& l)
{
	l = buf[0] << 24 | buf[1] << 16 |
		buf[2] <<  8 | buf[3];

	return buf + 4;
}

static inline char* reads(char *buf, unsigned short& s)
{
	s = buf[2] <<  8 | buf[3];
	return buf + 2;
}

ServiceManager::ServiceManager()
{
	mSock = 0;
	mValid = true;
	mInitialized = false;
	mDone = true;
	mCookie = 0;
}

ServiceManager::~ServiceManager()
{
	release();
}

void ServiceManager::release()
{
	if (!mValid)
		return;

	mValid = false;
	if (mInitialized)
	{
		mDone = true;
		mThread.Join();
	}
	mInitialized = false;
	delete mSock;
	mSock = 0;
}

ServiceManager& ServiceManager::getInstance()
{
	static ServiceManager mgr;
	return mgr;
}

void ServiceManager::serverProc(void* arg)
{
	ServiceManager* mgr = (ServiceManager*)arg;
	mgr->server();
}

void ServiceManager::processEchoPacket(char*			buf,
				       int			buflen,
				       const std::string&	addr,
				       unsigned short		port)
{
	char outbuf[16];
	char* ptr = outbuf;

	ptr[0] = 'E';
	ptr[1] = 'C';
	ptr[2] = 'R';
	ptr[3] = 'P';
	ptr += 4;

	memcpy(ptr, &buf[4], 4);
	ptr += 4;

	ptr = writel(ptr, 4);
	ptr = writel(ptr, mCookie);
	mSock->sendTo(outbuf, sizeof(outbuf), addr, port);
}

void ServiceManager::processQueryPacket(char*			buf,
					int			buflen,
					const std::string&	addr,
					unsigned short		port)
{
	size_t count = mServiceInfoMap.size();
	if (!count)
	{
		char outbuf[20];
		char* ptr = outbuf;

		ptr[0] = 'Q';
		ptr[1] = 'R';
		ptr[2] = 'R';
		ptr[3] = 'P';
		ptr += 4;

		memcpy(ptr, &buf[4], 4);
		ptr += 4;

		ptr = writel(ptr, 8);
		ptr = writel(ptr, mCookie);
		ptr = writel(ptr, 0);

		//printf("ServiceManager: Sending a query reply packet(max: %d: size: %d) to %s\n",
		//       (int)count, (int)sizeof(outbuf), addr.c_str());

		mSock->sendTo(outbuf, sizeof(outbuf), addr, port);
		return;
	}

	char outbuf[512];
	int i = 0;

	ServiceInfoMap::iterator it = mServiceInfoMap.begin();

	while (it != mServiceInfoMap.end())
	{
		ServiceInfo& si = it->second;

		char* ptr = outbuf;

		ptr[0] = 'Q';
		ptr[1] = 'R';
		ptr[2] = 'R';
		ptr[3] = 'P';
		ptr += 4;

		memcpy(ptr, &buf[4], 4);
		ptr += 4;

		// skip the payload size;
		ptr += 4;

		ptr = writel(ptr, mCookie);
		ptr = writel(ptr, count);
		ptr = writel(ptr, i);
		ptr = writestr(ptr, "name", 4);
		ptr = writestr(ptr, si.mName.data(), si.mName.length());

		ptr = writestr(ptr, "desc", 4);
		ptr = writestr(ptr, si.mDesc.data(), si.mDesc.length());

		ptr = writestr(ptr, "type", 4);
		ptr = writestr(ptr, si.mType.data(), si.mType.length());

		ptr = writestr(ptr, "addr", 4);
		ptr = writestr(ptr, si.mAddr.data(), si.mAddr.length());

		ptr = writestr(ptr, "port", 4);
		ptr = writestr(ptr, si.mPort.data(), si.mPort.length());

		size_t size = ptr - outbuf;
		writel(outbuf + 8, size - 12);

		assert(size < sizeof(outbuf));

		//printf("ServiceManager: Sending a query reply packet(max: %d: index:%d size: %d) to %s\n",
		//       (int)count, i, (int)size, addr.c_str());

		mSock->sendTo(outbuf, size, addr, port);

		i++;
		it++;
	}
}

void ServiceManager::processQueryInfoPacket(char*		buf,
					    int			buflen,
					    const std::string&	addr,
					    unsigned short	port)
{
	char outbuf[512];
	char* ptr = outbuf;

	ptr[0] = 'Q';
	ptr[1] = 'I';
	ptr[2] = 'R';
	ptr[3] = 'P';
	ptr += 4;

	memcpy(ptr, &buf[4], 4);
	ptr += 4;

	ptr += 4;
	ptr = writel(ptr, mCookie);

	size_t count = mServiceInfoMap.size();
	ptr = writel(ptr, count);
	ptr = writestr(ptr, "name", 4);
	ptr = writestr(ptr, mName.data(), mName.length());

	size_t size = ptr - outbuf;
	assert(size < sizeof(outbuf));

	// payload size
	writel(outbuf + 8, size - 12);

	//printf("ServiceManager: Sending a query info reply packet(size: %d) to %s\n",
	//       (int)size, addr.c_str());

	mSock->sendTo(outbuf, size, addr, port);
}

void ServiceManager::processPacket(char*		buf,
				   int			buflen,
				   const std::string&	addr,
				   unsigned short	port)
{
	if (buflen < 12)
		return;

	std::string tag(&buf[0], &buf[4]);

	// printf ("ServiceManager: Received a packet(%s) from %s.\n", tag.c_str(), addr.c_str());

	if (tag == "ECHO")
		processEchoPacket(buf, buflen, addr, port);
	else if (tag == "QURY")
		processQueryPacket(buf, buflen, addr, port);
	else if (tag == "QRIF")
		processQueryInfoPacket(buf, buflen, addr, port);
}

void ServiceManager::server()
{
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

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		int ret = select(sock + 1, &rfds, &wfds, &efds, &tv);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret < 0)
			break;

		if (!FD_ISSET(sock, &rfds))
			continue;

		char buf[512];
		std::string addr;
		unsigned short port;

		ret = mSock->recvFrom(buf, 512, addr, port);
		if (ret <= 0)
			continue;
		processPacket(buf, ret, addr, port);
	}
}

bool ServiceManager::initialize()
{
	if (mInitialized)
		return true;


	mSock = new UDPSocket("", 11053);
	if (mSock->hasError())
	{
		delete mSock;
		mSock = 0;
		return false;
	}
	mSock->joinGroup("224.0.0.251");

	mDone = false;
	mThread = Thread::Create(serverProc, this);
	mInitialized = true;
	return true;
}

bool ServiceManager::registerService(const ServiceInfo& info)
{
	if (!mValid || !initialize())
		return true;

	ServiceInfoMap::iterator it = mServiceInfoMap.find(info.mName);
	if (it != mServiceInfoMap.end())
		return false;

	const size_t MAX_SIZE = 450;
	if (info.mName.length() + info.mType.length() + info.mDesc.length() +
	    info.mAddr.length() + info.mPort.length() > MAX_SIZE)
		return false;

	mServiceInfoMap.insert(ServiceInfoMap::value_type(info.mName, info));
	mCookie++;
	return true;
}

void ServiceManager::unregisterService(const ServiceInfo& info)
{
	if (!mValid)
		return;


	ServiceInfoMap::iterator it = mServiceInfoMap.find(info.mName);
	if (it == mServiceInfoMap.end())
		return;

	mServiceInfoMap.erase(it);
	mCookie++;
}

void ServiceManager::setName(const std::string& name)
{
	mName = name;
}
