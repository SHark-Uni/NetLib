#pragma once

#include "MessageFormat.h"
using namespace Common;

namespace Common
{
	class SerializeBuffer;
}
namespace Core
{
	void buildMsg_Header(_BYTE code, _BYTE hlen, _BYTE type, SerializeBuffer* header);
	void buildMsg_createMyCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, SerializeBuffer* message);
	void buildMsg_createOtherCharacter(int id, _BYTE direction, unsigned short x, unsigned short y, _BYTE hp, SerializeBuffer* message);
	void buildMsg_deleteCharacter(int id, SerializeBuffer* message);
	void buildMsg_move_start(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message);
	void buildMsg_move_stop(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message);
	void buildMsg_attack_lefthand(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message);
	void buildMsg_attack_righthand(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message);
	void buildMsg_attack_kick(int id, _BYTE direction, unsigned short x, unsigned short y, SerializeBuffer* message);

	void buildMsg_damage(int attackId, int targetId, _BYTE targetHp, SerializeBuffer* message);
} 