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
	//Player ���
	_keys.insert({ key, playerKey });
	_Players.insert({ playerKey, newPlayer });

	//1. �� ĳ���� ���� �޽��� ����
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
	//�޽��� ���� �� ����. ������ �ӽù��� ���� ��
	char buffer[32] = { 0, };
	memcpy(buffer, &header, sizeof(header_t));
	memcpy(buffer + sizeof(header_t), &sendMsg, sizeof(MESSAGE_RES_CREATE_MY_CHARACTER));
	SendUniCast(key, buffer, sizeof(MESSAGE_RES_CREATE_MY_CHARACTER) + sizeof(header_t));

	//2.�� ĳ���� ���� �޽��� ��ο��� �����ֱ�
	MESSAGE_RES_CREATE_OTHER_CHARACTER otherChracterMsg;
	buildMsg_Header(
		SIGNITURE,
		sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER),
		static_cast<int>(MESSAGE_DEFINE::RES_CREATE_OTHER_CHARACTER),
		header
	);
	buildMsg_createOtherCharacter(playerKey, newPlayer->GetDirection(), newPlayer->GetX(), newPlayer->GetY(), newPlayer->GetHp(), otherChracterMsg);
	memcpy(buffer, &header, sizeof(header_t));
	memcpy(buffer + sizeof(header_t), &otherChracterMsg, sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));
	SendBroadCast(key, buffer, sizeof(header_t) + sizeof(MESSAGE_RES_CREATE_OTHER_CHARACTER));

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

		/*TODO : ����ȭ ���۷� �� �ٲܿ���.*/
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

		//MOVE START �޽����� ��������.
		if (curAction > 0)
		{
			MESSAGE_RES_MOVE_START moveMsg;
			buildMsg_Header(
				SIGNITURE,
				sizeof(MESSAGE_RES_MOVE_START),
				static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START),
				header
			);
			buildMsg_move_start(curId, curAction, curX, curY, moveMsg);
			memcpy(buffer, &header, sizeof(header_t));
			memcpy(buffer + sizeof(header_t), &moveMsg, sizeof(MESSAGE_RES_MOVE_START));

			SendUniCast(key, buffer, sizeof(header_t) + sizeof(MESSAGE_RES_MOVE_START));
		}
	}
}

void GameServer::OnRecvProc(char* message, char* header, size_t hLen, SESSION_KEY key)
{
	char msgType = reinterpret_cast<header_t*>(header)->_MessageType;
	//Header �����ϰ� payload �Ѱ�����
	char* payload = message + hLen;

	switch (msgType)
	{
	case static_cast<int>(MESSAGE_DEFINE::REQ_MOVE_START):
		ReqMoveStartProc(payload, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_MOVE_STOP):
		ReqMoveStopProc(payload, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_LEFT_HAND):
		ReqAttackLeftHandProc(payload, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_RIGHT_HAND):
		ReqAttackRightHandProc(payload, key);
		break;
	case static_cast<int>(MESSAGE_DEFINE::REQ_ATTACK_KICK):
		ReqAttackKickProc(payload, key);
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
void GameServer::ReqMoveStartProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_MOVE_START* recvMsg = reinterpret_cast<MESSAGE_REQ_MOVE_START*>(message);
	char action = recvMsg->_Direction;
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

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

	//�� ���� �� ������
	MESSAGE_RES_MOVE_START sendMsg;
	MESSAGE_HEADER header;
	char buffer[32] = { 0 , };
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_START), static_cast<char>(MESSAGE_DEFINE::RES_MOVE_START), header);
	buildMsg_move_start(playerKey, action, recvX, recvY, sendMsg);

	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_MOVE_START));
#ifdef GAME_DEBUG
	printf("============================================================\n");
	printf("MOVE START MESSAGE\n");
	printf("PLAYER ID : %d | SESSION ID : %d | PARAMETER KEY : %d |CUR_X : %hd  | CUR_Y : %hd |\n", player->GetPlayerId(), player->GetSessionId(), key, player->GetX(), player->GetY());
	printf("============================================================\n");
#endif
	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_MOVE_START) + sizeof(MESSAGE_HEADER));
}

void GameServer::ReqMoveStopProc(char* message, const SESSION_KEY key)
{
	// stop�� ��ǥ���� 
	// Ʋ������ ������ ������ ����
	// ���� Ʋ������ ���� ����.
	MESSAGE_REQ_MOVE_STOP* recvMsg = reinterpret_cast<MESSAGE_REQ_MOVE_STOP*>(message);
	char direction = recvMsg->_Direction;
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

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
	//���꽺ž �޽��� ���� �� ������
	MESSAGE_RES_MOVE_STOP sendMsg;
	MESSAGE_HEADER header;
	char buffer[32] = { 0 , };
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_STOP), static_cast<char>(MESSAGE_DEFINE::RES_MOVE_STOP), header);
	buildMsg_move_stop(playerKey, direction, player->GetX(), player->GetY(), sendMsg);

	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_MOVE_STOP));
	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_MOVE_STOP) + sizeof(MESSAGE_HEADER));

	return;
}

