#include "Session.h"
#include "MemoryPool.h"

using namespace NetLib;

Session::Session()
{

}

Session::~Session()
{
	MemoryPool<CircularQueue, RING_BUFFER_POOL_SIZE>& pool = MemoryPool<CircularQueue, RING_BUFFER_POOL_SIZE>::getInstance();
	pool.deAllocate(_pSendQueue);
	pool.deAllocate(_pRecvQueue);
}

void Session::CreateSession(const SOCKET connectSocket, const SOCKADDR_IN& connectInfo)
{
	_Socket = connectSocket;
	_AddrInfo = connectInfo;
	_Alive = true;
	
	MemoryPool<CircularQueue, RING_BUFFER_POOL_SIZE>& pool = MemoryPool<CircularQueue, RING_BUFFER_POOL_SIZE>::getInstance();

	_pSendQueue = pool.allocate_constructor(SEND_BUFFER_SIZE);
	_pRecvQueue = pool.allocate_constructor(RECV_BUFFER_SIZE);
}

void Session::GetIP(WCHAR* out, size_t buffersize)
{
	InetNtop(AF_INET, &_AddrInfo.sin_addr, out, sizeof(buffersize));
	return;
}

USHORT Session::GetPort()
{
	short ret = ntohs(_AddrInfo.sin_port);
	return ret;
}

