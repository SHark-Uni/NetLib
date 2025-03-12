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
extern int g_count;
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
		/*=======
			TODO : RUDP
		===========*/
		//bool InitForUDP();
		void SendUniCast(const int sessionKey, char* message, const size_t messageLen);
		void SendBroadCast(char* message, const size_t messageLen);
		void SendBroadCast(int exceptSession, char* message, const size_t messageLen);
		/*========
		* ��ӹ޾Ƽ� ���� �������� ������ ���� 
		===========*/
		virtual void OnAcceptProc(const int key) = 0; 
		virtual void OnRecvProc(char* message, char* header, size_t hLen, int sessionkey) = 0;
		/* ���� ������û. */
		void Disconnect(int sessionKey);
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