void GameServer::ReqAttackLeftHandProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_ATTACK_LEFT_HAND* recvMsg = reinterpret_cast<MESSAGE_REQ_ATTACK_LEFT_HAND*>(message);
	char attackDir = recvMsg->_Direction;

	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	short myX = attacker->GetX();
	short myY = attacker->GetY();

	MESSAGE_HEADER header;
	MESSAGE_RES_ATTACK_LEFT_HAND sendMsg;
	char buffer[32] = { 0, };

	//���� Message Send
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND), header);
	buildMsg_attack_lefthand(playerKey, attackDir, myX, myY, sendMsg);
	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND));

	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND) + sizeof(MESSAGE_HEADER));

	//�����̰� �־��ٸ�, ���ߴ� Message ���� 
	MESSAGE_RES_MOVE_STOP moveStopMsg;
	if (attacker->GetAction() != static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION))
	{
		attacker->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));

		buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_STOP), sizeof(MESSAGE_RES_MOVE_STOP), header);
		buildMsg_move_stop(playerKey, attacker->GetDirection(), myX, myY, moveStopMsg);
		memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
		memcpy(buffer + sizeof(MESSAGE_HEADER), &moveStopMsg, sizeof(MESSAGE_RES_MOVE_STOP));

		//Ŭ��� �ڱⰡ �˾Ƽ� �������. 
		SendBroadCast(key, buffer, sizeof(MESSAGE_HEADER) + sizeof(MESSAGE_RES_MOVE_STOP));
	}

	//���ݹ��� ����
	MESSAGE_RES_DAMAGE damageSendMsg;
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
			target->Attacked(static_cast<int>(PLAYER_DAMAGE::LEFT_HAND));
			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), header);
			buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), damageSendMsg);
			memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
			memcpy(buffer + sizeof(MESSAGE_HEADER), &damageSendMsg, sizeof(MESSAGE_RES_DAMAGE));

			SendBroadCast(buffer, sizeof(MESSAGE_RES_DAMAGE) + sizeof(MESSAGE_HEADER));
		}
	}
}

void GameServer::ReqAttackRightHandProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_ATTACK_RIGHT_HAND* recvMsg = reinterpret_cast<MESSAGE_REQ_ATTACK_RIGHT_HAND*>(message);
	char attackDir = recvMsg->_Direction;

	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	int myX = attacker->GetX();
	int myY = attacker->GetY();

	MESSAGE_HEADER header;
	MESSAGE_RES_ATTACK_RIGHT_HAND sendMsg;
	char buffer[32] = { 0, };

	//���� Message Send
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_RIGHT_HAND), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_RIGHT_HAND), header);
	buildMsg_attack_righthand(playerKey, attackDir, myX, myY, sendMsg);
	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_ATTACK_RIGHT_HAND));

	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_ATTACK_RIGHT_HAND) + sizeof(MESSAGE_HEADER));

	//�����̰� �־��ٸ�, ���ߴ� Message ���� 
	MESSAGE_RES_MOVE_STOP moveStopMsg;
	if (attacker->GetAction() != static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION))
	{
		attacker->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));

		buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_STOP), sizeof(MESSAGE_RES_MOVE_STOP), header);
		buildMsg_move_stop(playerKey, attacker->GetDirection(), myX, myY, moveStopMsg);
		memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
		memcpy(buffer + sizeof(MESSAGE_HEADER), &moveStopMsg, sizeof(MESSAGE_RES_MOVE_STOP));

		//Ŭ��� �ڱⰡ �˾Ƽ� �������. 
		SendBroadCast(key, buffer, sizeof(MESSAGE_HEADER) + sizeof(MESSAGE_RES_MOVE_STOP));
	}

	//���ݹ��� ����
	MESSAGE_RES_DAMAGE damageSendMsg;
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
			target->Attacked(static_cast<int>(PLAYER_DAMAGE::RIGHT_HAND));
			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), header);
			buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), damageSendMsg);
			memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
			memcpy(buffer + sizeof(MESSAGE_HEADER), &damageSendMsg, sizeof(MESSAGE_RES_DAMAGE));

			SendBroadCast(buffer, sizeof(MESSAGE_RES_DAMAGE) + sizeof(MESSAGE_HEADER));
		}
	}

}

