
#include "stm32h7xx.h"

#include "FreeRTOS.h"
#include "task.h"

#include <sys/reent.h>
#include <stdlib.h>
#include <stdbool.h>

void memset_volatile(volatile void* ptr, char c, size_t n);

bool isInterrupt()
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

void memset_volatile(volatile void* ptr, char c, size_t n)
{
	volatile char* ptr_c = ptr;
	while(n-- > 0)
	{
		*ptr_c++ = c;
	}
}

#if 1
void* _malloc_r(struct _reent* r, size_t sz)
{
	return pvPortMalloc(sz);
}

void* _calloc_r(struct _reent* r, size_t nmemb, size_t sz)
{
	if( (nmemb == 0) || (sz == 0) )
	{
		return NULL;
	}

	if( (SIZE_MAX / nmemb) < sz)
	{
		return NULL;	
	}

	const size_t num_char = nmemb*sz;
	void* ptr = _malloc_r(r, num_char);

	if(ptr != NULL)
	{
		memset_volatile(ptr, 0, num_char);
	}

	return ptr;
}

void* _realloc_r(struct _reent *r, void* ptr, size_t sz)
{
	if(ptr != NULL)
	{
		if(sz == 0)
		{
			_free_r(r, ptr);
		}
		return NULL;
	}

	return _malloc_r(r, sz);
}

void _free_r(struct _reent* r, void* ptr)
{
	if(ptr != NULL)
	{
		vPortFree(ptr);
	}
}
#endif
