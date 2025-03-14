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
	DWORD nextTick;
	int sleepTime;

	nextTick = timeGetTime();
	while (true)
	{
		//네트워크
		gameServer->Process();
		//프레임 로직
		gameServer->update();
		gameServer->cleanUpPlayer();
		gameServer->CleanupSession();

		nextTick += TIME_PER_FRAME;
		sleepTime = nextTick - timeGetTime();
		if (sleepTime > 0)
		{
			Sleep(sleepTime);
		}
		//TODO : 프레임이 떨어진다면, fixed update 방식 근데, 
	}
	return 0;
}