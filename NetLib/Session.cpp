#include "Session.h"
#include "ObjectPool.h"

using namespace NetLib;

constexpr size_t POOL_SIZE = 2048;
constexpr int QSIZE = 4096;

void Session::InitSession(const SOCKET connectSocket, const SOCKADDR_IN& connectInfo, const int key)
{
	_Socket = connectSocket;
	_AddrInfo = connectInfo;
	_Alive = true;
	_Key = key;
	//TODO : 오브젝트 풀로 RINGBUFFER 만들어서 생성 / 반납


	auto& pool = ObjectPool<CircularQueue, POOL_SIZE>::getInstance();

	_pRecvQueue = pool.allocate_reuse(QUEUE_SIZE);
	_pSendQueue = pool.allocate_reuse(QUEUE_SIZE);

	//_pRecvQueue->clear();
	//_pSendQueue->clear();

	//_pRecvQueue = new CircularQueue(QUEUE_SIZE);
	//_pSendQueue = new CircularQueue(QUEUE_SIZE);
	
}

void NetLib::Session::DestroySession()
{

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

