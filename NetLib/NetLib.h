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

		//�̵�, ���� �����ڵ���?
	protected:
		Common::eERROR_MESSAGE Init();
		/*=======
			TODO : RUDP
		===========*/
		//bool InitForUDP();

		void Process();
		void SendUniCast(const int sessionKey, char* message, const size_t messageLen);
		void SendBroadCast(char* message, const size_t messageLen);
		void SendBroadCast(int exceptSession, char* message, const size_t messageLen);

		/*========
		* ��ӹ޾Ƽ� ���� �������� ������ ���� 
		===========*/

		virtual void OnAcceptProc(const int key) = 0; 
		virtual void OnRecvProc(char* message, size_t messageLen, char* header, size_t hLen) = 0;

		/* ���� ������û. */
		void Disconnect(int sessionKey);
		/*TODO */
		void CleanupSession();
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