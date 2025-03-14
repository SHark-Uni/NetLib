#pragma comment(lib, "winmm")
#define _WINSOCKAPI_

#include <iostream>
#include "MemoryPool.h"
#include "NetLib.h"
#include "Session.h"
#include "GameServer.h"

using namespace Common;
using namespace NetLib;
using namespace Core;

int main()
{
	::timeBeginPeriod(1);
	GameServer* gameServer = new GameServer();
	if (gameServer->Init() != eERROR_MESSAGE::SUCCESS)
	{
		return 0;
	}
	while (true)
	{
		//네트워크
		gameServer->Process();
		//프레임 로직
		gameServer->update();

		gameServer->cleanUpPlayer();
		gameServer->CleanupSession();
	}
	return 0;
}