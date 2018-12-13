#pragma once

#include "freertos_util/Queue_static_pod.hpp"

#include "freertos_util/object_pool/Object_pool_node.hpp"

#include <type_traits>
#include <utility>

template< typename T>
class Object_pool_base
{
public:

	typedef std::aligned_storage_t<sizeof(T), alignof(T)> Aligned_T;
	typedef Object_pool_node<T> Node_T;

	Object_pool_base() = default;
	virtual ~Object_pool_base()
	{

	}

	virtual void deallocate(T* const ptr) = 0;
	virtual void deallocate(Node_T* const node) = 0;

protected:

};

template< typename T, size_t LEN >
class Object_pool : public Object_pool_base<T>
{

public:

	using typename Object_pool_base<T>::Aligned_T;
	using typename Object_pool_base<T>::Node_T;

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

	void deallocate(T* const ptr) override
	{
		Node_T* node = Node_T::get_this_from_val(ptr);
		node->deallocate();

		if(!m_free_nodes.push_back(node))
		{
			//this should never fail
			//very bad if this fails
		}
	}

	void deallocate(Node_T* const node) override
	{
		node->deallocate();

		if(!m_free_nodes.push_back(node))
		{
			//this should never fail
			//very bad if this fails
		}
	}

	class Node_T_deleter
	{
	public:
		void operator()(T* ptr) const
		{
			Node_T* node = Node_T::get_this_from_val(ptr);
			Object_pool_base<T>* pool = node->get_pool();

			pool->deallocate(node);
		}
	};

	typedef std::unique_ptr<T, Node_T_deleter> unique_node_ptr;

	template<typename... Args>
	unique_node_ptr allocate_unique(const TickType_t xTicksToWait, Args... args)
	{
		T* val = allocate(xTicksToWait, std::forward<Args>(args)...);

		return unique_node_ptr(val);
	}

	template<typename... Args>
	unique_node_ptr allocate_unique(Args... args)
	{
		T* val = allocate(std::forward<Args>(args)...);

		return unique_node_ptr(val);
	}

protected:

	std::array<Aligned_T, LEN> m_mem_pool;
	std::array<Node_T,    LEN> m_node_pool;

	Queue_static_pod<Node_T*, LEN> m_free_nodes;
};