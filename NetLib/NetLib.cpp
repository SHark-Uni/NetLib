#include "NetLib.h"
#include "Logger.h"
#include "Session.h"


using namespace Common;
using namespace NetLib;

template<typename T, size_t BucketSize>
MemoryPool<T, BucketSize> MemoryPool<T, BucketSize>::_singleton;

NetWorkLib::~NetWorkLib()
{
	closesocket(_ListenSocket);
	::WSACleanup();
}

eERROR_MESSAGE NetWorkLib::Init()
{
	WSADATA wsaData;

	int Flag;
	int errorCode;
	Flag = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Flag != 0)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"WSA_START_UP_FAIL");
		return eERROR_MESSAGE::WSA_START_FAIL;
	}

	_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (_ListenSocket == INVALID_SOCKET)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"MAKE_SOCKET_ERROR");
		return eERROR_MESSAGE::MAKE_SOCKET_FAIL;
	}

	//비동기
	ULONG on = 1;
	Flag = ::ioctlsocket(_ListenSocket, FIONBIO, &on);
	if (Flag != 0)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"Set nonblocking IO Error");
		return eERROR_MESSAGE::SET_SOCKET_NONBLOCKING_FAIL;
	}

	//링거
	LINGER option;
	option.l_onoff = 1;
	option.l_linger = 0;
	Flag = ::setsockopt(_ListenSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&option), sizeof(option));
	if (Flag != 0)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"SET LINGER ERROR");
		return eERROR_MESSAGE::SET_LINGER_FAIL;
	}
	//서버정보 읽어오기
	if (!ReadConfig())
	{
		Logger::Logging(-1, __LINE__, L"READ SERVER_CONFIG_ERROR");
		return eERROR_MESSAGE::SET_SERVER_CONFIG_FAIL;
	}

	SOCKADDR_IN addrInfo;
	::ZeroMemory(&addrInfo, sizeof(addrInfo));
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_port = htons(_ServerConfig._Port);
	::InetPton(AF_INET, L"0.0.0.0", &addrInfo.sin_addr);
	//바인딩
	Flag = ::bind(_ListenSocket, reinterpret_cast<SOCKADDR*>(&addrInfo), sizeof(addrInfo));
	if (Flag == SOCKET_ERROR)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"BIND_SERVER_FAIL");
		return eERROR_MESSAGE::BIND_SERVER_FAIL;
	}
	//리슨
	Flag = ::listen(_ListenSocket, SOMAXCONN_HINT(65535));
	if (Flag == SOCKET_ERROR)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"LISTEN_ERROR");
		return eERROR_MESSAGE::SET_LISTEN_FAIL;
	}

	_Sessions.reserve(4096);
	return eERROR_MESSAGE::SUCCESS;
}

void NetWorkLib::Process()
{
	int Flag;
	FD_SET readSet;
	FD_SET writeSet;
	TIMEVAL option = { 0,0 };

	const int SELECT_MAX_CNT = 63;
	int maxLoopCnt = SELECT_MAX_CNT;
	
	/*이번 프레임의 인원들에 대해서만 처리해줌.*/
	auto SesionStart_iter = _Sessions.begin();
	auto iter = _Sessions.begin();
	auto SessionEnd_iter = _Sessions.end();

	while (true)
	{
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		maxLoopCnt = SELECT_MAX_CNT;
		FD_SET(_ListenSocket, &readSet);
		/* TODO : 64번씩 돌아야함.MAX_LOOP_CNT로 64번씩 통제해주는 코드 추가해야함. */
		for (; iter != SessionEnd_iter && maxLoopCnt < 0; ++iter)
		{
			Session* curSession = iter->second;
			FD_SET(curSession->GetSocket(), &readSet);
			if (curSession->CanSendData())
			{
				FD_SET(curSession->GetSocket(), &writeSet);
			}
			--maxLoopCnt;
		}

		Flag = ::select(0, &readSet, nullptr, nullptr, &option);
		if (Flag == SOCKET_ERROR)
		{
			Logger::Logging(static_cast<int>(eERROR_MESSAGE::SELECT_FAIL), __LINE__, L"SELECT ERROR");
			DebugBreak();
		}
		if (FD_ISSET(_ListenSocket, &readSet))
		{
			_AcceptProc();
		}

		for (; SesionStart_iter != iter; ++SesionStart_iter)
		{
			Session* curSession = SesionStart_iter->second;
			if (FD_ISSET(curSession->GetSocket(), &readSet))
			{
				_RecvProc(curSession);
			}

			if (FD_ISSET(curSession->GetSocket(), &writeSet))
			{
				//SendQueue에 복사된거 보내기.
				_SendProc(curSession);
			}
		}

		if (SesionStart_iter == SessionEnd_iter)
		{
			break;
		}
	}
	//진짜 Send하기

}

void NetWorkLib::_RecvProc(Session* session)
{
	//해당 세션의 RingBuffer 수신버퍼의 주소를 그냥 박음.(Rear의 주소를 줘야겠지)
	int recvLen;
	int errorCode;

	//Reading 용도로만 RecvQ 쓰기
	CircularQueue* const pRecvQ = session->_pRecvQueue;

	recvLen = ::recv(session->GetSocket(), pRecvQ->GetRearPtr(), pRecvQ->GetDirect_EnqueueSize(), 0);
	if (recvLen <= 0)
	{
		errorCode = ::WSAGetLastError();
		if (errorCode == WSAEWOULDBLOCK
			|| errorCode == WSAECONNABORTED
			|| errorCode == WSAECONNRESET)
		{
			return;
		}

		//다른 에러가 기록되는 경우
		Logger::Logging(errorCode, __LINE__, L"Recv Error");
		session->SetDisconnect();
		return;
	}
	pRecvQ->MoveRear(recvLen);	
	//OnRecvProc() 호출
	//Header체크
	//Header Peek
	//완성되었다면, header의 길이확인
	//header의 길이 확인 후, payload완성여부 확인

	//payload완성되었다면 Dequeue
}

