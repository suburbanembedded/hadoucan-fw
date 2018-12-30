
#include "FreeRTOS.h"
#include "task.h"

#include <sys/reent.h>

void memset_volatile(volatile void* ptr, char c, size_t n);


void __malloc_lock(struct _reent* r )
{
	vTaskSuspendAll();
}

void __malloc_unlock(struct _reent* r )
{
	BaseType_t ret = xTaskResumeAll();

	if(ret == pdFALSE )
	{
		taskYIELD();
	}
}

void __env_lock(struct _reent* r )
{
	vTaskSuspendAll();
}

void __env_unlock(struct _reent* r )
{
	BaseType_t ret = xTaskResumeAll();

	if(ret == pdFALSE )
	{
		taskYIELD();
	}
}

void memset_volatile(volatile void* ptr, char c, size_t n)
{
	volatile char* ptr_c = ptr;
	while(n-- > 0)
	{
		*ptr_c++ = c;
	}
}

void* _malloc_r(struct _reent* r, size_t sz)
{
	return pvPortMalloc(sz);
}

void* _calloc_r(struct _reent* r, size_t nmemb, size_t sz)
{
	if( (nmemb == 0) || (sz == 0))
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

void* _realloc_r (struct _reent *r, void* ptr, size_t sz)
{
	if(ptr)
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
	vPortFree(ptr);
}