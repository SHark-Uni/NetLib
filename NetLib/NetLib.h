#pragma once

#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <unordered_map>


namespace NetLib
{
	class Session;
	class NetWorkLib
	{
	public:
		NetWorkLib() = default;
		virtual ~NetWorkLib();

		bool Init_For_TCP();
		/*=======
			TODO : RUDP
		===========*/
		//bool Init_For_UDP();

		void Process();

		void SendUniCast(int sessionKey, char* message);
		void SendBroadCast(char* message);
		void SendBroadCast(int exceptSession, char* message);

		/*========
		* ��ӹ޾Ƽ� ���� �������� ������ ���� 
		===========*/

		virtual void OnAcceptProc() = 0; 
		virtual void OnRecvProc() = 0;
		virtual void OnDestroyProc() = 0;
		/* TODO : ĳ���Ͱ� �����Ǵ� ���� �� ���� �غ���. */

	private:
		SOCKET _ListenSocket;

		void RecvProc();
		void AcceptProc();
		void SendProc();

		//���ǵ� 
		std::unordered_map<int, Session*> _Sessions;
	};
}