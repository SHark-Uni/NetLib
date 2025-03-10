#pragma once

#include "NetLib.h"

namespace Core
{
	class GameServer : public NetLib::NetWorkLib
	{
	public:
		GameServer() = default;
		virtual ~GameServer();
		//NetLIB -> Server 
		void OnAcceptProc(char* message) override;
		void OnRecvProc(char* message) override;

		void update();

		/* TODO : 캐릭터가 삭제되는 경우는 더 생각 해보자. */
		//SERVER -> NETLIB
		void OnDestroyProc() override;
	private:
		/* TODO PLAYER 관리 개체 만들어야함.*/
	};
}