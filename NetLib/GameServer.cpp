#include "GameServer.h"
#include "MessageFormat.h"
#include "MessageBuilder.h"
#include "MemoryPool.h"
#include "Player.h"
#include "PlayerDefine.h"
#include "Logger.h"
#include "NetDefine.h"
#include "ObjectPool.h"

#include <cassert>

using namespace NetLib;
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
onAcceptProc���� ����
1. �÷��̾� ���� (Contents�ڵ忡��
2. �ٸ� �÷��̾�� �� �÷��̾� ���� �޽��� ������
3. ���� �÷��̾�� ������ ���� �޽��� ������
===================================*/
void GameServer::OnAcceptProc(const SESSION_KEY key)
{
	//1. �÷��̾� ����
	int playerKey;
	MemoryPool<Player, PLAYER_POOL_SIZE>& pool = MemoryPool<Player, PLAYER_POOL_SIZE>::getInstance();
	Player* newPlayer = pool.allocate();
	playerKey = newPlayer->generatePlayerId();

	newPlayer->Init(playerKey, key);
	_keys.insert({ key, playerKey });
	_Players.insert({ playerKey, newPlayer });

	auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();
	SerializeBuffer* sBuffer = SbufferPool.allocate_reuse(static_cast<int>(NETLIB_POOL_SIZE::SBUFFER_DEFAULT_SIZE));
	sBuffer->clear();
	//1. �� ĳ���� ���� �޽��� ����
	buildMsg_Header(
		SIGNITURE,
		sizeof(MESSAGE_RES_CREATE_MY_CHARACTER),
		static_cast<int>(MESSAGE_DEFINE::RES_CREATE_MY_CHARACTER),
		sBuffer
	);

	buildMsg_createMyCharacter(
		playerKey,
		newPlayer->GetDirection(),
		newPlayer->GetX(),
		newPlayer->GetY(),
		newPlayer->GetHp(),
		sBuffer
	);
	//�޽��� ���� �� ����. ������ �ӽù��� ���� ��
	SendUniCast(key, sBuffer, sizeof(MESSAGE_RES_CREATE_MY_CHARACTER) + sizeof(header_t));

	sBuffer->clear();
	//2.�� ĳ���� ���� �޽��� ��ο��� �����ֱ�
	buildMsg_Header(
		SIGNITURE,
		sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER),
		static_cast<int>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
		sBuffer
	);
	buildMsg_createOtherCharacter(playerKey, newPlayer->GetDirection(), newPlayer->GetX(), newPlayer->GetY(), newPlayer->GetHp(), sBuffer);
	SendBroadCast(key, sBuffer, sizeof(header_t) + sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));

	//3. ������ �ִ� ĳ���͵� �������ִ� �޽��� ������.
	int						curId;
	int						curAction;
	char					curDir;
	unsigned short			curX;
	unsigned short			curY;
	char					hp;
	//3. ������ ���� ĳ���� ���� �޽��� ������(�� ĳ���� ����)
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

		sBuffer->clear();
		buildMsg_Header(
			SIGNITURE,
			sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER),
			static_cast<char>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
			sBuffer
		);
		buildMsg_createOtherCharacter(curId, curDir, curX, curY, hp, sBuffer);
		SendUniCast(key, sBuffer, sizeof(header_t) + sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));

		//MOVE START �޽����� ��������.
		if (curAction > 0)
		{
			sBuffer->clear();
			buildMsg_Header(
				SIGNITURE,
				sizeof(MESSAGE_RES_MOVE_START),
				static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START),
				sBuffer
			);
			buildMsg_move_start(curId, curAction, curX, curY, sBuffer);
			SendUniCast(key, sBuffer, sizeof(header_t) + sizeof(MESSAGE_RES_MOVE_START));
		}
	}
	SbufferPool.deAllocate(sBuffer);
}

