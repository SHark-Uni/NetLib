#pragma once

namespace Common
{
	class CircularQueue
	{
	public:
		CircularQueue(int size);
		~CircularQueue()
		{
			delete[] _Buffer;
		}

		/*====
		* 사용중인 크기 / 사용할 수 있는 크기
		======*/
		inline int GetDequeueSize() const
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
		inline int GetEnqueueSize() const
		{
			//비어 있다면
			if (_Front == _Rear)
			{
				return _Capacity;
			}
			// 꽉찬 경우도 커버함.
			if (_Front < _Rear)
			{
				return (_Front) + (_Capacity - _Rear );
			}
			else
			{
				return (_Front - _Rear - 1);
			}
		}

		/*
			Enqueue시 불가능하다면 0
		*/
		int Enqueue(const char* message, int size);
		int Dequeue(char* out, int size);
		int Peek(char* out, int size);

	private:
		int _Front;
		int _Rear;

		int _Capacity;
		char* _Buffer;
	};
}