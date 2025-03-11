#pragma once

#include "NetLib.h"

namespace Core
{
	class Player;
	class GameServer : public NetLib::NetWorkLib
	{
	public:
		GameServer();
		virtual ~GameServer();
		void OnAcceptProc(const int key) override;
		void OnRecvProc(char* message, size_t messageLen, char* header, size_t hLen) override;

		void update();
	private:
		typedef int PLAYER_KEY;
		typedef int SESSION_KEY;
		enum
		{
			PLAYER_POOL_SIZE = 4096,
		};
		std::unordered_map<PLAYER_KEY, Player*> _Players;
		std::unordered_map<SESSION_KEY, PLAYER_KEY> _keys;
	};
}