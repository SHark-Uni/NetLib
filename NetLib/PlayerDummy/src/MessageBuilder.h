#pragma once

#include "MessageFormat.h"

namespace Common
{
	void buildMsg_Header(_BYTE code, _BYTE hlen, _BYTE type, MESSAGE_HEADER& header);
	void buildMsg_createMyCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, MESSAGE_RES_CREATE_MY_CHARACTER& message);
	void buildMsg_createOtherCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, MESSAGE_RES_CREATE_OTHER_CHARACTER& message);
	void buildMsg_deleteCharacter(int id, MESSAGE_RES_DELETE_CHARACTER& message);
	void buildMsg_move_start(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_MOVE_START& message);
	void buildMsg_move_stop(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_MOVE_STOP& message);
	void buildMsg_attack_lefthand(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_ATTACK_LEFT_HAND& message);
	void buildMsg_attack_righthand(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_ATTACK_RIGHT_HAND& message);
	void buildMsg_attack_kick(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_ATTACK_KICK& message);

	void buildMsg_damage(int attackId, int targetId, _BYTE targetHp, MESSAGE_RES_DAMAGE& message);
}
