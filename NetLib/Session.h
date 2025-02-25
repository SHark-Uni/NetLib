#pragma once
#define _WINSOCKAPI_

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

namespace NetLib
{
	class Session
	{
	public:
		Session();
		virtual ~Session();

		void CreateSession(SOCKET connectSocket, SOCKADDR_IN pConnectInfo);

		inline SOCKET GetSocket() const
		{
			return _Socket;
		}
		inline USHORT GetPort() const
		{
			return _Port;
		}
		void GetIP(WCHAR* out);
		
	private:
		SOCKET _Socket;
		WCHAR _IP[32];
		USHORT _Port;

		/*========
		* TODO : 송/수신 버퍼 -> 환형 큐로 변경
		============*/
		char* _SendBuffer[512];
		char* _RendBuffer[512];
	};
}