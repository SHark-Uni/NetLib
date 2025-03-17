#pragma once
#include <utility>
#include <new>
namespace Common
{
	//objectpool 
	template<typename T, size_t BucketCount = 256>
	class ObjectPool
	{
	public:
		typedef char _BYTE;
		ObjectPool()
			:_CurBucket(nullptr)
			, _CurSlot(nullptr)
			, _LastSlot(nullptr)
			, _FreeList(nullptr)
		{

		}

		~ObjectPool()
		{
			Slot* cur = _CurBucket;
			while (cur != nullptr)
			{
				Slot* prev = cur->_pNext;
				operator delete(reinterpret_cast<void*>(cur));
				cur = prev;
			}
		}

		T* allocate()
		{
			T* ret;
			if (_FreeList != nullptr)
			{
				ret = reinterpret_cast<T*>(&_CurSlot->_Data);
				_FreeList = _FreeList->_pNext;
				return ret;
			}

			//다 소모함 
			if (_CurSlot >= _LastSlot)
			{
				allocateBucket();
			}
			ret = reinterpret_cast<T*>(&_CurSlot->_Data);
			_CurSlot++;
			return ret;
		}

		template<typename U, class... Args>
		void construct(U* p, Args&&... args)
		{
			new (p) U(std::forward<Args>(args)...);
		}

		template<class... Args>
		T* allocate_constructor(Args&&... args)
		{
			T* element = allocate();
			new (static_cast<void*>(element)) T(std::forward<Args>(args)...);
			return element;
		}

		void deAllocate_destructor(T* pMemory)
		{
			pMemory->~T();
			deAllocate(pMemory);
		}

		void deAllocate(T* addr)
		{
			if (addr != nullptr)
			{
				Slot* retSlot = reinterpret_cast<Slot*>(reinterpret_cast<_BYTE*>(addr) - sizeof(Slot*));
				retSlot->_pNext = _FreeList;
				_FreeList = retSlot;
				return;
			}
		}
		static ObjectPool<T, BucketCount>& getInstance()
		{
			static ObjectPool<T, BucketCount> _singleton;
			return _singleton;
		}
	private:
		struct Slot
		{
			struct Slot* _pNext;
			T _Data;
		};

		void allocateBucket()
		{
			Slot* newBucket = reinterpret_cast<Slot*>(operator new(sizeof(Slot) * (BucketCount + DUMMY_SIZE)));
			newBucket->_pNext = _CurBucket;
			_CurBucket = newBucket;
			_CurSlot = _CurBucket + DUMMY_SIZE;
			_LastSlot = newBucket + (BucketCount + DUMMY_SIZE);
		}

		enum {
			DUMMY_SIZE = 1,
		};

		Slot* _CurBucket;
		Slot* _CurSlot;
		Slot* _LastSlot;
		Slot* _FreeList;
	};
}