void GameServer::ReqAttackKickProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_ATTACK_KICK* recvMsg = reinterpret_cast<MESSAGE_REQ_ATTACK_KICK*>(message);
	char attackDir = recvMsg->_Direction;

	//�̻��� ������ ���Դٸ� ����. (�׷����� ��������)
	if (CheckDirection(attackDir) == false)
	{
		return;
	}
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	short myX = attacker->GetX();
	short myY = attacker->GetY();

	MESSAGE_HEADER header;
	MESSAGE_RES_ATTACK_KICK sendMsg;
	char buffer[32] = { 0, };

	//���� Message Send
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_KICK), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_KICK), header);
	buildMsg_attack_kick(playerKey, attackDir, myX, myY, sendMsg);
	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_ATTACK_KICK));
	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_ATTACK_KICK) + sizeof(MESSAGE_HEADER));

	//�����̰� �־��ٸ�, ���ߴ� Message ���� 
	MESSAGE_RES_MOVE_STOP moveStopMsg;
	if (attacker->GetAction() != static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION))
	{
		attacker->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));

		buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_MOVE_STOP), sizeof(MESSAGE_RES_MOVE_STOP), header);
		buildMsg_move_stop(playerKey, attacker->GetDirection(), myX, myY, moveStopMsg);
		memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
		memcpy(buffer + sizeof(MESSAGE_HEADER), &moveStopMsg, sizeof(MESSAGE_RES_MOVE_STOP));

		//Ŭ��� �ڱⰡ �˾Ƽ� �������. 
		SendBroadCast(key, buffer, sizeof(MESSAGE_HEADER) + sizeof(MESSAGE_RES_MOVE_STOP));
	}

	//���ݹ��� ����
	MESSAGE_RES_DAMAGE damageSendMsg;
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
			static_cast<int>(PLAYER_ATTACK_RANGE::KICK_X),
			static_cast<int>(PLAYER_ATTACK_RANGE::KICK_Y),
			targetX, targetY, attackDir))
		{
			target->Attacked(static_cast<int>(PLAYER_DAMAGE::KICK));
			buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DAMAGE), static_cast<char>(MESSAGE_DEFINE::RES_DAMAGE), header);
			buildMsg_damage(playerKey, target->GetPlayerId(), target->GetHp(), damageSendMsg);
			memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
			memcpy(buffer + sizeof(MESSAGE_HEADER), &damageSendMsg, sizeof(MESSAGE_RES_DAMAGE));

			SendBroadCast(buffer, sizeof(MESSAGE_RES_DAMAGE) + sizeof(MESSAGE_HEADER));
		}
	}
}

bool GameServer::CheckAttackInRange(const short attackerX, const short attackerY, const int AttackRangeX, const int AttackRangeY, const short targetX, const short targetY, const char direction)
{
	if (direction == static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		if (attackerX <= targetX && targetX <= (attackerX + AttackRangeX) && abs(attackerY - targetY) <= AttackRangeY)
		{
			return true;
		}
	}
	//LEFT
	else
	{
		if (attackerX - AttackRangeX <= targetX && targetX <= attackerX && abs(attackerY - targetY) <= 10)
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
		return;
	}
	PLAYER_KEY playerKey = iter->second;
	//��ȿ���� ���� �÷��̾�Ű?
	const auto& iter2 = _Players.find(playerKey);
	if (iter2 == _Players.end())
	{
		return;
	}
	Player* DeathPlayer = iter2->second;

	MESSAGE_HEADER header;
	MESSAGE_RES_DELETE_CHARACTER sendMsg;
#ifdef GAME_DEBUG
	printf("============================================================\n");
	printf("DELETE CHARACTER MESSAGE\n");
	printf("PLAYER ID : %d \n", playerKey);
	printf("============================================================\n");
#endif
	char buffer[32] = { 0, };
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_DELETE_CHARACTER), static_cast<char>(MESSAGE_DEFINE::RES_DELETE_CHARACTER), header);
	buildMsg_deleteCharacter(playerKey, sendMsg);

	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_DELETE_CHARACTER));
	SendBroadCast(key, buffer, sizeof(MESSAGE_HEADER) + sizeof(MESSAGE_RES_DELETE_CHARACTER));

	Disconnect(key);
	MemoryPool<Player, PLAYER_POOL_SIZE>& pool = MemoryPool<Player, PLAYER_POOL_SIZE>::getInstance();
	pool.deAllocate(DeathPlayer);
	_Players.erase(playerKey);
}
//������ ���� 
void GameServer::update()
{
	DWORD nextTick;
	int sleepTime;

	nextTick = timeGetTime();

	nextTick += TIME_PER_FRAME;
	sleepTime = nextTick - timeGetTime();
	if (sleepTime > 0)
	{
		Sleep(sleepTime);
	}

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
		printf("PLAYER X : %d  |  PLAYER Y : %d \n", nextX, nextY);
#endif
	}


}