void GameServer::OnRecvProc(SerializeBuffer* message, const char msgType, SESSION_KEY key)
{
	//Header �����ϰ� payload �Ѱ�����
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
#ifdef GAME_DEBUG
		//TODO : ���� �������.
		printf("BAD REQUEST!\n");
#endif
		OnDestroyProc(key);
		break;
	}
	return;
}
/* ������ ���� �ϱ�! ���� ���̽��� �����غ���.*/
void GameServer::ReqMoveStartProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char action;
	unsigned short recvX;
	unsigned short recvY;

	*message >> action >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		//�б� ����. ����ȭ ���� ���� �߸��Ѱ���. Ȥ��, �޽��� ũ�Ⱑ ���ǵ��� �������·� ����.
		DebugBreak();
	}

	//���� �Ѵ� �޽��� ����
	if (recvX > static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) || recvY > static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return;
	}
	//�̻��� ���� ����
	if (action < static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION) || action > static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM))
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;
	player->SetAction(action);
	//���⼳��
	switch (action)
	{
	case static_cast<int>(MOVE_DIRECTION::LEFT):
	case static_cast<int>(MOVE_DIRECTION::LEFT_TOP):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::LEFT));
		break;
	case static_cast<int>(MOVE_DIRECTION::RIGHT_TOP):
	case static_cast<int>(MOVE_DIRECTION::RIGHT):
	case static_cast<int>(MOVE_DIRECTION::RIGHT_BOTTOM):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::RIGHT));
		break;
	case static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM):
		player->SetDirection(static_cast<char>(CHARCTER_DIRECTION_2D::LEFT));
		break;
	default:
		break;
	}

	auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();
	SerializeBuffer* sBuffer = SbufferPool.allocate_reuse(static_cast<int>(NETLIB_POOL_SIZE::SBUFFER_DEFAULT_SIZE));
	sBuffer->clear();
	//�� ���� �� ������

	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_START), static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START), sBuffer);
	buildMsg_move_start(playerKey, action, recvX, recvY, sBuffer);

#ifdef GAME_DEBUG
	printf("============================================================\n");
	printf("MOVE START MESSAGE\n");
	printf("PLAYER ID : %d | SESSION ID : %d | PARAMETER KEY : %d |CUR_X : %hd  | CUR_Y : %hd |\n", player->GetPlayerId(), player->GetSessionId(), key, player->GetX(), player->GetY());
	printf("============================================================\n");
#endif
	SendBroadCast(key, sBuffer, sizeof(MESSAGE_RES_MOVE_START) + sizeof(MESSAGE_HEADER));
	SbufferPool.deAllocate(sBuffer);
}

void GameServer::ReqMoveStopProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char direction;
	unsigned short recvX;
	unsigned short recvY;

	*message >> direction >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		//�б� ����. ����ȭ ���� ���� �߸��Ѱ���. Ȥ��, �޽��� ũ�Ⱑ ���ǵ��� �������·� ����.
		DebugBreak();
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;

	short playerX = player->GetX();
	short playerY = player->GetY();


	if (abs(recvX - playerX) > static_cast<int>(MAX_MAP_BOUNDARY::MAX_ERROR_BOUNDARY) ||
		abs(recvY - playerY) > static_cast<int>(MAX_MAP_BOUNDARY::MAX_ERROR_BOUNDARY))
	{
#ifdef GAME_DEBUG
		printf("CURRENT X : %hd | CURRENT Y : %hd \n", playerX, playerY);
		printf("OUT OF BOUNDARY DISCONNECT!\n");
		WCHAR buffer[80] = { 0, };
		swprintf_s(buffer, L"PLAYER X : %hd | PLAYER Y : %hd | RECV X : %hd | RECV Y : %hd\n", playerX, playerY, recvX, recvY);
		Logger::Logging(-2, __LINE__, buffer);
#endif
		OnDestroyProc(key);
		return;
	}

	if (recvX > static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) || recvY > static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return;
	}
	if (CheckDirection(direction) == false)
	{
		return;
	}

	player->SetX(recvX);
	player->SetY(recvY);
	player->SetDirection(direction);
	player->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));

