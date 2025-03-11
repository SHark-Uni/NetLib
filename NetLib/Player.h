#pragma once

namespace Core
{


	class Player
	{
	public:
		void Init(const int playerId, const int sessionId);
		void OnPlayerDeath();

		static int generatePlayerId()
		{
			static int playerId = 0;
			return playerId++;
		}
		void Move(const int x, const int y);
		void Attacked(const int damage);

		inline void SetX(short x)
		{
			_X = x;
		}
		inline void SetY(short y)
		{
			_Y = y;
		}
		inline void SetAction(int action)
		{
			_Action = action;
			return;
		}
		inline void SetDirection(char direction)
		{
			_Direction = direction;
		}
		inline char GetDirection() const
		{
			return _Direction;
		}
		inline int GetAction() const
		{
			return _Action;
		}
		inline int GetPlayerId() const
		{
			return _PlayerId;
		}
		inline int GetSessionId() const
		{
			return _SessionId;
		}
		inline int GetHp() const
		{
			return _Hp;
		}
		inline short GetX() const
		{
			return _X;
		}
		inline short GetY() const
		{
			return _Y;
		}
	private:
		int _PlayerId;
		int _SessionId;
		
		int _Action;
		char _Direction;

		short _X;
		short _Y;

		char _Hp;
	};
}