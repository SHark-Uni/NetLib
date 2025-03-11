#include "MessageBuilder.h"


namespace Core
{
	void buildMsg_Header(_BYTE code, _BYTE hlen, _BYTE type, MESSAGE_HEADER& header)
	{
		header._Code = code;
		header._PayloadLen = hlen;
		header._MessageType = type;
		return;
	}

	void buildMsg_createMyCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, MESSAGE_RES_CREATE_MY_CHARACTER& message)
	{
		message._Id = id;
		message._Direction = direction;
		message._X = x;
		message._Y = y;
		message._HP = hp;
		return;
	}

	void buildMsg_createOtherCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, MESSAGE_RES_CREATE_OTHER_CHARACTER& message)
	{
		message._Id = id;
		message._Direction = direction;
		message._X = x;
		message._Y = y;
		message._HP = hp;
		return;
	}

	void buildMsg_deleteCharacter(int id, MESSAGE_RES_DELETE_CHARACTER& message)
	{
		message._Id = id;
		return;
	}

	void buildMsg_move_start(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_MOVE_START& message)
	{
		message._Id = id;
		message._Direction = direction;
		return;
	}

	void buildMsg_move_stop(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_MOVE_STOP& message)
	{
		message._Id = id;
		message._Direction = direction;
		message._X = x;
		message._Y = y;
		return;
	}

	void buildMsg_attack_lefthand(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_ATTACK_LEFT_HAND& message)
	{
		message._Id = id;
		message._Direction = direction;
		message._X = x;
		message._Y = y;
		return;
	}

	void buildMsg_attack_righthand(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_ATTACK_RIGHT_HAND& message)
	{
		message._Id = id;
		message._Direction = direction;
		message._X = x;
		message._Y = y;
		return;
	}

	void buildMsg_attack_kick(int id, _BYTE direction, unsigned short x, unsigned short y, MESSAGE_RES_ATTACK_KICK& message)
	{
		message._Id = id;
		message._Direction = direction;
		message._X = x;
		message._Y = y;
		return;
	}

	void buildMsg_damage(int attackId, int targetId, _BYTE targetHp, MESSAGE_RES_DAMAGE& message)
	{
		message._AttackId = attackId;
		message._HitTargetId = targetId;
		message._HitTargetHp = targetHp;
		return;
	}
}

