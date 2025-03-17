#include "NetLib.h"
#include "Logger.h"
#include "Session.h"
#include "MessageFormat.h"
#include "ObjectPool.h"

using namespace Common;
using namespace NetLib;

NetWorkLib::~NetWorkLib()
{
	//Session�� �޸� Ǯ�� �ݳ�. 
	for (auto& session : _Sessions)
	{
		Session* cur = session.second;
		cur->DestroySession();
		MemoryPool<Session, SESSION_POOL_SIZE>::getInstance().deAllocate(cur);
	}

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

	//�񵿱�
	ULONG on = 1;
	Flag = ::ioctlsocket(_ListenSocket, FIONBIO, &on);
	if (Flag != 0)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"Set nonblocking IO Error");
		return eERROR_MESSAGE::SET_SOCKET_NONBLOCKING_FAIL;
	}

	//����
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
	//�������� �о����
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
	//���ε�
	Flag = ::bind(_ListenSocket, reinterpret_cast<SOCKADDR*>(&addrInfo), sizeof(addrInfo));
	if (Flag == SOCKET_ERROR)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"BIND_SERVER_FAIL");
		return eERROR_MESSAGE::BIND_SERVER_FAIL;
	}
	//����
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

	TIMEVAL option = { 0,0 };

	const int SELECT_MAX_CNT = 63;
	int maxLoopCnt = 0;

	/*�̹� �������� �ο��鿡 ���ؼ��� ó������.*/
	auto SesionStart_iter = _Sessions.begin();
	auto iter = _Sessions.begin();
	auto SessionEnd_iter = _Sessions.end();

	Session* curSession;
	while (true)
	{
		FD_SET readSet;
		FD_SET writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		maxLoopCnt = 0;
		FD_SET(_ListenSocket, &readSet);
		/* TODO : 64���� ���ƾ���.MAX_LOOP_CNT�� 64���� �������ִ� �ڵ� �߰��ؾ���. */
		for (; iter != SessionEnd_iter && maxLoopCnt < SELECT_MAX_CNT; ++iter)
		{
			curSession = iter->second;
			FD_SET(curSession->GetSocket(), &readSet);
			if (curSession->CanSendData())
			{
				FD_SET(curSession->GetSocket(), &writeSet);
			}
			++maxLoopCnt;
		}

		Flag = ::select(0, &readSet, &writeSet, nullptr, &option);
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
			curSession = SesionStart_iter->second;
			if (FD_ISSET(curSession->GetSocket(), &readSet))
			{
				_RecvProc(curSession);
			}

			if (FD_ISSET(curSession->GetSocket(), &writeSet))
			{
				//SendQueue�� ����Ȱ� ������.
				_SendProc(curSession);
			}
		}

		if (SesionStart_iter == SessionEnd_iter)
		{
			break;
		}

	}
}

void NetWorkLib::_RecvProc(Session* session)
{
	int recvLen;
	int errorCode;

	//Reading �뵵�θ� RecvQ ����
	CircularQueue* const pRecvQ = session->_pRecvQueue;

	recvLen = ::recv(session->GetSocket(), pRecvQ->GetRearPtr(), pRecvQ->GetDirect_EnqueueSize(), 0);
	if (recvLen <= 0)
	{
		errorCode = ::WSAGetLastError();
		if (errorCode == WSAEWOULDBLOCK
			|| recvLen == 0
			|| errorCode == WSAECONNABORTED
			|| errorCode == WSAECONNRESET)
		{
#ifdef GAME_DEBUG
			printf("RECV ERROR NORMAL DISCONNECT!\n");
#endif
			OnDestroyProc(session->GetSessionKey());
			return;
		}
		//�ٸ� ������ ��ϵǴ� ���
#ifdef GAME_DEBUG
		printf("RECV ERROR UNUSAL DISCONNECT!\n");
#endif
		Logger::Logging(errorCode, __LINE__, L"Recv Error");
		OnDestroyProc(session->GetSessionKey());
		return;
	}
	pRecvQ->MoveRear(recvLen);

	// �ѹ� ���� �޽����� ��� ó���Ѵ�.
	while (true)
	{	//TODO : �� �κ��� �������� ���յǹ���. ���� ����
		int peekMessageLen;
		int payLoadLen;

		char tmp[3] = { 0 };
		peekMessageLen = pRecvQ->Peek(tmp, sizeof(header_t));
		if (peekMessageLen < sizeof(header_t))
		{
			break;
		}
		header_t* header = reinterpret_cast<header_t*>(tmp);
		if (header->_Code != SIGNITURE)
		{
			break;
		}

		payLoadLen = header->_PayloadLen;
		char buffer[32] = { 0, };
		peekMessageLen = pRecvQ->Peek(buffer, (payLoadLen + sizeof(header_t)));
		if (peekMessageLen < payLoadLen + sizeof(header_t))
		{
			break;
		}
		pRecvQ->Dequeue(buffer, payLoadLen + sizeof(header_t));
		int debugKey = session->GetSessionKey();
#ifdef GAME_DEBUG
		printf("============================================================\n");
		printf("SESSION Key : %d | In Network | \n", debugKey);
		printf("RECEIVE HEADER , CODE : %d | TYPE :%d | PAYLOADLEN : %d\n", header->_Code, header->_MessageType, header->_PayloadLen);
		printf("============================================================\n");
#endif
		OnRecvProc(buffer, tmp, sizeof(header_t), session->GetSessionKey());
	}
	return;
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

	//���������� Accpet
	//���� ����
	Session* newSession = MemoryPool<Session, SESSION_POOL_SIZE>::getInstance().allocate();
	int key = newSession->GenerateSessionKey();
	newSession->InitSession(connectSocket, connectInfo, key);
	_Sessions.insert({ key, newSession });

	OnAcceptProc(key);
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
		// send�� WOULDBLOCK�� L4�� �۽Ź��۰� ��á�ٴ°Ŵ�. �̰� ������ ������ �� á�ٴ°���. 
		// ������ �������� ������ ����ų�.. ��� �׳� �����ָ� ��.
		// �Ȳ�������� ������ �ֳ�?
#ifdef GAME_DEBUG
		printf("SEND ERROR\n");
#endif
		OnDestroyProc(session->GetSessionKey());
		return;
	}
	if (sendLen < sendQLen)
	{
#ifdef GAME_DEBUG
		printf("L7 BUFFER IS FULL DISCONNECT!\n");
#endif
		OnDestroyProc(session->GetSessionKey());
		return;
	}
	pSendQueue->MoveFront(sendLen);
}

