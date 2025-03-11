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
void GameServer::OnAcceptProc(const int key)
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

	//2. �ٸ�ģ���鿡�� �� ĳ���� ���� �޽��� ������(�� ����)
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
	SendBroadCast(buffer, sizeof(MESSAGE_RES_CREATE_MY_CHARACTER) + sizeof(header_t));

	MESSAGE_RES_CREATE_OTHER_CHARACTER otherChracterMsg;
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
		//TODO : ���� �������.
		Disconnect(key);
		break;
	}
	return;
}

/* ������ ���� �ϱ�! ���� ���̽��� �����غ���.*/
void GameServer::ReqMoveStartProc(char* message, const SESSION_KEY key)
{
	/*TODO :
	1. Player ���� ����
	  - �����̴� ���⿡ ����, ĳ���� ���⵵ ���������.

	*/
	MESSAGE_REQ_MOVE_START* recvMsg = reinterpret_cast<MESSAGE_REQ_MOVE_START*>(message);
	char direction = recvMsg->_Direction;
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//���� �Ѵ� �޽��� ����
	if (recvX > static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) || recvY > static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;

	//���⼳��
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
		//�̻��� ���� ����
		return;
	}
	player->SetAction(direction);

	//�� ���� �� ������
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
	// stop�� ��ǥ���� 
	// Ʋ������ ������ ������ ����
	// ���� Ʋ������ ���� ����.
	MESSAGE_REQ_MOVE_STOP* recvMsg;
	char direction = recvMsg->_Direction;
	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//���� �Ѵ� �޽��� ����
	if (recvX > static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) || recvY > static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return;
	}

	//��/�찡 �ƴѰ�� ����
	if (direction != static_cast<int>(CHARCTER_DIRECTION_2D::LEFT) ||
		direction != static_cast<int>(CHARCTER_DIRECTION_2D::RIGHT))
	{
		return;
	}

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* player = _Players.find(playerKey)->second;

	short playerX = player->GetX();
	short playerY = player->GetY();

	//�������� �̳�
	if (abs(recvX - playerX) <= static_cast<int>(MAX_MAP_BOUNDARY::MAX_ERROR_BOUNDARY) &&
		abs(recvY - playerY) <= static_cast<int>(MAX_MAP_BOUNDARY::MAX_ERROR_BOUNDARY))
	{
		player->SetX(recvX);
		player->SetY(recvY);
		player->SetDirection(direction);
		player->SetAction(static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION));
		//���꽺ž �޽��� ���� �� ������

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
	//���������� �ʹ� ���ٸ� �����ش�.
	Disconnect(key);
	return;
}

void GameServer::ReqAttackLeftHandProc(char* message, const SESSION_KEY key)
{
	MESSAGE_REQ_ATTACK_LEFT_HAND* recvMsg = reinterpret_cast<MESSAGE_REQ_ATTACK_LEFT_HAND*>(message);
	//���� ����
	char attackDir = recvMsg->_Direction;
	if (attackDir != static_cast<char>(CHARCTER_DIRECTION_2D::RIGHT) || static_cast<char>(CHARCTER_DIRECTION_2D::LEFT))
	{
		return;
	}

	int RangeX = static_cast<int>(PLAYER_ATTACK_RANGE::LEFT_HAND_X);

	unsigned short recvX = recvMsg->_X;
	unsigned short recvY = recvMsg->_Y;

	//�� ĳ���� ���� ã��
	int playerKey = _keys.find(key)->second;
	Player* attacker = _Players.find(playerKey)->second;

	int myX = attacker->GetX();
	int myY = attacker->GetY();

	MESSAGE_HEADER header;
	MESSAGE_RES_ATTACK_LEFT_HAND sendMsg;
	char buffer[32] = { 0, };

	//���� Message Send
	buildMsg_Header(SIGNITURE, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND), static_cast<char>(MESSAGE_DEFINE::RES_ATTACK_LEFT_HAND), header);
	buildMsg_attack_lefthand(playerKey, attackDir, myX, myY, sendMsg);
	memcpy(buffer, &header, sizeof(MESSAGE_HEADER));
	memcpy(buffer + sizeof(MESSAGE_HEADER), &sendMsg, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND));

	SendBroadCast(key, buffer, sizeof(MESSAGE_RES_ATTACK_LEFT_HAND));
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
		//������ 
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

//������ ���� 
void GameServer::update()
{
	//�÷��̾ �׾��ٸ�,
	// 1. �÷��̾� ��ü �ı�, �޸� Ǯ �ݳ�.
	// 2. ���ǿ��� ��ü �ı� ��û. 

	//�����Ӹ��� �����̱�.
}
