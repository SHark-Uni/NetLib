#pragma once

#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

using namespace Common;

#ifdef 0
namespace Test
{
	void RINGBUFFER_ENQUEUE_ELEMENT_TEST()
	{
		int ret;
		CircularQueue Queue;

		char A = 'a';
		ret = Queue.Enqueue(&A, sizeof(A));
		assert(ret == 1);
		assert(Queue._Front == 0);
		assert(Queue._Rear == 1);

		char arr[16] = { 0, };
		memset(arr, 'b', sizeof(arr));
		ret = Queue.Enqueue(arr, sizeof(arr));
		assert(ret == 16);
		assert(Queue.GetCurrentSize() == 17);
		assert(Queue._Front == 0);
		assert(Queue._Rear == 17);


		char arr2[20] = { 0, };
		memset(arr2, 'c', sizeof(arr2));
		ret = Queue.Enqueue(arr2, sizeof(arr2));
		assert(ret == 20);
		assert(Queue.GetCurrentSize() == 37);
		assert(Queue._Front == 0);
		assert(Queue._Rear == 37);
	}

	void RINGBUFFER_ENQUEUE_ELEMENT_TO_MAX()
	{
		int ret;
		CircularQueue Queue(10);

		char A[10] = { 0, };
		memset(A, 'a', sizeof(A));
		ret = Queue.Enqueue(A, sizeof(A));
		assert(ret == 10);
		assert(Queue._Front == 0);
		assert(Queue._Rear == 10);
		assert(Queue.GetRemainingSize() == 0);
		assert(Queue.GetCurrentSize() == 10);

		//MAX일 때 집어넣는 경우
		ret = Queue.Enqueue(A, sizeof(A));
		assert(ret == 0);
		assert(Queue._Front == 0);
		assert(Queue._Rear == 10);
		assert(Queue.GetRemainingSize() == 0);
		assert(Queue.GetCurrentSize() == 10);
	}

	void RINGBUFFER_DEQEUE_ELEMENT_TEST()
	{
		int ret;
		CircularQueue Queue(10);
		assert(Queue.GetRemainingSize() == 10);

		char buffer[5] = { 'A','B','C','D','E' };
		ret = Queue.Enqueue(buffer, sizeof(buffer));
		assert(ret == 5);
		assert(Queue._Front == 0);
		assert(Queue._Rear == 5);
		assert(Queue.GetCurrentSize() == 5);
		assert(Queue.GetRemainingSize() == 5);

		char dBuffer[5] = { 0 };
		ret = Queue.Dequeue(dBuffer, sizeof(dBuffer));
		assert(ret == 5);
		assert(Queue._Front == 5);
		assert(Queue._Rear == 5);
		assert(Queue.GetCurrentSize() == 0);
		assert(Queue.GetRemainingSize() == 10);
		assert(memcmp(dBuffer, buffer, sizeof(buffer)) == 0);
	}