void NetWorkLib::SendUniCast(const SESSION_KEY sessionKey, char* message, const size_t messageLen)
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

		if (enqueueLen < static_cast<int>(messageLen))
		{
#ifdef GAME_DEBUG
			printf("L7 BUFFER IS FULL DISCONNECT!\n");
#endif
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			OnDestroyProc(findSession->GetSessionKey());
			return;
		}
	}
	return;
}

void NetWorkLib::SendBroadCast(char* message, const size_t messageLen)
{
	int enqueueLen;
	for (auto& session : _Sessions)
	{
		if (session.second->GetConnection() == false)
		{
			continue;
		}

		CircularQueue* const curSendQ = session.second->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message, messageLen);
		if (enqueueLen < static_cast<int>(messageLen))
		{
#ifdef GAME_DEBUG
			printf("L7 BUFFER IS FULL DISCONNECT!\n");
#endif
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			OnDestroyProc(session.second->GetSessionKey());
			continue;
		}
	}
}

void NetWorkLib::SendBroadCast(SESSION_KEY exceptSession, char* message, const size_t messageLen)
{
	int enqueueLen;
	for (auto& session : _Sessions)
	{
		if (session.second->GetConnection() == false)
		{
			continue;
		}
		if (session.first == exceptSession)
		{
			continue;
		}

		CircularQueue* const curSendQ = session.second->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message, messageLen);

		if (enqueueLen < static_cast<int>(messageLen))
		{
#ifdef GAME_DEBUG
			printf("L7 BUFFER IS FULL DISCONNECT!\n");
#endif
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			OnDestroyProc(session.second->GetSessionKey());
			continue;
		}
	}
}

void NetWorkLib::Disconnect(SESSION_KEY sessionKey)
{
	//������ ���� �̻��� ����Ű�� �´ٸ�? ���� Ű�� ��������ΰ�?
	const auto& iter = _Sessions.find(sessionKey);
	Session* cur = iter->second;
	if (iter != _Sessions.end())
	{
		cur->SetDisconnect();
		return;
	}
	return;
}

void NetWorkLib::CleanupSession()
{
	auto& pool = MemoryPool<Session, SESSION_POOL_SIZE>::getInstance();
	auto& ringbufferPool = ObjectPool<CircularQueue, Session::POOL_SIZE>::getInstance();

	auto iter = _Sessions.begin();
	auto iter_e = _Sessions.end();

	for (; iter != iter_e; )
	{
		Session* cur = iter->second;
		int sessionKey = iter->first;
		if (cur->GetConnection() == false)
		{
			closesocket(cur->GetSocket());
			ringbufferPool.deAllocate(cur->_pRecvQueue);
			ringbufferPool.deAllocate(cur->_pSendQueue);
			pool.deAllocate(cur);
			iter = _Sessions.erase(iter);
			continue;
		}
		++iter;
	}
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

		//PORT  ��Ʈ����
		WCHAR* tmp = nullptr;
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
