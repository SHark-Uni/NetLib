#pragma once

#include "NetLib.h"

namespace Core
{
	class Player;
	class GameServer : public NetLib::NetWorkLib
	{
	public:
		typedef int PLAYER_KEY;
		typedef int SESSION_KEY;
		GameServer();
		virtual ~GameServer();
		void OnAcceptProc(const int key) override;
		void OnRecvProc(char* message, char* header, size_t hLen, SESSION_KEY key) override;

		/*=== ÄÁÅÙÃ÷ ====*/
		void ReqMoveStartProc(char* message, const SESSION_KEY key);
		void ReqMoveStopProc(char* message, const SESSION_KEY key);
		void ReqAttackLeftHandProc(char* message, const SESSION_KEY key);
		void ReqAttackRightHandProc(char* message, const SESSION_KEY key);
		void ReqAttackKickProc(char* message, const SESSION_KEY key);

		void update();
	private:

		enum
		{
			PLAYER_POOL_SIZE = 4096,
		};
		std::unordered_map<PLAYER_KEY, Player*> _Players;
		std::unordered_map<SESSION_KEY, PLAYER_KEY> _keys;
	};
}