	void RINGBUFFER_DEQEUE_MANY_TEST()
	{
		int ret;
		CircularQueue Queue(10);
		assert(Queue.GetRemainingSize() == 10);

		char buffer[5] = { 'A','B','C','D','E' };
		ret = Queue.Enqueue(buffer, sizeof(buffer));
		assert(ret == 5);
		assert(Queue._Front == 0);
		assert(Queue._Rear == 5);
		assert(Queue.GetCurrentSize() == 5);
		assert(Queue.GetRemainingSize() == 5);


		char dBuffer_LEN_2[2] = { 0 };
		char dBuffer_LEN_3[3] = { 0 };
		ret = Queue.Dequeue(dBuffer_LEN_2, sizeof(dBuffer_LEN_2));
		assert(ret == sizeof(dBuffer_LEN_2));
		assert(Queue._Front == 2);
		assert(Queue._Rear == 5);
		assert(Queue.GetCurrentSize() == 3);
		assert(Queue.GetRemainingSize() == 7);

		assert(memcmp(dBuffer_LEN_2, buffer, 2) == 0);

		ret = Queue.Dequeue(dBuffer_LEN_3, sizeof(dBuffer_LEN_3));
		assert(ret == sizeof(dBuffer_LEN_3));
		assert(Queue._Front == 5);
		assert(Queue._Rear == 5);
		assert(Queue.GetCurrentSize() == 0);
		assert(Queue.GetRemainingSize() == 10);
		assert(memcmp(dBuffer_LEN_3, buffer + 2, 3) == 0);
	}
	/*====
	*  Front가 0이 아닐 때, 데이터를 잘 잘라서 넣는지 테스트
	====*/
	void RINGBUFFER_ENQUEUE_CHECK_CIRCULAR_ROTATION()
	{
		int ret;
		CircularQueue Queue(10);

		char buffer[5] = { 'a','b','c','d','e' };
		ret = Queue.Enqueue(buffer, sizeof(buffer));
		assert(ret == sizeof(buffer));
		assert(Queue.GetCurrentSize() == sizeof(buffer));
		assert(Queue.GetRemainingSize() == (Queue._Capacity - sizeof(buffer)));

		char dBuffr[5] = { 0, };
		ret = Queue.Dequeue(dBuffr, sizeof(dBuffr));
		assert(ret == sizeof(dBuffr));
		assert(Queue._Front == 5);
		assert(Queue._Rear == 5);
		assert(memcmp(dBuffr, buffer, sizeof(dBuffr)) == 0);

		char maxBuffer[8] = { 'A','B','C','D','E','F','G','H' };
		ret = Queue.Enqueue(maxBuffer, sizeof(maxBuffer));
		assert(ret == sizeof(maxBuffer));
		assert(Queue._Front == 5);
		assert(Queue._Rear == 2);
		assert(Queue.GetCurrentSize() == sizeof(maxBuffer));
		assert(Queue.GetRemainingSize() == Queue._Capacity - sizeof(maxBuffer));

		ret = Queue.Enqueue(maxBuffer, sizeof(maxBuffer));
		assert(ret == 2);
		assert(Queue.GetCurrentSize() == Queue._Capacity);
		assert(Queue.GetRemainingSize() == 0);
		assert(Queue._Front == 5);
		assert(Queue._Rear == 4);

		char Buffer[10] = { 0, };
		char Expected[10] = { 'A','B','C','D','E','F','G','H','A','B' };
		ret = Queue.Dequeue(Buffer, sizeof(Buffer));
		assert(ret == 10);
		assert(Queue.GetCurrentSize() == 0);
		assert(Queue.GetRemainingSize() == 10);
		assert(Queue._Front == 4);
		assert(Queue._Rear == 4);

		assert(memcmp(Buffer, Expected, 10) == 0);
	}

	void RINGBUFFER_DEQUEUE_CHECK_CIRCULAR_ROTATION()
	{
		int ret;

		CircularQueue Queue(10);

		char dummy[5] = { 0, };
		memset(dummy, 'a', sizeof(dummy));
		Queue.Enqueue(dummy, sizeof(dummy));
		Queue.Dequeue(dummy, sizeof(dummy));

		assert(Queue._Front = 5);
		assert(Queue._Rear = 5);

		char data[8] = { 'A','B','C','D','E','F','G','H' };
		Queue.Enqueue(data, sizeof(data));

		char dBuffer[8] = { 0, };
		ret = Queue.Dequeue(dBuffer, sizeof(data));
		assert(ret == 8);
		assert(Queue.GetCurrentSize() == 0);
		assert(Queue.GetRemainingSize() == 10);
		assert(Queue._Front == 2);
		assert(Queue._Rear == 2);
		assert(memcmp(data, dBuffer, sizeof(dBuffer)) == 0);
	}

	void UNIFORM_TEST_RANDOM_ENQUEUE_RANDOM_DEQUEUE()
	{
		const char* PATTERN = "ABCDEFGHIJKLNMOPQRSTUVWXYZ";

		int PATTERN_front = 0;
		int PATTERN_Rear = strlen(PATTERN) - 1;
		const int PATTERN_LENGTH = strlen(PATTERN);

		srand(5);

		CircularQueue Queue(PATTERN_LENGTH);

		while (true)
		{
			int EnqueueLen = rand() % PATTERN_LENGTH + 1;
			int enqueueRet;
			enqueueRet = Queue.Enqueue(PATTERN, EnqueueLen);

			int prev = 0;
			int cur;
			char* dBuffer = new char[enqueueRet];
			while (Queue.GetCurrentSize() != 0)
			{
				cur = rand() % PATTERN_LENGTH + 1;
				Queue.Dequeue(dBuffer + prev, cur);
				prev += cur;
			}

			assert(memcmp(PATTERN, dBuffer, enqueueRet) == 0);
			delete[] dBuffer;
		}

	}
}
#endif