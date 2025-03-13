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
