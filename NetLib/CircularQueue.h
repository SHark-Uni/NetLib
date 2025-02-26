#pragma once

namespace Common
{
	class CircularQueue
	{
	public:
		CircularQueue();
		CircularQueue(int size);
		~CircularQueue()
		{
			delete[] _Buffer;
		}

		inline int GetCurrentSize() const
		{
			if ((_Rear + 1) % (_Capacity + 1) == _Front)
			{
				return _Capacity;
			}

			if (_Front <= _Rear)
			{
				return (_Rear - _Front);
			}
			else
			{
				return (_Capacity - _Front + 1 + _Rear);
			}

		}
		inline int GetRemainingSize() const
		{
			//비어 있다면
			if (_Front == _Rear)
			{
				return _Capacity;
			}
			// 꽉찬 경우도 커버함.
			if (_Front < _Rear)
			{
				return (_Front)+(_Capacity - _Rear);
			}
			else
			{
				return (_Front - _Rear - 1);
			}
		}

		int Enqueue(const char* message, int size);
		int Dequeue(char* out, int size);
		int Peek(char* out, int size);

		void clear();
	private:
		int _Front;
		int _Rear;
		int _Capacity;
		enum {
			DEFAULT_SIZE = 64,
		};
		char* _Buffer;
	};
}