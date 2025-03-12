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

extern int g_count;
int main()
{
	::timeBeginPeriod(1);


	int delayTime = 0;
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
		gameServer->CleanupSession();

		++g_count;
		//else
		//{
		//	delayTime = delayTime + abs(sleepTime);
		//	if (delayTime > TIME_PER_FRAME)
		//	{
		//		//프레임 로직 업데이트
		//		for (int i = 0; i < delayTime / TIME_PER_FRAME; i++)
		//		{
		//			gameServer->update();
		//		}
		//		delayTime = 0;
		//	}
		//}
		//세션정리

	}
	return 0;
}