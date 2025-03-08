#pragma once

namespace Common
{
	class CircularQueue
	{
	public:
		friend class Session;
		CircularQueue();
		CircularQueue(int size);
		~CircularQueue()
		{
			delete[] _pBuffer;
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
			//��� �ִٸ�
			if (_Front == _Rear)
			{
				return _Capacity;
			}
			// ���� ��쵵 Ŀ����.
			if (_Front < _Rear)
			{
				return (_Front)+(_Capacity - _Rear);
			}
			else
			{
				return (_Front - _Rear - 1);
			}
		}

		int Enqueue(const char* pMessage, int size);
		int Dequeue(char* out, int size);
		int Peek(char* out, int size);

		inline char* GetBufferPtr()
		{
			return _pBuffer;
		}

		inline char* GetRearPtr() const
		{
			return &_pBuffer[_Rear];
		}
		inline char* GetFrontPtr() const
		{
			return &_pBuffer[_Front];
		}

		inline int GetDirect_EnqueueSize()
		{
			//Front�� 0�� ���, R�� Capacityĭ�� ������ϱ� ������.. +1�� ������.
			if (_Front == 0)
			{
				return _Capacity - _Rear;
			}
			if (_Front <= _Rear)
			{
				//Rear�� ����Ű�� ���� �׻� ����־����.
				return _Capacity - _Rear + 1;
			}
			return _Front - _Rear - 1;
		}
		inline void MoveRear(int size)
		{
			_Rear = (_Rear + size) % (_Capacity + 1);
			return;
		}

		inline int GetDirect_DequeueSize()
		{
			if (_Front <= _Rear)
			{
				return (_Rear - _Front);
			}
			return (_Capacity - _Front + 1);
		}
		inline void MoveFront(int size)
		{
			_Front = (_Front + size) % (_Capacity + 1);
			return;
		}

		void clear();
	private:
		int _Front;
		int _Rear;
		int _Capacity;
		enum {
			DEFAULT_SIZE = 64,
		};
		char* _pBuffer;
	};
}