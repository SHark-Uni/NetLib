#include "Player.h"
#include "PlayerDefine.h"

using namespace Core;

void Player::Init(const int playerId, const int sessionId)
{
	_PlayerId = playerId;
	_SessionId = sessionId;

	_Action = static_cast<int>(PLAYER_DEFAULT::DEFAULT_ACTION);
	_Direction = static_cast<int>(PLAYER_DEFAULT::DEFAULT_DIR);

	_X = static_cast<int>(PLAYER_DEFAULT::SPAWN_X);
	_Y = static_cast<int>(PLAYER_DEFAULT::SPAWN_Y);

	_Hp = static_cast<int>(PLAYER_DEFAULT::PLAYER_HP);
}

void Player::OnPlayerDeath()
{
	/*���࿡ �״�ȿ���� �ִٸ� ����� ����*/
}

void Player::Move(const int x, const int y)
{
	_X += x;
	if (_X <= static_cast<int>(MAX_MAP_BOUNDARY::LEFT))
	{
		_X = static_cast<int>(MAX_MAP_BOUNDARY::LEFT);
		return;
	}
	if (_X >= static_cast<int>(MAX_MAP_BOUNDARY::RIGHT))
	{
		_X = static_cast<int>(MAX_MAP_BOUNDARY::RIGHT);
		return;
	}
	_Y += y;
	if (_Y <= static_cast<int>(MAX_MAP_BOUNDARY::TOP))
	{
		_Y = static_cast<int>(MAX_MAP_BOUNDARY::TOP);
		return;
	}
	if (_Y >= static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		_Y = static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM);
		return;
	}
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
