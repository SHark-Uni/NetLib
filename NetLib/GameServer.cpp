#include "GameServer.h"
#include "MessageFormat.h"
#include "MessageBuilder.h"
#include "MemoryPool.h"
#include "Player.h"
#include "PlayerDefine.h"

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
	//Player 등록
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
	memcpy(buffer + sizeof(header_t), &sendMsg, sizeof(MESSAGE_RES_CREATE_MY_CHARACTER));
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
			static_cast<char>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
			header
		);
		buildMsg_createOtherCharacter(curId, curDir, curX, curY, hp, otherChracterMsg);

		memcpy(buffer, &header, sizeof(header_t));
		memcpy(buffer + sizeof(header_t), &otherChracterMsg, sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));
		SendUniCast(key, buffer, sizeof(header_t) + sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));

		//MOVE START 메시지도 보내야함.
		if (curAction > 0)
		{
			MESSAGE_RES_MOVE_START moveMsg;
			buildMsg_Header(
				SIGNITURE,
				sizeof(MESSAGE_RES_MOVE_START),
				static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START),
				header
			);
			buildMsg_move_start(curId, curDir, curX, curY, moveMsg);
			memcpy(buffer, &header, sizeof(header_t));
			memcpy(buffer + sizeof(header_t), &moveMsg, sizeof(MESSAGE_RES_MOVE_START));

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
		ReqMoveStartProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_MOVE_STOP):
		ReqMoveStopProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_LEFT_HAND):
		ReqAttackLeftHandProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_RIGHT_HAND):
		ReqAttackRightHandProc(message, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_KICK):
		ReqAttackKickProc(message, key);
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
	/*TODO :
	1. Player 상태 변경
	  - 움직이는 방향에 따라, 캐릭터 방향도 돌려줘야함.

	*/
	MESSAGE_REQ_MOVE_START* recvMsg = reinterpret_cast<MESSAGE_REQ_MOVE_START*>(message);
	char direction = recvMsg->_Direction;
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//범위 넘는 메시지 무시
	if (recvX > static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) || recvY > static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return;
	}

	//내 캐릭터 정보 찾기
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;

	//방향설정
	switch (direction)
	{
	case static_cast<int>(MOVE_DIRECTION::LEFT):
		_FALLTHROUGH
	case static_cast<int>(MOVE_DIRECTION::LEFT_TOP):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::LEFT));
		break;
	case static_cast<int>(MOVE_DIRECTION::RIGHT_TOP):
	case static_cast<int>(MOVE_DIRECTION::RIGHT):
		_FALLTHROUGH
	case static_cast<int>(MOVE_DIRECTION::RIGHT_BOTTOM):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::RIGHT));
		break;
	case static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::LEFT));
		break;
	default:
		//이상한 방향 무시
		return;
	}
	player->SetAction(direction);

	//나 빼고 다 보내기
	MESSAGE_RES_MOVE_START sendMsg;
	MESSAGE_HEADER header;
	char buffer[32] = { 0 , };
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_START), static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START));
	buildMsg_move_start(playerKey, direction, recvX, recvY, sendMsg);

	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_MOVE_START));

	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_MOVE_START) + sizeof(MESSAGE_HEADER));
}

