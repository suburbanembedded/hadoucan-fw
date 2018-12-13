#pragma once

#include "freertos_util/object_pool/Object_pool_fwd.hpp"

#include <array>
#include <memory>
#include <utility>

template<typename T>
class Object_pool_node
{

public:

	Object_pool_node()
	{
		set_pool(nullptr);
		set_this();
		set_val(nullptr);
	}

	Object_pool_node(Object_pool_base<T>* const pool, T* const val)
	{
		set_pool(pool);
		set_this();
		set_val(val);
	}

	~Object_pool_node()
	{
		//doing this will likely break things
		//better to leak mem?
		//although, if this is going out of scope the pool is likely also being deleted
		//so there will be no mem to leak.....
		//maybe through an exception
		//this never should be called if m_val is not null
		//if(m_val != nullptr)
		//{
			//get_val()->~T();
		//}
	}

	template<typename... Args>
	T* allocate(Args... args)
	{
		if(m_val == nullptr)
		{
			return nullptr;
		}

		::new(m_val) T(std::forward<Args>(args)...);

		return m_val;
	}

	void deallocate()
	{
		if(m_val != nullptr)
		{
			get_val()->~T();
		}
	}

	static Object_pool_node<T>* get_this_from_val(void* val)
	{
		void** ptr = &val;
		return static_cast< Object_pool_node<T>* >(ptr[-1]);
	}

	T* get_val() const
	{
		return static_cast< T* >(m_ptr_pack[2]);
	}

protected:

	void set_pool(Object_pool_base<T>* pool)
	{
		m_ptr_pack[0] = pool;
	}
	void set_this()
	{
		m_ptr_pack[1] = this;
	}
	void set_val(T* val)
	{
		m_ptr_pack[2] = val;
	}

	Object_pool_base<T>* get_pool() const
	{
		return static_cast< Object_pool_base<T>* >(m_ptr_pack[0]);
	}
	Object_pool_node<T>* get_this() const
	{
		return static_cast< Object_pool_node<T>* >(m_ptr_pack[1]);
	}

	Object_pool_node<T>* m_this;
	T* m_val;

	//Object_pool_base<T>* m_pool;
	//Object_pool_node<T>* m_this;
	//T* m_val;

	std::array<void*, 3> m_ptr_pack;
};

