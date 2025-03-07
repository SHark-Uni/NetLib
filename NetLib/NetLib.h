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
		* 상속받아서 게임 서버에서 컨텐츠 구현 
		===========*/

		virtual void OnAcceptProc() = 0; 
		virtual void OnRecvProc() = 0;
		virtual void OnDestroyProc() = 0;
		/* TODO : 캐릭터가 삭제되는 경우는 더 생각 해보자. */

	private:
		SOCKET _ListenSocket;

		void RecvProc();
		void AcceptProc();
		void SendProc();

		//세션들 
		std::unordered_map<int, Session*> _Sessions;
	};
}