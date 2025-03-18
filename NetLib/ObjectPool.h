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
		
		inline bool empty() const
		{
			return (_FreeList == nullptr);
		}

		T* allocate()
		{
			T* ret;
			if (_FreeList != nullptr)
			{
				ret = reinterpret_cast<T*>(&_FreeList->_Data);
				_FreeList = _FreeList->_pNext;
				return ret;
			}

			//�� �Ҹ��� 
			if (_CurSlot >= _LastSlot)
			{
				allocateBucket();
			}

			ret = reinterpret_cast<T*>(&_CurSlot->_Data);
			_CurSlot++;
			return ret;
		}

		template<class... Args>
		T* allocate_constructor(Args&&... args)
		{
			// ������ �ִ� ����� ��Ȱ���ϴ� ���..
			T* element;

			// ������Ʈ ���� �����͵��� ��Ȱ���� ���x.
			element = allocate();
			new (static_cast<void*>(element)) T(std::forward<Args>(args)...);
			return element;
		}

		//��ü�� �����͸� ���°��, �� �����͸� �ȳ����ְ� ���� �� ����.
		template<class... Args>
		T* allocate_reuse(Args&&... args)
		{
			T* element;
			if (_FreeList != nullptr)
			{
				element = &_FreeList->_Data;
				_FreeList = _FreeList->_pNext;
				return element;
			}

			//��¥ ���Ӱ� �Ҵ����־�� �ϴ� ����̶��
			if (_CurSlot >= _LastSlot)
			{
				allocateBucket();
			}

			element = reinterpret_cast<T*>(&_CurSlot->_Data);
			new (static_cast<void*>(element)) T(std::forward<Args>(args)...);
			_CurSlot++;
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