#include "freertos_util/Queue_base.hpp"

Queue_base::Queue_base()
{
	m_queue = nullptr;
}
Queue_base::~Queue_base()
{
	if(m_queue)
	{
		vQueueDelete(m_queue);
		m_queue = nullptr;
	}
}