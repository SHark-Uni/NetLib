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

		/* TODO : ĳ���Ͱ� �����Ǵ� ���� �� ���� �غ���. */
		//SERVER -> NETLIB
		void OnDestroyProc() override;
	private:
		/* TODO PLAYER ���� ��ü ��������.*/
	};
}