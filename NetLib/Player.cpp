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

}

void Player::Move(const int x, const int y)
{

}



void Player::Attacked(const int damage)
{

}