#ifdef GAME_DEBUG
	printf("MOVE STOP MESSAGE\n");
	printf("PLAYER ID : %d | SESSION ID : %d | PARAM KEY : %d |CUR_X : %hd  | CUR_Y : %hd |\n", player->GetPlayerId(), player->GetSessionId(), key, player->GetX(), player->GetY());
#endif
	auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();
	SerializeBuffer* sBuffer = SbufferPool.allocate_reuse(static_cast<int>(NETLIB_POOL_SIZE::SBUFFER_DEFAULT_SIZE));
	sBuffer->clear();

	//���꽺ž �޽��� ���� �� ������
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_STOP), static_cast<char>(MESSAGE_DEFINE::RES_MOVE_STOP), sBuffer);
	buildMsg_move_stop(playerKey, direction, player->GetX(), player->GetY(), sBuffer);
	SendBroadCast(key, sBuffer, sizeof(MESSAGE_RES_MOVE_STOP) + sizeof(MESSAGE_HEADER));

	SbufferPool.deAllocate(sBuffer);
	return;
}

void GameServer::ReqAttackLeftHandProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char attackDir;
	unsigned short recvX;
	unsigned short recvY;

	*message >> attackDir >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		//�б� ����. ����ȭ ���� ���� �߸��Ѱ���. Ȥ��, �޽��� ũ�Ⱑ ���ǵ��� �������·� ����.
		DebugBreak();
	}
	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;
	short myX = attacker->GetX();
	short myY = attacker->GetY();

	auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();
	SerializeBuffer* sBuffer = SbufferPool.allocate_reuse(static_cast<int>(NETLIB_POOL_SIZE::SBUFFER_DEFAULT_SIZE));
	
	//���� Message Send
	sBuffer->clear();
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND), sBuffer);
	buildMsg_attack_lefthand(playerKey, attackDir, myX, myY, sBuffer);
	SendBroadCast(key, sBuffer, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND) + sizeof(MESSAGE_HEADER));

	//���ݹ��� ����
	for (auto& player : _Players)
	{
		Player* target = player.second;
		//���� �� �ڽ��� ������ �ȵ�.
		if (target->GetPlayerId() == playerKey)
		{
			continue;
		}
		int targetX = target->GetX();
		int targetY = target->GetY();
		if (CheckAttackInRange(
			myX,
			myY,
			static_cast<int>(PLAYER_ATTACK_RANGE::LEFT_HAND_X),
			static_cast<int>(PLAYER_ATTACK_RANGE::LEFT_HAND_Y),
			targetX, targetY, attackDir))
		{
			sBuffer->clear();
			target->Attacked(static_cast<int>(PLAYER_DAMAGE::LEFT_HAND));
			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), sBuffer);
			buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), sBuffer);
			SendBroadCast(sBuffer, sizeof(MESSAGE_RES_DAMAGE) + sizeof(MESSAGE_HEADER));
		}
	}
	SbufferPool.deAllocate(sBuffer);
}

