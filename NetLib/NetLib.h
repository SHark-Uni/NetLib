#pragma once
#pragma comment(lib, "Ws2_32.lib")

#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <unordered_map>

#include "ErrorMessage.h"
#include "Config.h"
#include "SerializeBuffer.h"

namespace NetLib
{
	using namespace Common;
	class Session;
	class NetWorkLib
	{
	public:
		NetWorkLib() = default;
		virtual ~NetWorkLib();
		Common::eERROR_MESSAGE Init();
		void Process();
		void CleanupSession();
	protected:
		typedef int SESSION_KEY;
		/*=======
			TODO : RUDP
		===========*/
		//bool InitForUDP();
		void SendUniCast(const SESSION_KEY sessionKey, SerializeBuffer* message, const size_t messageLen);
		void SendBroadCast(SerializeBuffer* message, const size_t messageLen);
		void SendBroadCast(SESSION_KEY exceptSession, SerializeBuffer* message, const size_t messageLen);

		virtual void OnAcceptProc(const SESSION_KEY key) = 0;
		virtual void OnRecvProc(SerializeBuffer* message, const char msgType, SESSION_KEY sessionkey) = 0;
		virtual void OnDestroyProc(const SESSION_KEY key) = 0;
		/* ���� ������û. */
		void Disconnect(SESSION_KEY sessionKey);

	private:
		std::unordered_map<SESSION_KEY, Session*> _Sessions;
		SOCKET _ListenSocket;

		void _AcceptProc();
		void _RecvProc(Session* session);
		void _SendProc(Session* session);
		bool ReadConfig();
		CONFIG_t _ServerConfig;
	};
}