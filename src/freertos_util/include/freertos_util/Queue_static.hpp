#pragma once

#include "freertos_util/Queue_base.hpp"

#include <array>

template<typename T, size_t LEN>
class Queue_static : public Queue_template_base<T>
{
public:
	Queue_static()
	{
		this->m_queue = xQueueCreateStatic(LEN, sizeof(T), reinterpret_cast<uint8_t*>(m_buf.data()), &m_queue_buf);
	}
//	~Queue_static() override
//	{
//
//	}

protected:
	StaticQueue_t m_queue_buf;

	std::array<T, LEN> m_buf;
};
