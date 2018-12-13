#pragma once

#include "freertos_util/Queue_static.hpp"

#include "freertos_util/object_pool/Object_pool_node.hpp"

#include <type_traits>
#include <utility>

template< typename T>
class Object_pool_base
{

public:

	Object_pool_base() = default;
	virtual ~Object_pool_base()
	{

	}


protected:

	typedef std::aligned_storage_t<sizeof(T), alignof(T)> Aligned_T;

	typedef Object_pool_node<T> Node_T;
};

template< typename T, size_t LEN >
class Object_pool : public Object_pool_base<T>
{

public:

	Object_pool()
	{
		for(size_t i = 0; i < LEN; i++)
		{
			void* mem_ptr = &m_mem_pool[i];
			m_node_pool[i] = Node_T(this, static_cast<T*>(mem_ptr));
			m_free_nodes.push_back(&m_node_pool[i]);
		}
	}

	template<typename... Args>
	T* allocate(const TickType_t xTicksToWait, Args... args)
	{
		Node_T* node = nullptr;
		if(!m_free_nodes.pop_front(&node, xTicksToWait))
		{
			return false;
		}

		return node->allocate(std::forward<Args>(args)...);
	}

	template<typename... Args>
	T* allocate(Args... args)
	{
		Node_T* node = nullptr;
		if(!m_free_nodes.pop_front(&node))
		{
			return nullptr;
		}

		return node->allocate(std::forward<Args>(args)...);
	}

	void deallocate(T* const ptr)
	{
		Node_T* node = Node_T::get_this_from_val(ptr);
		node->deallocate();

		if(!m_free_nodes.push_back(node))
		{
			//this should never fail
			//very bad if this fails
		}
	}

protected:

	using typename Object_pool_base<T>::Aligned_T;
	using typename Object_pool_base<T>::Node_T;

	std::array<Aligned_T, LEN> m_mem_pool;
	std::array<Node_T,    LEN> m_node_pool;

	Queue_static<Node_T*, LEN> m_free_nodes;
};