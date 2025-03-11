#include "GameServer.h"
#include "MessageFormat.h"
#include "MessageBuilder.h"
#include "MemoryPool.h"
#include "Player.h"

using namespace Core;
using namespace Common;

GameServer::GameServer()
{
	_Players.reserve(1024);
	_keys.reserve(1024);
}

GameServer::~GameServer()
{

}
/*==================================
onAcceptProc에서 할일
1. 플레이어 생성 (Contents코드에서
2. 다른 플레이어에게 내 플레이어 생성 메시지 보내기
3. 기존 플레이어들 나에게 생성 메시지 보내기
===================================*/
void GameServer::OnAcceptProc(const int key)
{
	//1. 플레이어 생성
	int playerKey;
	MemoryPool<Player, PLAYER_POOL_SIZE>& pool = MemoryPool<Player, PLAYER_POOL_SIZE>::getInstance();
	Player* newPlayer = pool.allocate();
	playerKey = newPlayer->generatePlayerId();

	newPlayer->Init(playerKey, key);
	_keys.insert({ key, playerKey });
	_Players.insert({ playerKey, newPlayer });

	//2. 다른친구들에게 내 캐릭터 생성 메시지 보내기(나 포함)
	header_t						header;
	MESSAGE_RES_CREATE_MY_CHARACTER sendMsg;

	buildMsg_Header(
		SIGNITURE, 
		sizeof(MESSAGE_RES_CREATE_MY_CHARACTER), 
		static_cast<int>(MESSAGE_DEFINE::RES_CREATE_MY_CHARACTER), 
		header
	);
	buildMsg_createMyCharacter(
		playerKey, 
		newPlayer->GetDirection(), 
		newPlayer->GetX(), 
		newPlayer->GetY(),
		newPlayer->GetHp(), 
		sendMsg
	);
	//메시지 조립 후 전달. 지금은 임시버퍼 쓰는 중
	char buffer[32] = { 0, };
	memcpy(buffer, &header, sizeof(header_t));
	memcpy(buffer, &sendMsg, sizeof(MESSAGE_RES_CREATE_MY_CHARACTER));
	SendBroadCast(buffer, sizeof(MESSAGE_RES_CREATE_MY_CHARACTER) + sizeof(header_t));

	MESSAGE_RES_CREATE_OTHER_CHARACTER otherChracterMsg;
	int						curId;
	int						curAction;
	char					curDir;
	unsigned short			curX;
	unsigned short			curY;
	char					hp;

	//3. 나에게 기존 캐릭터 생성 메시지 보내기(내 캐릭터 제외)
	for (auto& player : _Players)
	{
		if (player.first == playerKey)
		{
			continue;
		}
		Player* cur = player.second;
		curId = cur->GetPlayerId();
		curDir = cur->GetDirection();
		curX = cur->GetX();
		curY = cur->GetY();
		curAction = cur->GetAction();
		hp = cur->GetHp();

		/*TODO : 직렬화 버퍼로 다 바꿀예정.*/
		buildMsg_Header(
			SIGNITURE,
			sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER),
			static_cast<int>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
			header
		);
		buildMsg_createOtherCharacter(curId, curDir, curX, curY, hp, otherChracterMsg);

		memcpy(buffer, &header, sizeof(header_t));
		memcpy(buffer, &otherChracterMsg, sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));
		SendUniCast(key, buffer, sizeof(header_t) + sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));

		//MOVE START 메시지도 보내야함.
		if (curAction > 0)
		{
			MESSAGE_RES_MOVE_START moveMsg;
			buildMsg_Header(
				SIGNITURE,
				sizeof(MESSAGE_RES_MOVE_START),
				static_cast<int>(MESSAGE_DEFINE::RES_MOVE_START),
				header
			);
			buildMsg_move_start(curId, curDir, curX, curY, moveMsg);
			memcpy(buffer, &header, sizeof(header_t));
			memcpy(buffer, &moveMsg, sizeof(MESSAGE_RES_MOVE_START));

			SendUniCast(key, buffer, sizeof(header_t) + sizeof(MESSAGE_RES_MOVE_START));
		}
	}
}

void GameServer::OnRecvProc(char* message, char* header, size_t hLen, SESSION_KEY key)
{	
	char msgType = reinterpret_cast<header_t*>(header)->_MessageType;
	switch (msgType)
	{
	case static_cast<int>(MESSAGE_DEFINE::REQ_MOVE_START):
		
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_MOVE_STOP):

		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_LEFT_HAND):

		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_RIGHT_HAND):

		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_KICK):

		break;
	default:
		//TODO : 연결 끊어야함.
		Disconnect(key);
		break;
	}
	return;
}

/* 컨텐츠 구현 하기! 예외 케이스들 생각해보자.*/
void GameServer::ReqMoveStartProc(char* message, const SESSION_KEY key)
{
	
	
}

void GameServer::ReqMoveStopProc(char* message, const SESSION_KEY key)
{

}

void GameServer::ReqAttackLeftHandProc(char* message,  const SESSION_KEY key)
{

}

void GameServer::ReqAttackRightHandProc(char* message, const SESSION_KEY key)
{

}

void GameServer::ReqAttackKickProc(char* message, const SESSION_KEY key)
{

}

//프레임 로직 
void GameServer::update()
{
	//플레이어가 죽었다면,
	// 1. 플레이어 객체 파괴, 메모리 풀 반납.
	// 2. 세션에게 객체 파괴 요청. 

	//프레임마다 움직이기.
}
