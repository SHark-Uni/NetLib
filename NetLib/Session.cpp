#include "Session.h"
#include "MemoryPool.h"

using namespace NetLib;

void Session::InitSession(const SOCKET connectSocket, const SOCKADDR_IN& connectInfo, const int key)
{
	_Socket = connectSocket;
	_AddrInfo = connectInfo;
	_Alive = true;
	_Key = key;
	//TODO : 오브젝트 풀로 RINGBUFFER 만들어서 생성 / 반납
	_pRecvQueue = new CircularQueue(RECV_BUFFER_SIZE);
	_pSendQueue = new CircularQueue(SEND_BUFFER_SIZE);
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

