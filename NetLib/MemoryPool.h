#pragma once
#include <utility>

#ifdef MEMORY_POOL_DEBUG
#include <windows.h>
#endif

/* �޸� Ǯ ����
	* -------------------------------------------------------------------------
	|	prevBlock	|  padding(optional) |    currentSlot	  | .... | LastSlot|
	----------------------------------------------------------------------------
*/
namespace Common
{
	template<typename T, size_t BucketSize = 4096>
	class MemoryPool
	{
	public:
		typedef char _BYTE;

		union Slot
		{
			T _Element;
			Slot* _pNext;
#ifdef MEMORY_POOL_DEBUG
			int GUARD;
#endif
		};

		MemoryPool()
			:_CurrentBucket(nullptr)
			, _CurSlot(nullptr)
			, _LastSlot(nullptr)
			, _FreeList(nullptr)
		{

		}

		/*������ ���� ��¥ ������ ������.. ���� Block���� �ٸ� ������ ����Ű�� ��Ȳ�� �������.*/
		MemoryPool(const MemoryPool& other) = delete;
		MemoryPool& operator=(const MemoryPool& rhs) = delete;

		/*=========
			�޸�Ǯ�� �̵��� ����ϱ� �ϴµ�.. ���� ��¥ ��¥ �ʿ��ϳİ� ������ .. ��
		========*/
		MemoryPool(MemoryPool&& other)
			:_CurrentBucket(other._CurrentBucket)
			, _CurSlot(other._CurSlot)
			, _LastSlot(other._LastSlot)
			, _FreeList(other._FreeList)
		{
			other._CurrentBucket = nullptr;
			other._CurSlot = nullptr;
			other._LastSlot = nullptr;
			other._FreeList = nullptr;
		}
		MemoryPool& operator=(MemoryPool&& rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}

			_CurrentBucket = rhs._CurrentBucket;
			_CurSlot = rhs._CurSlot;
			_LastSlot = rhs._LastSlot;
			_FreeList = rhs._FreeList;

			rhs._CurrentBucket = nullptr;
			rhs._CurSlot = nullptr;
			rhs._LastSlot = nullptr;
			rhs._FreeList = nullptr;
		}
#ifndef MEMORY_POOL_DEBUG
		~MemoryPool()
		{
			Slot* cur = _CurrentBucket;
			while (cur != nullptr)
			{
				Slot* prev = cur->_pNext;
				operator delete(reinterpret_cast<void*>(cur));
				cur = prev;
			}
		}
#else
		~MemoryPool()
		{
			Slot* cur = _CurrentBucket;

			if (cur->GUARD != UNDERFLOW_GUARD)
			{
				DebugBreak();
			}
			int* overflowGuard = reinterpret_cast<int*>((_BYTE*)cur + sizeof(int) + BucketSize);
			if (*overflowGuard != OVERFLOW_GUARD)
			{
				DebugBreak();
			}

			while (cur != nullptr)
			{
				Slot* prev = reinterpret_cast<Slot*>(((_BYTE*)cur + sizeof(int)))->_pNext;
				operator delete(reinterpret_cast<void*>(cur));
				cur = prev;
			}
		}
#endif
		T* allocate()
		{
			if (_FreeList != nullptr)
			{
				T* ret = reinterpret_cast<T*>(_FreeList);
				_FreeList = _FreeList->_pNext;
				return ret;
			}

			//slot�� ��� �Ҹ��ߴٸ�
			if (_CurSlot >= _LastSlot)
			{
#ifndef MEMORY_POOL_DEBUG
				allocateBucket();
#else
				allocateBucket_FOR_DEBUG();
#endif
			}
			return reinterpret_cast<T*>(_CurSlot++);
		}

		void deAllocate(T* pMemory)
		{
			if (pMemory != nullptr)
			{
				reinterpret_cast<Slot*>(pMemory)->_pNext = _FreeList;
				_FreeList = reinterpret_cast<Slot*>(pMemory);
			}
		}

		template<class... Args>
		T* allocate_constructor(Args&&... args)
		{
			T* element = allocate();
			new (element) T(std::forward<Args>(args)...);
			return element;
		}

		void deAllocate_Destructor(T* pMemory)
		{
			pMemory->~T();
			deAllocate(pMemory);
		}


		inline size_t getRemainSlot() const
		{
			if (_CurSlot >= _LastSlot)
			{
				return 0;
			}
			return (_LastSlot - _CurSlot) / sizeof(Slot);
		}
		Slot* _CurrentBucket;
		Slot* _CurSlot;
		Slot* _LastSlot;
		Slot* _FreeList;

#ifdef MEMORY_POOL_DEBUG
		enum MEMPORY_GUARD
		{
			UNDERFLOW_GUARD = 0xFFFFAAAA,
			OVERFLOW_GUARD = 0xFFFFAAAA,
		};
#endif
	private:
		void allocateBucket()
		{
			_BYTE* pBucket = reinterpret_cast<_BYTE*>(operator new(BucketSize));

			reinterpret_cast<Slot*>(pBucket)->_pNext = _CurrentBucket;
			_CurrentBucket = reinterpret_cast<Slot*>(pBucket);

			_BYTE* pBucketstart = pBucket + sizeof(Slot*);

			//Need for Align Slot Start Address.
			//ex) align 16byte T is exist, padding will be 8! 
			size_t padding = (alignof(T) - reinterpret_cast<uintptr_t>(pBucket)) % alignof(T);

			_CurSlot = reinterpret_cast<Slot*>(pBucketstart + padding);
			_LastSlot = reinterpret_cast<Slot*>(pBucket + BucketSize - sizeof(Slot) + 1);
		}
#ifdef MEMORY_POOL_DEBUG
		void allocateBucket_FOR_DEBUG()
		{
			_BYTE* pBucket = reinterpret_cast<_BYTE*>(operator new(BucketSize + (sizeof(int) * 2)));
			*(reinterpret_cast<int*>(pBucket)) = UNDERFLOW_GUARD;


			_BYTE* pBlockStart = pBucket + sizeof(int);
			*(reinterpret_cast<int*>((pBlockStart + BucketSize))) = OVERFLOW_GUARD;

			reinterpret_cast<Slot*>(pBlockStart)->_pNext = _CurrentBucket;
			_CurrentBucket = reinterpret_cast<Slot*>(pBucket);

			_BYTE* pBucketstart = pBlockStart + sizeof(Slot*);

			//Need for Align Slot Start Address.
			//ex) align 16byte T is exist, padding will be 8! 
			size_t padding = (alignof(T) - reinterpret_cast<uintptr_t>(pBlockStart)) % alignof(T);

			_CurSlot = reinterpret_cast<Slot*>(pBucketstart + padding);
			_LastSlot = reinterpret_cast<Slot*>(pBlockStart + BucketSize - sizeof(Slot) + 1);
		}
#endif
	};

}