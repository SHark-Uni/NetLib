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
	//MemoryPool<Player, POOL_SIZE>& sessionPool = MemoryPool<Player, POOL_SIZE>::getInstance();
	::timeBeginPeriod(1);

	DWORD nextTick;
	int sleepTime;
	int delayTime = 0;
	GameServer* gameServer = new GameServer();
	gameServer->Init();
	while (true)
	{
		//네트워크
		gameServer->Process();
		nextTick = timeGetTime();
		//프레임 로직 
		gameServer->update();
		nextTick += TIME_PER_FRAME;
		sleepTime = nextTick - timeGetTime();
		if (sleepTime > 0)
		{
			Sleep(sleepTime);
		}
		else
		{
			delayTime = delayTime + abs(sleepTime);
			if (delayTime > TIME_PER_FRAME)
			{
				//프레임 로직 업데이트
				for (int i = 0; i < delayTime / TIME_PER_FRAME; i++)
				{
					gameServer->update();
				}
				delayTime = 0;
			}
		}
		//세션정리
		gameServer->CleanupSession();
	}
	return 0;
}