
#include "FreeRTOS.h"

#include <new>

void* operator new(std::size_t size) throw(std::bad_alloc)
{
	void* const ptr = pvPortMalloc(size);

	if(ptr == nullptr)
	{
		throw std::bad_alloc();
	}

	return ptr;
}

void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept
{
	return pvPortMalloc(size);
}

void operator delete(void* ptr) noexcept
{
	vPortFree(ptr);
}