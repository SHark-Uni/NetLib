#pragma once
#define _WINSOCKAPI_

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "CircularQueue.h"

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

		inline bool CanSendData()
		{
			return (_pSendQueue->GetCurrentSize() > 0);
		}

		void GetIP(WCHAR* out);
		
	private:
		SOCKET _Socket;
		WCHAR _IP[32];
		USHORT _Port;

		/*========
		* TODO : ��/���� ���� -> ȯ�� ť�� ����
		============*/
		CircularQueue* _pSendQueue;
		CircularQueue* _pRecvQueue;
	};
}