void NetWorkLib::_AcceptProc()
{
	int errorCode;
	SOCKET connectSocket;
	SOCKADDR_IN connectInfo;
	ZeroMemory(&connectInfo, sizeof(connectInfo));
	int conncetLen = sizeof(connectInfo);

	connectSocket = ::accept(_ListenSocket, reinterpret_cast<SOCKADDR*>(&connectInfo), &conncetLen);
	if (connectSocket == INVALID_SOCKET)
	{
		errorCode = ::WSAGetLastError();
		if (errorCode != WSAEWOULDBLOCK)
		{
			return;
		}
		Logger::Logging(errorCode, __LINE__, L"ACCPET Error");
		return;
	}

	//성공적으로 Accpet
	
	//세션 생성
	Session* newSession = MemoryPool<Session, SESSION_POOL_SIZE>::getInstance().allocate();
	newSession->CreateSession(connectSocket, connectInfo);
	int key = newSession->GenerateSessionKey();
	_Sessions.insert({ key, newSession });

	OnAcceptProc(key);
	//onAcceptProc에서 할일 
	//1. 플레이어 생성 (Contents코드에서
	//2. 다른 플레이어에게 내 플레이어 생성 메시지 보내기
	//3. 기존 플레이어들 나에게 생성 메시지 보내기

}

void NetWorkLib::_SendProc(Session* session)
{
	int sendLen;
	int errorCode;
	CircularQueue* const pSendQueue = session->_pSendQueue;
	int sendQLen = pSendQueue->GetDirect_DequeueSize();
	
	sendLen = ::send(session->GetSocket(), pSendQueue->GetFrontPtr(), sendQLen, 0);
	if (sendLen == SOCKET_ERROR)
	{
		//send시 WOULDBLOCK는 L4의 송신버퍼가 꽉찼다는거다. 이건 상대방의 수신이 다 찼다는거임. 
		//나머지 에러들은 연결이 끊겼거나.. 등에는 그냥 끊어주면 됨.
		// 안끊어줘야할 사유가 있나?
		session->SetDisconnect();
		return;
	}
	// L7버퍼에서 L4로 원하는 만큼 복사 x 
	if (sendLen < sendQLen)
	{
		session->SetDisconnect();
		return;
	}
	pSendQueue->MoveFront(sendLen);
}

void NetWorkLib::SendUniCast(const int sessionKey, char* message, const size_t messageLen)
{
	int enqueueLen;
	const auto& iter = _Sessions.find(sessionKey);
	if (iter != _Sessions.end())
	{
		Session* findSession = iter->second;
		if (findSession->GetConnection() == false)
		{
			return;
		}
		
		CircularQueue* const curSendQ = findSession->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message, messageLen);
		// 요청한 만큼 복사를 못함. L7버퍼가 꽉 차는 경우. 
		// 이때, 끊어주기로함 우리는. 왜? => 꼭 기억해야함. 
		if (enqueueLen < static_cast<int>(messageLen))
		{
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			findSession->SetDisconnect();
			return;
		}
	}
	return;
}

void NetWorkLib::SendBroadCast(char* message, const size_t messageLen)
{
	int enqueueLen;
	for (const auto& session : _Sessions)
	{
		Session* cur = session.second;
		if (cur->GetConnection() == false)
		{
			continue;
		}

		CircularQueue* const curSendQ = cur->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message, messageLen);
		if (enqueueLen < static_cast<int>(messageLen))
		{
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			cur->SetDisconnect();
			continue;
		}
	}
}

void NetWorkLib::SendBroadCast(int exceptSession, char* message, const size_t messageLen)
{
	int enqueueLen;
	for (const auto& session : _Sessions)
	{
		int curkey = session.first;
		Session* cur = session.second;

		if (cur->GetConnection() == false)
		{
			continue;
		}
		if (curkey == exceptSession)
		{
			continue;
		}

		CircularQueue* const curSendQ = cur->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message, messageLen);
		if (enqueueLen < static_cast<int>(messageLen))
		{
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			cur->SetDisconnect();
			continue;
		}
	}
}

void NetWorkLib::Disconnect(int sessionKey)
{
	//서버로 부터 이상한 세션키가 온다면? 세션 키도 관리대상인가?
	const auto& iter = _Sessions.find(sessionKey);
	if (iter != _Sessions.end())
	{
		iter->second->SetDisconnect();
		return;
	}
	return;
}

bool NetWorkLib::ReadConfig()
{
	FILE* fp;
	_wfopen_s(&fp, L"ServerConfig.txt", L"rb");
	if (fp == NULL)
	{
		return false;
	}

	const WCHAR* delims = L"\t, \r\n";
	const int BUFFER_SIZE = 128;
	WCHAR buffer[BUFFER_SIZE] = { 0, };
	WCHAR* Token;
	//Read Header
	fgetws(buffer, BUFFER_SIZE, fp);
	memset(buffer, 0, sizeof(buffer));
	//Read {
	fgetws(buffer, BUFFER_SIZE, fp);
	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		if (fgetws(buffer, BUFFER_SIZE, fp) == nullptr)
		{
			break;
		}

		//PORT  포트정보
		WCHAR* tmp;
		Token = wcstok_s(buffer, delims, &tmp);

		if (wcscmp(L"PORT", Token) == 0)
		{
			Token = wcstok_s(NULL, delims, &tmp);
			swscanf_s(Token, L"%hd", &_ServerConfig._Port);
			continue;
		}
	}
	fclose(fp);
	return true;
}
