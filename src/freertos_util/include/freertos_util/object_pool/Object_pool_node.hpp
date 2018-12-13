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
		T* const val_ptr = get_val();

		if(val_ptr == nullptr)
		{
			return nullptr;
		}

		::new(val_ptr) T(std::forward<Args>(args)...);

		return val_ptr;
	}

	void deallocate()
	{
		T* const val_ptr = get_val();

		if(val_ptr != nullptr)
		{
			val_ptr->~T();
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
	Object_pool_base<T>* get_pool() const
	{
		return static_cast< Object_pool_base<T>* >(m_ptr_pack[0]);
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

	Object_pool_node<T>* get_this() const
	{
		return static_cast< Object_pool_node<T>* >(m_ptr_pack[1]);
	}

	//0 - Object_pool_base<T>* m_pool;
	//1 - Object_pool_node<T>* m_this;
	//2 - T* m_val;
	std::array<void*, 3> m_ptr_pack;
};

