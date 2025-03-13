#pragma once
#pragma comment(lib, "Ws2_32.lib")

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
		Common::eERROR_MESSAGE Init();
		void Process();
		void CleanupSession();
		//�̵�, ���� �����ڵ���?
	protected:
		typedef int SESSION_KEY;
		/*=======
			TODO : RUDP
		===========*/
		//bool InitForUDP();
		void SendUniCast(const SESSION_KEY sessionKey, char* message, const size_t messageLen);
		void SendBroadCast(char* message, const size_t messageLen);
		void SendBroadCast(SESSION_KEY exceptSession, char* message, const size_t messageLen);
		/*========
		* ��ӹ޾Ƽ� ���� �������� ������ ���� 
		===========*/
		virtual void OnAcceptProc(const SESSION_KEY key) = 0;
		virtual void OnRecvProc(char* message, char* header, size_t hLen, SESSION_KEY sessionkey) = 0;
		virtual void OnDestroyProc(const SESSION_KEY key) = 0;
		/* ���� ������û. */
		void Disconnect(SESSION_KEY sessionKey);
		/*TODO */
	private:
		//���ǵ� 
		std::unordered_map<int, Session*> _Sessions;
		SOCKET _ListenSocket;
		enum
		{
			SESSION_POOL_SIZE = 4096,
		};
		void _AcceptProc();
		void _RecvProc(Session* session);
		void _SendProc(Session* session);
		bool ReadConfig();
		Common::CONFIG_t _ServerConfig;
	};
}