#pragma once

#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <unordered_map>

#include "ErrorMessage.h"
#include "Config.h"

namespace NetLib
{
	class Session;
	class NetWorkLib
	{
	public:
		NetWorkLib() = default;
		virtual ~NetWorkLib();

		Common::eERROR_MESSEAGE InitForTCP();
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

		/* TODO : ĳ���Ͱ� �����Ǵ� ���� �� ���� �غ���. */
		virtual void OnDestroyProc() = 0;
	
	private:
		SOCKET _ListenSocket;
		Common::CONFIG_t _ServerConfig;
		void RecvProc();
		void AcceptProc();
		void SendProc();

		bool ReadConfig();
		//���ǵ� 
		std::unordered_map<int, Session*> _Sessions;
	};
}