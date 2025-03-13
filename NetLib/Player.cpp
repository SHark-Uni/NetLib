#include "Player.h"
#include "PlayerDefine.h"

#include <stdlib.h>

using namespace Core;

void Player::Init(const int playerId, const int sessionId)
{
	_PlayerId = playerId;
	_SessionId = sessionId;

	_Action = static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION);
	_Direction = static_cast<int>(PLAYER_DEFAULT::DEFAULT_DIR);

	_X = generateSpawnX();
	_Y = generateSpawnY();

	_Hp = static_cast<int>(PLAYER_DEFAULT::PLAYER_HP);
}

int Player::generateSpawnY() const
{
	return (rand() % (static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM) - static_cast<int>(MAX_MAP_BOUNDARY::TOP)) + static_cast<int>(MAX_MAP_BOUNDARY::TOP));
}

int Player::generateSpawnX() const
{
	return (rand() % (static_cast<int>(MAX_MAP_BOUNDARY::RIGHT) - static_cast<int>(MAX_MAP_BOUNDARY::LEFT)) + static_cast<int>(MAX_MAP_BOUNDARY::LEFT));
}
void Player::Move(const short x, const short y)
{
	// �ϳ��� ���������� ������ �������� �����ؾ���. 
	// �̰� MAX���� üũ�� ������. �̷���� �밢������ ���� �ո�

	int nextX = _X + x;
	int nextY = _Y + y;

	//���࿡, �������� ������ ���� ���̶�� �������. (������ x)
	if (CheckWallCollision(nextX, nextY))
	{
		return;
	}
	
	_X += x;
	_Y += y;
	return;
}



void Player::Attacked(const int damage)
{
	//����� ������ ������ �޴� ȸ������ ������ ����. ���� �ذ��� �˻� ��
	_Hp -= damage;
	if (_Hp <= 0)
	{
		_Hp = 0;
	}
	return;
}

bool Core::Player::CheckWallCollision(const int x, const int y)
{
	if (x < static_cast<short>(MAX_MAP_BOUNDARY::LEFT)
		|| x > static_cast<short>(MAX_MAP_BOUNDARY::RIGHT)
		|| y < static_cast<short>(MAX_MAP_BOUNDARY::TOP)
		|| y > static_cast<short>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return true;
	}
	return false;
}
