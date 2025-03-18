#define _WINSOCKAPI_
#include "MessageBuilder.h"
#include "SerializeBuffer.h"

#include <windows.h>

namespace Core
{
	void buildMsg_Header(_BYTE code, _BYTE hlen, _BYTE type, SerializeBuffer* message)
	{
		*message << code << hlen << type;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}

	void buildMsg_createMyCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, SerializeBuffer* message)
	{
		*message << id << direction << x << y << hp;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}

	void buildMsg_createOtherCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, SerializeBuffer* message)
	{
		*message << id << direction << x << y << hp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}

	void buildMsg_deleteCharacter(int id, SerializeBuffer* message)
	{
		*message << id;
		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}

	void buildMsg_move_start(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}

	void buildMsg_move_stop(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}

	void buildMsg_attack_lefthand(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}

	void buildMsg_attack_righthand(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}

	void buildMsg_attack_kick(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}

	void buildMsg_damage(int attackId, int targetId, _BYTE targetHp, SerializeBuffer* message)
	{
		*message << attackId << targetId << targetHp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}
}

