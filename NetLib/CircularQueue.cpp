#include "CircularQueue.h"
#include <assert.h>

using namespace Common;

#include <string.h>

CircularQueue::CircularQueue()
    :_Front(0)
    , _Rear(0)
    , _Capacity(DEFAULT_SIZE)
{
    _Buffer = new char[DEFAULT_SIZE + 1];
}

CircularQueue::CircularQueue(int size)
    :_Front(0)
    ,_Rear(0)
    ,_Capacity(size)
{
    _Buffer = new char[size + 1];
}

//�ִ��� �־��ְ� return.
int CircularQueue::Enqueue(const char* message, int size)
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

        if (rSize >= size)
        {
            memcpy(_Buffer + _Rear, message, size);
            _Rear += size;
            return size;
        }
        //�ɰ��� �־�� �ϴ� ���
        else
        {
            if (size - rSize < fSize)
            {
                fSize = size - rSize;
            }
            memcpy(_Buffer + _Rear, message, rSize);
            memcpy(_Buffer, message + rSize, fSize);
            _Rear = (_Rear + (fSize + rSize)) % (_Capacity + 1);
            return fSize + rSize;
        }
    }
    else
    {
        int cpySize = _Front - _Rear - 1;

        //�����ִ� ����� ����� ���
        if (size < cpySize)
        {
            cpySize = size;
        }
        memcpy(_Buffer + _Rear, message, cpySize);
        _Rear += cpySize;
        return cpySize;
    }

    assert(false);
    return 0;
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
        memcpy(out, _Buffer + _Front, cpySize);
        _Front += cpySize;
        return cpySize;
    }

    int fSize = _Capacity - _Front + 1;
    int cpySize;

    if (size < fSize)
    {
        memcpy(out, _Buffer + _Front, size);
        _Front = (_Front + size) % (_Capacity + 1);
        return size;
    }
    //�ɰ����ϴ� ��� 
    cpySize = _Rear;
    if (size - fSize < cpySize)
    {
        cpySize = size - fSize;
    }
    memcpy(out, _Buffer + _Front, fSize);
    memcpy(out + fSize, _Buffer, cpySize);

    _Front = (_Front + fSize + cpySize) % (_Capacity + 1);
    return fSize + cpySize;
}

int CircularQueue::Peek(char* out, int size)
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
        memcpy(out, _Buffer + _Front, cpySize);
        return cpySize;
    }

    int fSize = _Capacity - _Front + 1;
    int cpySize;

    if (size < fSize)
    {
        memcpy(out, _Buffer + _Front, size);
        return size;
    }
    //�ɰ����ϴ� ��� 
    cpySize = _Rear;
    if (size - fSize < cpySize)
    {
        cpySize = size - fSize;
    }
    memcpy(out, _Buffer + _Front, fSize);
    memcpy(out + fSize, _Buffer, cpySize);
    return fSize + cpySize;
}

void CircularQueue::clear()
{
    _Rear = _Front = 0;
}
