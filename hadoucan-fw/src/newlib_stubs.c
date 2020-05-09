
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
// static UBaseType_t uxSavedInterruptStatus_malloc;
void __malloc_lock(struct _reent* r)
{
	if(isInterrupt())
	{
		// uxSavedInterruptStatus_malloc = taskENTER_CRITICAL_FROM_ISR();
		for(;;)
		{

		}
	}
	else
	{
		vTaskSuspendAll();
	}
}

void __malloc_unlock(struct _reent* r)
{
	if(isInterrupt())
	{
		//taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus_malloc);
		for(;;)
		{

		}
	}
	else
	{
		BaseType_t ret = xTaskResumeAll();

		if(ret == pdFALSE )
		{
			taskYIELD();
		}
	}
}

// static UBaseType_t uxSavedInterruptStatus_env;
void __env_lock(struct _reent* r)
{
	if(isInterrupt())
	{
		// uxSavedInterruptStatus_env = taskENTER_CRITICAL_FROM_ISR();
		for(;;)
		{
			
		}
	}
	else
	{
		vTaskSuspendAll();
	}
}

void __env_unlock(struct _reent* r)
{
	if(isInterrupt())
	{
		// taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus_env);
		for(;;)
		{
			
		}
	}
	else
	{
		BaseType_t ret = xTaskResumeAll();

		if(ret == pdFALSE )
		{
			taskYIELD();
		}
	}
}

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
