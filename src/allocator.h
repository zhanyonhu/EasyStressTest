#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include "commondef.h"
#include <boost/pool/object_pool.hpp>

namespace std
{
	template <typename T, typename MemoryNode>
	class stl_allocator : 
		public std::allocator<T>
	{
	public:
		template<class Other, typename MemoryNode>
		struct rebind
		{
			typedef stl_allocator<Other, MemoryNode> other;
		};

		stl_allocator() 
		{ 
			m_pool = NULL;
		}
		stl_allocator(stl_allocator<T, MemoryNode> const&)
		{
			m_pool = NULL;
		}
		stl_allocator(boost::object_pool<MemoryNode> * pool)
		{
			m_pool = pool;
		}
		stl_allocator<T, MemoryNode>& operator=(stl_allocator<T, MemoryNode> const& r)
		{ 
			m_pool = r.pool();
			return (*this); 
		}
		template<class Other, typename OtherMemoryNode>
		stl_allocator(stl_allocator<Other, OtherMemoryNode> const&r)
		{
			ASSERT(sizeof(MemoryNode) == sizeof(OtherMemoryNode));
			if (sizeof(MemoryNode) == sizeof(OtherMemoryNode))
			{
				m_pool = (boost::object_pool<MemoryNode> *)((stl_allocator<Other, OtherMemoryNode> &)r).pool();
			}
			else
			{
				m_pool = NULL;
			}
		}
		template<class Other, typename OtherMemoryNode>
		stl_allocator<T, MemoryNode>& operator=(stl_allocator<Other, OtherMemoryNode> const&)
		{
			m_pool = r.pool();
			return (*this); 
		}

		pointer allocate(size_type count) 
		{ 
			if (m_pool==NULL)
			{
				return (allocator<T>::allocate(count));
			}

			pointer pt = (pointer)m_pool->construct();
			if (pt==NULL)
			{
				throw std::bad_alloc();
			}

			return pt;
		}
		void deallocate(pointer ptr, size_type count) 
		{
			if (m_pool == NULL)
			{
				return (allocator<T>::deallocate(ptr, count));
			}

			ASSERT(m_pool->is_from((MemoryNode *)ptr));
			m_pool->destroy((MemoryNode *)ptr);
		}

		boost::object_pool<MemoryNode> * pool()
		{
			return m_pool;
		}

		void setpool(boost::object_pool<MemoryNode> * pool)
		{
			m_pool = pool;
		}

	protected:
		boost::object_pool<MemoryNode> * m_pool;
	};
};

#endif	/*_THREADPOLL_H_*/