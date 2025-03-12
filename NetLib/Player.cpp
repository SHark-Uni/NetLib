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
	/*만약에 죽는효과가 있다면 여기다 구현*/
}

void Player::Move(const short x, const short y)
{
	// 하나라도 막혀있으면 못가는 로직으로 변경해야함. 
	// 이건 MAX값만 체크한 로직임. 이럴경우 대각선으로 가면 뚫림
	if (CheckWallCollision())
	{
		return;
	}

	_X += x;
	if (_X <= static_cast<short>(MAX_MAP_BOUNDARY::LEFT))
	{
		_X = static_cast<short>(MAX_MAP_BOUNDARY::LEFT);
		return;
	}
	if (_X >= static_cast<short>(MAX_MAP_BOUNDARY::RIGHT))
	{
		_X = static_cast<short>(MAX_MAP_BOUNDARY::RIGHT);
		return;
	}
	_Y += y;
	if (_Y <= static_cast<short>(MAX_MAP_BOUNDARY::TOP))
	{
		_Y = static_cast<int>(MAX_MAP_BOUNDARY::TOP);
		return;
	}
	if (_Y >= static_cast<short>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		_Y = static_cast<int>(MAX_MAP_BOUNDARY::BOTTOM);
		return;
	}
}



void Player::Attacked(const int damage)
{
	//양수로 음수로 데미지 받는 회복같은 개념은 없음. 양의 극값은 검사 ㄴ
	_Hp -= damage;
	if (_Hp <= 0)
	{
		_Hp = 0;
	}
	return;
}

bool Core::Player::CheckWallCollision()
{
	if (_X <= static_cast<short>(MAX_MAP_BOUNDARY::LEFT)
		|| _X >= static_cast<short>(MAX_MAP_BOUNDARY::RIGHT)
		|| _Y <= static_cast<short>(MAX_MAP_BOUNDARY::TOP)
		|| _Y >= static_cast<short>(MAX_MAP_BOUNDARY::BOTTOM))
	{
		return true;
	}
	return false;
}
