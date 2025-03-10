#include "CircularQueue.h"
#include <assert.h>

using namespace Common;
#include <string.h>

CircularQueue::CircularQueue()
	:_Front(0)
	, _Rear(0)
	, _Capacity(DEFAULT_SIZE)
{
	_pBuffer = new char[DEFAULT_SIZE + 1];
}

CircularQueue::CircularQueue(int size)
	:_Front(0)
	, _Rear(0)
	, _Capacity(size)
{
	_pBuffer = new char[size + 1];
}

//�ִ��� �־��ְ� return.
int CircularQueue::Enqueue(const char* pMessage, int size)
{
	//��á�ٸ� return 0
	if ((_Rear + 1) % (_Capacity + 1) == _Front)
	{
		return 0;
	}

	//Front�� �ռ��ٸ�
	if (_Front <= _Rear)
	{
		// �ִ��� �������ְ� ��ȯ
		int rSize = _Capacity - _Rear + 1;
		int fSize = _Front - 1;

		//FRONT�� 0�̸� ���ʿ� �ɰ��� ���� ���� �����ʳ� 
		if (fSize < 0)
		{
			--rSize;
		}
		if (size <= rSize)
		{
			memcpy(_pBuffer + _Rear, pMessage, size);
			_Rear = (_Rear + size) % (_Capacity + 1);
			return size;
		}
		//�ɰ��� �־�� �ϴ� ���
		if (size < fSize + rSize)
		{
			fSize = size - rSize;
		}
		memcpy(_pBuffer + _Rear, pMessage, rSize);
		memcpy(_pBuffer, pMessage + rSize, fSize);
		_Rear = (_Rear + (fSize + rSize)) % (_Capacity + 1);
		return fSize + rSize;
	}

	int cpySize = _Front - _Rear - 1;
	//�����ִ� ����� ����� ���
	if (size < cpySize)
	{
		cpySize = size;
	}
	memcpy(_pBuffer + _Rear, pMessage, cpySize);
	_Rear += cpySize;
	return cpySize;
}

int CircularQueue::Dequeue(char* out, int size)
{
	//����ִ� ���
	if (_Front == _Rear)
	{
		return 0;
	}

	if (_Front < _Rear)
	{
		int cpySize = _Rear - _Front;
		//��û�� ũ�⺸�� ���� �ִ°��
		if (size < cpySize)
		{
			cpySize = size;
		}
		memcpy(out, _pBuffer + _Front, cpySize);
		_Front += cpySize;
		return cpySize;
	}

	int fSize = _Capacity - _Front + 1;
	int cpySize;

	if (size < fSize)
	{
		memcpy(out, _pBuffer + _Front, size);
		_Front = (_Front + size) % (_Capacity + 1);
		return size;
	}
	//�ɰ����ϴ� ��� 
	cpySize = _Rear;
	if (size - fSize < cpySize)
	{
		cpySize = size - fSize;
	}
	memcpy(out, _pBuffer + _Front, fSize);
	memcpy(out + fSize, _pBuffer, cpySize);

	_Front = (_Front + fSize + cpySize) % (_Capacity + 1);
	return fSize + cpySize;
}

int CircularQueue::Peek(char* out, int size) const
{
	//����ִ� ���
	if (_Front == _Rear)
	{
		return 0;
	}

	if (_Front < _Rear)
	{
		int cpySize = _Rear - _Front;
		//��û�� ũ�⺸�� ���� �ִ°��
		if (size < cpySize)
		{
			cpySize = size;
		}
		memcpy(out, _pBuffer + _Front, cpySize);
		return cpySize;
	}

	int fSize = _Capacity - _Front + 1;
	int cpySize;

	if (size < fSize)
	{
		memcpy(out, _pBuffer + _Front, size);
		return size;
	}
	//�ɰ����ϴ� ��� 
	cpySize = _Rear;
	if (size - fSize < cpySize)
	{
		cpySize = size - fSize;
	}
	memcpy(out, _pBuffer + _Front, fSize);
	memcpy(out + fSize, _pBuffer, cpySize);
	return fSize + cpySize;
}

void CircularQueue::clear()
{
	_Rear = _Front = 0;
}
