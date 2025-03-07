#pragma once
#include <utility> 


/* 메모리 풀 구조
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

		MemoryPool()
			:_CurrentBucket(nullptr)
			, _CurSlot(nullptr)
			, _LastSlot(nullptr)
			, _FreeList(nullptr)
		{

		}

		/*복사할 일은 진짜 없을거 같은데.. 같은 Block들을 다른 곳에서 가리키는 상황은 없어야함.*/
		MemoryPool(const MemoryPool& other) = delete;
		MemoryPool& operator=(const MemoryPool& rhs) = delete;

		/*=========
	        메모리풀의 이동은 허용하긴 하는데.. 막상 진짜 진짜 필요하냐고 물으면 .. 글쎄 일단 해놓고 나중에 풀어보자.
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
		~MemoryPool()
		{
			Slot* cur = _CurrentBucket;
			while (_CurrentBucket != nullptr)
			{
				Slot* prev = _CurrentBucket->_pNext;
				operator delete(reinterpret_cast<void*>(cur));
				cur = prev;
			}
		}

		T* allocate()
		{
			if (_FreeList != nullptr)
			{
				T* ret = reinterpret_cast<T*>(_FreeList);
				_FreeList = _FreeList->_pNext;
				return ret;
			}

			//slot을 모두 소모했다면
			if (_CurSlot >= _LastSlot)
			{
				allocateBucket();
			}
			return reinterpret_cast<Slot*>(_CurSlot++);
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
	private:
		union Slot
		{
			T _Element;
			Slot* _pNext;
		};

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
		Slot* _CurrentBucket;
		Slot* _CurSlot;
		Slot* _LastSlot;
		Slot* _FreeList;
	};


}