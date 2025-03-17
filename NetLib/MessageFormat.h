#pragma once

namespace Common
{
	constexpr char SIGNITURE = 0x89;
	typedef char _BYTE;
	enum class MESSAGE_DEFINE : int
	{
		RES_CREATE_MY_CHARACTER = 0,
		RES_CREATE_OTHER_CHARACTER,
		RES_DELETE_CHARACTER,

		REQ_MOVE_START = 10,
		RES_MOVE_START,
		REQ_MOVE_STOP,
		RES_MOVE_STOP,

		REQ_ATTACK_LEFT_HAND = 20,
		RES_ATTACK_LEFT_HAND,
		REQ_ATTACK_RIGHT_HAND,
		RES_ATTACK_RIGHT_HAND,
		REQ_ATTACK_KICK,
		RES_ATTACK_KICK,

		RES_DAMAGE = 30,

		REQ_SYNC = 250,
		RES_SYNC,
	};
	enum class MOVE_DIRECTION
	{
		LEFT = 0,
		LEFT_TOP =1,
		TOP = 2,
		RIGHT_TOP = 3,
		RIGHT = 4,
		RIGHT_BOTTOM = 5,
		BOTTOM = 6,
		LEFT_BOTTOM = 7
	};

	/* Client에서 위의 MOVE DIR을 재활용했기 때문에, 0,4를 사용*/
	enum class CHARCTER_DIRECTION_2D
	{
		LEFT = 0,
		RIGHT = 4
	};
	/*
		BYTE	byCode;			// 패킷코드 0x89 고정.
		BYTE	bySize;			// 패킷 사이즈.
		BYTE	byType;			// 패킷타입.
	*/
	#pragma pack(push, 1)
	typedef struct MESSAGE_HEADER
	{
		_BYTE _Code;
		_BYTE _PayloadLen;
		_BYTE _MessageType;
	}header_t;

	typedef struct MESSAGE_RES_CREATE_MY_CHARACTER
	{
		int _Id;
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
		_BYTE _HP;
	}res_createMyCharacter_t;

	typedef struct MESSAGE_RES_CREATE_OTHER_CHARACTER
	{
		int _Id;
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
		_BYTE _HP;
	}res_createOtherCharacter_t;

	typedef struct MESSAGE_RES_DELETE_CHARACTER
	{
		int _Id;
	}res_deleteCharacter_t;

	typedef struct MESSAGE_REQ_MOVE_START
	{
		_BYTE _Direction; // 8방향
		unsigned short _X;
		unsigned short _Y;
	}req_move_start_t;

	typedef struct MESSAGE_RES_MOVE_START
	{
		int _Id;
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
	}res_move_start_t;

	typedef struct MESSAGE_REQ_MOVE_STOP
	{
		_BYTE _Direction; //좌 / 우
		unsigned short _X;
		unsigned short _Y;
	}req_move_stop_t;

	typedef struct MESSAGE_RES_MOVE_STOP
	{
		int _Id;
		_BYTE _Direction; //좌/우
		unsigned short _X;
		unsigned short _Y;
	}res_move_stop_t;

	typedef struct MESSAGE_REQ_ATTACK_LEFT_HAND
	{
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
	}req_attack_lefthand_t;

	typedef struct MESSAGE_RES_ATTACK_LEFT_HAND
	{
		int _Id;
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
	}res_attack_lefthand_t;

	typedef struct MESSAGE_REQ_ATTACK_RIGHT_HAND
	{
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
	}req_attack_righthand_t;

	typedef struct MESSAGE_RES_ATTACK_RIGHT_HAND
	{
		int _Id;
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
	}res_attack_righthand_t;
	
	typedef struct MESSAGE_REQ_ATTACK_KICK
	{
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
	}req_attack_kick_t;

	typedef struct MESSAGE_RES_ATTACK_KICK
	{
		int _Id;
		_BYTE _Direction;
		unsigned short _X;
		unsigned short _Y;
	}res_attack_kick_t;

	typedef struct MESSAGE_RES_DAMAGE
	{
		int _AttackId;
		int _HitTargetId;
		_BYTE _HitTargetHp;
	}res_damage_t;
	#pragma pack(pop)

}