void GameServer::ReqMoveStopProc(char* message, const SESSION_KEY key)
{
	// stop시 좌표설정 
	// 틀어짐이 심하지 않으면 인정
	// 많이 틀어지면 연결 끊기.
	MESSAGE_REQ_MOVE_STOP* recvMsg;
	char direction = recvMsg->_Direction;
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//범위 넘는 메시지 무시
	if (recvX > static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) || recvY > static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return;
	}

	//좌/우가 아닌경우 무시
	if (direction != static_cast<int>(CHARCTER_DIRECTION_2D::LEFT) ||
		direction != static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		return;
	}

	//내 캐릭터 정보 찾기
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;

	short playerX = player->GetX();
	short playerY = player->GetY();

	//오차범위 이내
	if (abs(recvX - playerX) <= static_cast<int>(MAX_MAP_BOUNDARY::MAX_ERROR_BOUNDARY) &&
		abs(recvY - playerY) <= static_cast<int>(MAX_MAP_BOUNDARY::MAX_ERROR_BOUNDARY))
	{
		player->SetX(recvX);
		player->SetY(recvY);
		player->SetDirection(direction);
		player->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));
		//무브스탑 메시지 생성 후 보내기

		MESSAGE_RES_MOVE_STOP sendMsg;
		MESSAGE_HEADER header;
		char buffer[32] = { 0 , };
		buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_STOP), static_cast<char>(MESSAGE_DEFINE::RES_MOVE_STOP), header);
		buildMsg_move_stop(playerKey, direction, player->GetX(), player->GetY());

		memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
		memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_MOVE_STOP));
		SendBroadCast(key, buffer, sizeof(MESSAGE_RES_MOVE_STOP));
		return;
	}
	//오차범위가 너무 난다면 끊어준다.
	Disconnect(key);
	return;
}

void GameServer::ReqAttackLeftHandProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_ATTACK_LEFT_HAND* recvMsg = reinterpret_cast<MESSAGE_REQ_ATTACK_LEFT_HAND*>(message);
	//공격 방향
	char attackDir = recvMsg->_Direction;
	if (attackDir != static_cast<char>(CHARCTER_DIRECTION_2D::RIGHT) || static_cast<char>(CHARCTER_DIRECTION_2D::LEFT))
	{
		return;
	}

	int RangeX = static_cast<int>(PLAYER_ATTACK_RANGE::LEFT_HAND_X);

	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//내 캐릭터 정보 찾기
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	int myX = attacker->GetX();
	int myY = attacker->GetY();

	MESSAGE_HEADER header;
	MESSAGE_RES_ATTACK_LEFT_HAND sendMsg;
	char buffer[32] = { 0, };

	//어택 Message Send
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND), header);
	buildMsg_attack_lefthand(playerKey, attackDir, myX, myY, sendMsg);
	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND));

	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND));
	//공격범위 판정
	MESSAGE_RES_DAMAGE damageSendMsg;
	for (auto& player : _Players)
	{
		Player* target = player.second;
		//내가 내 자신을 때리면 안됨.
		if (target->GetPlayerId() == playerKey)
		{
			continue;
		}

		int targetX = target->GetX();
		int targetY = target->GetY();
		//오른쪽 
		if (attackDir == static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
		{
			if (myX <= targetX && targetX <= (myX + RangeX) && abs(myY - targetY) <= 10)
			{
				target->Attacked(static_cast<int>(PLAYER_DAMAGE::LEFT_HAND));
				buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), header);
				buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), damageSendMsg);
				memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
				memcpy(buffer + sizeof(MESSAGE_HEADER), &damageSendMsg, sizeof(MESSAGE_RES_DAMAGE));

				SendUniCast(key, buffer, sizeof(MESSAGE_RES_DAMAGE));
				continue;
			}
		}
		if (myX - RangeX <= targetX && targetX <= myX + RangeX && abs(myY - targetY) <= 10)
		{
			target->Attacked(static_cast<int>(PLAYER_DAMAGE::LEFT_HAND));
			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), header);
			buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), damageSendMsg);
			memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
			memcpy(buffer + sizeof(MESSAGE_HEADER), &damageSendMsg, sizeof(MESSAGE_RES_DAMAGE));

			SendUniCast(key, buffer, sizeof(MESSAGE_RES_DAMAGE));
		}
	}
}

void GameServer::ReqAttackRightHandProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_ATTACK_RIGHT_HAND* recvMsg = reinterpret_cast<MESSAGE_REQ_ATTACK_RIGHT_HAND*>(message);
}

void GameServer::ReqAttackKickProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_ATTACK_KICK* recvMsg = reinterpret_cast<MESSAGE_REQ_ATTACK_KICK*>(message);

}

//프레임 로직 
void GameServer::update()
{
	//플레이어가 죽었다면,
	// 1. 플레이어 객체 파괴, 메모리 풀 반납.
	// 2. 세션에게 객체 파괴 요청. 

	//프레임마다 움직이기.
}
