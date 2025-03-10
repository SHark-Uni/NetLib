#pragma once

#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <unordered_map>

#include "ErrorMessage.h"
#include "Config.h"
#include "MemoryPool.h"

namespace NetLib
{
	class Session;
	class NetWorkLib
	{
	public:
		NetWorkLib() = default;
		virtual ~NetWorkLib();
	protected:
		Common::eERROR_MESSAGE Init();
		/*=======
			TODO : RUDP
		===========*/
		//bool InitForUDP();

		void Process();
		void SendUniCast(const int sessionKey, char* message);
		void SendBroadCast(char* message);
		void SendBroadCast(int exceptSession, char* message);

		/*========
		* ��ӹ޾Ƽ� ���� �������� ������ ���� 
		===========*/

		virtual void OnAcceptProc(const int key) = 0; 
		virtual void OnRecvProc(char* message) = 0;

		/* TODO : ĳ���Ͱ� �����Ǵ� ���� �� ���� �غ���. */
		virtual void OnDestroyProc() = 0;
		void Disconnect(int sessionKey);
		
	private:
		//���ǵ� 
		std::unordered_map<int, Session*> _Sessions;
		SOCKET _ListenSocket;
		enum
		{
			SESSION_POOL_SIZE = 8192,
		};
		void _AcceptProc();
		void _RecvProc(Session* session);
		void _SendProc(Session* session);
		bool ReadConfig();
		Common::CONFIG_t _ServerConfig;
	};
}