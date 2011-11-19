#include "SexySocket.h"

#ifdef WIN32
#include <winsock.h>         // For socket(), connect(), send(), and recv()
#define socklen_t int
typedef char raw_type;       // Type used for raw data on this platform

#define errno        WSAGetLastError()
#define EINTR        WSAEINTR
#define EINPROGRESS  WSAEINPROGRESS
#define EWOULDBLOCK  WSAEWOULDBLOCK
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#else
#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>      // For sockaddr_in
typedef void raw_type;       // Type used for raw data on this platform

#include <errno.h>             // For errno
#endif

#include <string.h>
#include <stdlib.h>

using namespace std;

#ifdef WIN32
static bool initialized = false;
#endif

using namespace Sexy;

// Function to fill in address structure given an address and port
static bool fillAddr(const string &address, unsigned short port,
                     sockaddr_in &addr)
{
	memset(&addr, 0, sizeof(addr));  // Zero out address structure
	addr.sin_family = AF_INET;       // Internet address

	hostent *host;  // Resolve name
	host = gethostbyname(address.c_str());
	if (!host)
		return false;
	addr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
	addr.sin_port = htons(port);     // Assign port in network byte order
}

// Socket Code

Socket::Socket(int type, int protocol)
{
#ifdef WIN32
	if (!initialized)
	{
		WORD wVersionRequested;
		WSADATA wsaData;

		wVersionRequested = MAKEWORD(2, 0);              // Request WinSock v2.0
		WSAStartup(wVersionRequested, &wsaData);
		initialized = true;
	}
#endif

	// Make a new socket
	mSock = socket(PF_INET, type, protocol);
	mHasError = mSock >=0;
}

Socket::Socket(int sockDesc)
{
	this->mSock = sockDesc;
}

Socket::~Socket() {
#ifdef WIN32
	::closesocket(mSock);
#else
	::close(mSock);
#endif
	mSock = -1;
}

bool Socket::hasError()
{
	return mHasError;
}

string Socket::getLocalAddress()
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(mSock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
		return "";
	return inet_ntoa(addr.sin_addr);
}

unsigned Socket::getLocalPort()
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getsockname(mSock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
		return -1;
	return ntohs(addr.sin_port);
}

bool Socket::setLocalPort(unsigned short localPort)
{
	// Bind the socket to its port
	sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(localPort);

	if (bind(mSock, (sockaddr *) &localAddr, sizeof(sockaddr_in)) < 0)
		return false;

	return true;
}

bool Socket::setLocalAddressAndPort(const string &localAddress,
				    unsigned short localPort)
{
	// Get the address of the requested host
	sockaddr_in localAddr;
	fillAddr(localAddress, localPort, localAddr);

	if (bind(mSock, (sockaddr *) &localAddr, sizeof(sockaddr_in)) < 0)
		return false;

	return true;
}

void Socket::cleanup()
{
#ifdef WIN32
	if (initialized)
	{
		WSACleanup();
		initialized = false;
	}
#endif
}

unsigned short Socket::resolveService(const string &service,
                                      const string &protocol)
{
	struct servent *serv;        /* Structure containing service information */

	serv = getservbyname(service.c_str(), protocol.c_str());
	if (serv == NULL)
		return atoi(service.c_str());  /* Service is port number */
	return ntohs(serv->s_port);    /* Found port (network byte order) by name */
}

// CommunicatingSocket Code

CommunicatingSocket::CommunicatingSocket(int type, int protocol)
	: Socket(type, protocol)
{
}

CommunicatingSocket::CommunicatingSocket(int newConnSD) : Socket(newConnSD)
{
}

bool CommunicatingSocket::connect(const string &foreignAddress,
				  unsigned short foreignPort)
{
	// Get the address of the requested host
	sockaddr_in destAddr;
	fillAddr(foreignAddress, foreignPort, destAddr);

	// Try to connect to the given port
	if (::connect(mSock, (sockaddr *) &destAddr, sizeof(destAddr)) < 0)
		return false;
	return true;
}

bool CommunicatingSocket::send(const void *buffer, int bufferLen)
{
	if (::send(mSock, (raw_type *) buffer, bufferLen, 0) < 0)
		return false;
	return true;
}

int CommunicatingSocket::recv(void *buffer, int bufferLen)
{
	int ret;
	ret = ::recv(mSock, (raw_type *) buffer, bufferLen, 0);
	if (ret < 0)
		return -1;
	return ret;
}

string CommunicatingSocket::getForeignAddress()
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getpeername(mSock, (sockaddr *) &addr,(socklen_t *) &addr_len) < 0)
		return "";
	return inet_ntoa(addr.sin_addr);
}