void GameServer::ReqAttackRightHandProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char attackDir;
	unsigned short recvX;
	unsigned short recvY;

	*message >> attackDir >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		DebugBreak();
	}
	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	short myX = attacker->GetX();
	short myY = attacker->GetY();

	auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();
	SerializeBuffer* sBuffer = SbufferPool.allocate_reuse(static_cast<int>(NETLIB_POOL_SIZE::SBUFFER_DEFAULT_SIZE));
	sBuffer->clear();

	//���� Message Send
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_RIGHT_HAND), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_RIGHT_HAND), sBuffer);
	buildMsg_attack_righthand(playerKey, attackDir, myX, myY, sBuffer);
	SendBroadCast(key, sBuffer, sizeof(MESSAGE_RES_ATTACK_RIGHT_HAND) + sizeof(MESSAGE_HEADER));

	//���ݹ��� ����
	for (auto& player : _Players)
	{
		Player* target = player.second;
		//���� �� �ڽ��� ������ �ȵ�.
		if (target->GetPlayerId() == playerKey)
		{
			continue;
		}
		int targetX = target->GetX();
		int targetY = target->GetY();
		if (CheckAttackInRange(
			myX,
			myY,
			static_cast<int>(PLAYER_ATTACK_RANGE::RIGHT_HAND_X),
			static_cast<int>(PLAYER_ATTACK_RANGE::RIGHT_HAND_Y),
			targetX, targetY, attackDir))
		{
			sBuffer->clear();

			target->Attacked(static_cast<int>(PLAYER_DAMAGE::RIGHT_HAND));
			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), sBuffer);
			buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), sBuffer);

			SendBroadCast(sBuffer, sizeof(MESSAGE_RES_DAMAGE) + sizeof(MESSAGE_HEADER));
		}
	}
	SbufferPool.deAllocate(sBuffer);
}

void GameServer::ReqAttackKickProc(SerializeBuffer* message, const SESSION_KEY key)
{
	char attackDir;
	unsigned short recvX;
	unsigned short recvY;

	*message >> attackDir >> recvX >> recvY;
	if (message->checkFailBit() == true)
	{
		DebugBreak();
	}

	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();
	SerializeBuffer* sBuffer = SbufferPool.allocate_reuse(static_cast<int>(NETLIB_POOL_SIZE::SBUFFER_DEFAULT_SIZE));
	sBuffer->clear();

	//���� Message Send
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_KICK), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_KICK), sBuffer);
	buildMsg_attack_kick(playerKey, attackDir, attacker->GetX(), attacker->GetY(), sBuffer);
	SendBroadCast(key, sBuffer, sizeof(MESSAGE_RES_ATTACK_KICK) + sizeof(MESSAGE_HEADER));

	//���ݹ��� ����
	for (auto& player : _Players)
	{
		Player* target = player.second;
		if (target->GetPlayerId() == playerKey)
		{
			continue;
		}
		int targetX = target->GetX();
		int targetY = target->GetY();
		if (CheckAttackInRange(
			attacker->GetX(),
			attacker->GetY(),
			static_cast<int>(PLAYER_ATTACK_RANGE::KICK_X),
			static_cast<int>(PLAYER_ATTACK_RANGE::KICK_Y),
			targetX, targetY, attackDir))
		{
			sBuffer->clear();
			target->Attacked(static_cast<int>(PLAYER_DAMAGE::KICK));

			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), sBuffer);
			buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), sBuffer);
			SendBroadCast(sBuffer, sizeof(MESSAGE_RES_DAMAGE) + sizeof(MESSAGE_HEADER));

		}
	}
	SbufferPool.deAllocate(sBuffer);
}

bool GameServer::CheckAttackInRange(const short attackerX, const short attackerY, const int AttackRangeX, const int AttackRangeY, const short targetX, const short targetY, const char direction)
{
	if (direction == static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		if (attackerX < targetX && targetX <= (attackerX + AttackRangeX) && abs(attackerY - targetY) <= AttackRangeY)
		{
			return true;
		}
	}
	//LEFT
	else
	{
		if (attackerX - AttackRangeX < targetX && targetX <= attackerX && abs(attackerY - targetY) <= AttackRangeY)
		{
			return true;
		}
	}
	return false;
}

bool GameServer::CheckDirection(char direction)
{
	//��/�찡 �ƴѰ�� ����
	if (direction == static_cast<int>(CHARCTER_DIRECTION_2D::LEFT) || direction == static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		return true;
	}
	return false;
}


