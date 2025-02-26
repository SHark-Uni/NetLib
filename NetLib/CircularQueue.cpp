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

//최대한 넣어주고 return.
int CircularQueue::Enqueue(const char* message, int size)
{
    //꽉찼다면 return 0
    if ((_Rear + 1) % (_Capacity + 1) == _Front)
    {
        return 0;
    }

    //Front가 앞선다면
    if (_Front <= _Rear)
    {
        // 최대한 복사해주고 반환
        int rSize = _Capacity - _Rear + 1;
        int fSize = _Front - 1;

        if (rSize >= size)
        {
            memcpy(_Buffer + _Rear, message, size);
            _Rear += size;
            return size;
        }
        //쪼개서 넣어야 하는 경우
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

        //남이있는 사이즈가 충분한 경우
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
    //비어있는 경우
    if (_Front == _Rear)
    {
        return 0;
    }

    if (_Front < _Rear)
    {
        int cpySize = _Rear - _Front;
        //요청한 크기보다 많이 있는경우
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
    //쪼개야하는 경우 
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
    //비어있는 경우
    if (_Front == _Rear)
    {
        return 0;
    }

    if (_Front < _Rear)
    {
        int cpySize = _Rear - _Front;
        //요청한 크기보다 많이 있는경우
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
    //쪼개야하는 경우 
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
