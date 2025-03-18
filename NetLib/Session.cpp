#include "Session.h"
#include "ObjectPool.h"

using namespace NetLib;

void Session::InitSession(const SOCKET connectSocket, const SOCKADDR_IN& connectInfo, const int key)
{
	auto& pool = ObjectPool<CircularQueue, POOL_SIZE>::getInstance();

	_Socket = connectSocket;
	_AddrInfo = connectInfo;
	_Alive = true;
	_Key = key;
	
	_pRecvQueue = pool.allocate_reuse(QUEUE_SIZE);
	_pSendQueue = pool.allocate_reuse(QUEUE_SIZE);

	_pRecvQueue->clear();
	_pSendQueue->clear();
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