unsigned CommunicatingSocket::getForeignPort()
{
	sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);

	if (getpeername(mSock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
		return -1;
	return ntohs(addr.sin_port);
}

// TCPSocket Code

TCPSocket::TCPSocket() : CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP)
{
}

TCPSocket::TCPSocket(const string &foreignAddress, unsigned short foreignPort)
	: CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP)
{
	connect(foreignAddress, foreignPort);
}

TCPSocket::TCPSocket(int newConnSD) : CommunicatingSocket(newConnSD)
{
}

// TCPServerSocket Code

TCPServerSocket::TCPServerSocket(unsigned short localPort, int queueLen)
	: Socket(SOCK_STREAM, IPPROTO_TCP)
{
	setLocalPort(localPort);
	setListen(queueLen);
}

TCPServerSocket::TCPServerSocket(const string &localAddress,
				 unsigned short localPort, int queueLen)
	: Socket(SOCK_STREAM, IPPROTO_TCP)
{
	setLocalAddressAndPort(localAddress, localPort);
	setListen(queueLen);
}

TCPSocket *TCPServerSocket::accept()
{
	int newConnSD;

	newConnSD = ::accept(mSock, NULL, 0);
	if (newConnSD < 0)
		return 0;
	return new TCPSocket(newConnSD);
}

bool TCPServerSocket::setListen(int queueLen)
{
	if (listen(mSock, queueLen) < 0)
		return false;
	return true;
}

// UDPSocket Code

UDPSocket::UDPSocket() :
	CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
	setBroadcast();
}

UDPSocket::UDPSocket(unsigned short localPort) :
	CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
	setLocalPort(localPort);
	setBroadcast();
}

UDPSocket::UDPSocket(const string &localAddress, unsigned short localPort)
	: CommunicatingSocket(SOCK_DGRAM, IPPROTO_UDP)
{
	setLocalAddressAndPort(localAddress, localPort);
	setBroadcast();
}

void UDPSocket::setBroadcast()
{
	// If this fails, we'll hear about it when we try to send.  This will allow
	// system that cannot broadcast to continue if they don't plan to broadcast
	int broadcastPermission = 1;
	setsockopt(mSock, SOL_SOCKET, SO_BROADCAST,
		   (raw_type *) &broadcastPermission, sizeof(broadcastPermission));
}

bool UDPSocket::disconnect()
{
	sockaddr_in nullAddr;
	memset(&nullAddr, 0, sizeof(nullAddr));
	nullAddr.sin_family = AF_UNSPEC;

	// Try to disconnect
	if (::connect(mSock, (sockaddr *) &nullAddr, sizeof(nullAddr)) < 0) {
		if (errno != EAFNOSUPPORT)
			return false;
	}

	return true;
}

bool UDPSocket::sendTo(const void *buffer, int bufferLen,
		       const string &foreignAddress, unsigned short foreignPort)
{
	sockaddr_in destAddr;
	fillAddr(foreignAddress, foreignPort, destAddr);

	// Write out the whole buffer as a single message.
	if (sendto(mSock, (raw_type *) buffer, bufferLen, 0,
		   (sockaddr *) &destAddr, sizeof(destAddr)) != bufferLen)
		return false;

	return true;
}

int UDPSocket::recvFrom(void *buffer, int bufferLen, string &sourceAddress,
			unsigned short &sourcePort)
{
	sockaddr_in clntAddr;
	socklen_t addrLen = sizeof(clntAddr);
	int ret;

	ret = recvfrom(mSock, (raw_type *) buffer, bufferLen, 0,
		       (sockaddr *) &clntAddr, (socklen_t *) &addrLen);
	if (ret < 0)
		return -1;

	sourceAddress = inet_ntoa(clntAddr.sin_addr);
	sourcePort = ntohs(clntAddr.sin_port);

	return ret;
}

bool UDPSocket::setMulticastTTL(unsigned char multicastTTL)
{
	if (setsockopt(mSock, IPPROTO_IP, IP_MULTICAST_TTL,
		       (raw_type *) &multicastTTL, sizeof(multicastTTL)) < 0)
		return false;

	return true;
}

bool UDPSocket::joinGroup(const string &multicastGroup)
{
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(mSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		       (raw_type *) &multicastRequest,
		       sizeof(multicastRequest)) < 0)
		return false;

	return true;
}

bool UDPSocket::leaveGroup(const string &multicastGroup)
{
	struct ip_mreq multicastRequest;

	multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastGroup.c_str());
	multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(mSock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		       (raw_type *) &multicastRequest,
		       sizeof(multicastRequest)) < 0)
		return false;

	return true;
}
