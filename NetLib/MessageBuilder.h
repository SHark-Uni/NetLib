#pragma once

#include "MessageFormat.h"
#include "SerializeBuffer.h"

#define _WINSOCKAPI_
#include <windows.h>
using namespace Common;

namespace Core
{
	inline void buildMsg_Header(_BYTE code, _BYTE hlen, _BYTE type, Common::SerializeBuffer* message)
	{
		*message << code << hlen << type;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}
	inline void buildMsg_createMyCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, Common::SerializeBuffer* message)
	{
		*message << id << direction << x << y << hp;
		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}
	inline void buildMsg_createOtherCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, Common::SerializeBuffer* message)
	{
		*message << id << direction << x << y << hp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}
	inline void buildMsg_deleteCharacter(int id, Common::SerializeBuffer* message)
	{
		*message << id;
		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}
	inline void buildMsg_move_start(int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}
	inline void buildMsg_move_stop(int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}

		return;
	}
	inline void buildMsg_attack_lefthand(int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}
	inline void buildMsg_attack_righthand(int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}
	inline void buildMsg_attack_kick(int id, _BYTE direction, unsigned short x, unsigned short y, Common::SerializeBuffer* message)
	{
		*message << id << direction << x << y;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}

	inline void buildMsg_damage(int attackId, int targetId, _BYTE targetHp, Common::SerializeBuffer* message)
	{
		*message << attackId << targetId << targetHp;

		if (message->checkFailBit())
		{
			DebugBreak();
		}
		return;
	}
} 