void GameServer::OnDestroyProc(const SESSION_KEY key)
{
	const auto& iter = _keys.find(key);
	//��ȿ���� ���� ����Ű?
	if (iter == _keys.end())
	{
		DebugBreak();
		return;
	}

	PLAYER_KEY playerKey = iter->second;
	const auto& iter2 = _Players.find(playerKey);
	if (iter2 == _Players.end())
	{	
		//��ȿ���� ���� �÷��̾�Ű?
		DebugBreak();
		return;
	}

	Player* DeathPlayer = iter2->second;

#ifdef GAME_DEBUG
	printf("============================================================\n");
	printf("DELETE CHARACTER MESSAGE\n");
	printf("PLAYER ID : %d \n", playerKey);
	printf("============================================================\n");
#endif
	Disconnect(key);
	DeathPlayer->SetPlayerDeath();
}

void GameServer::cleanUpPlayer()
{
	//��¥ ����� ���
	auto iter = _Players.begin();
	auto iter_e = _Players.end();
	MemoryPool<Player, PLAYER_POOL_SIZE>& pool = MemoryPool<Player, PLAYER_POOL_SIZE>::getInstance();

	auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();
	SerializeBuffer* sBuffer = SbufferPool.allocate_reuse(static_cast<int>(NETLIB_POOL_SIZE::SBUFFER_DEFAULT_SIZE));
	
	for (; iter != iter_e; )
	{
		Player* cur = iter->second;
		int key = iter->first;

		if (cur->IsAlive() == false)
		{
			sBuffer->clear();

			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DELETE_CHARACTER), static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), sBuffer);
			buildMsg_deleteCharacter(key, sBuffer);
			SendBroadCast(cur->GetSessionId(), sBuffer, sizeof(MESSAGE_HEADER) + sizeof(MESSAGE_RES_DELETE_CHARACTER));

			_keys.erase(cur->GetSessionId());
			pool.deAllocate(cur);
			iter = _Players.erase(iter);
			continue;
		}
		++iter;
	}
	SbufferPool.deAllocate(sBuffer);
}
//������ ���� 
void GameServer::update()
{
	//�����Ӹ��� �����̱�.
	for (auto& player : _Players)
	{
		Player* cur = player.second;
		if (cur->GetHp() <= 0)
		{
#ifdef GAME_DEBUG
			printf("PLAYER DIE DISCONNECT!\n");
#endif		
			OnDestroyProc(cur->GetSessionId());
			continue;
		}
#ifdef GAME_DEBUG
		//FOR DEBUG
		int prevX = cur->GetX();
		int prevY = cur->GetY();
#endif
		int action = cur->GetAction();
		switch (action)
		{
		case static_cast<int>(MOVE_DIRECTION::LEFT):
			cur->Move(-(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED)), 0);
			break;
		case static_cast<int>(MOVE_DIRECTION::LEFT_TOP):
			cur->Move(-(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED)), -(static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED)));
			break;
		case static_cast<int>(MOVE_DIRECTION::TOP):
			cur->Move(0, -(static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED)));
			break;
		case static_cast<int>(MOVE_DIRECTION::RIGHT_TOP):
			cur->Move(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED), -(static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED)));
			break;
		case static_cast<int>(MOVE_DIRECTION::RIGHT):
			cur->Move(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED), 0);
			break;
		case static_cast<int>(MOVE_DIRECTION::RIGHT_BOTTOM):
			cur->Move(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED), static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED));
			break;
		case static_cast<int>(MOVE_DIRECTION::BOTTOM):
			cur->Move(0, static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED));
			break;
		case static_cast<int>(MOVE_DIRECTION::LEFT_BOTTOM):
			cur->Move(-(static_cast<short>(PLAYER_MOVE_SPEED::X_SPEED)), static_cast<short>(PLAYER_MOVE_SPEED::Y_SPEED));
			break;
		default:
			break;
		}
#ifdef GAME_DEBUG
		int nextX = cur->GetX();
		int nextY = cur->GetY();
		if (prevX == nextX && prevY == nextY)
		{
			continue;
		}
		printf("PLAYER ID : %d | PLAYER X : %hd  |  PLAYER Y : %hd \n", cur->GetPlayerId(), nextX, nextY);
#endif
	}


}
