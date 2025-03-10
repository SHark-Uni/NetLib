#include "NetLib.h"
#include "Logger.h"
#include "Session.h"

using namespace Common;
using namespace NetLib;

NetWorkLib::~NetWorkLib()
{
	closesocket(_ListenSocket);
	::WSACleanup();
}

eERROR_MESSAGE NetWorkLib::InitForTCP()
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
				_RecvProc();
			}

			if (FD_ISSET(curSession->GetSocket(), &writeSet))
			{
				//SendQueue에 복사된거 보내기.
				_SendProc();
			}
		}

		if (SesionStart_iter == SessionEnd_iter)
		{
			break;
		}
	}
	//진짜 Send하기

}

void NetWorkLib::SendUniCast(int sessionKey, char* message)
{

}

void NetWorkLib::SendBroadCast(char* message)
{

}

void NetWorkLib::SendBroadCast(int exceptSession, char* message)
{

}

void NetWorkLib::RecvProc()
{

}

void NetWorkLib::AcceptProc()
{

}

void NetWorkLib::SendProc()
{

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
