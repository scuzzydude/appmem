#include "appmemlib.h"
#include "am_stata.h"

typedef int (*am_sort_compare_fn)(void *p1, void *p2);

int am_unsigned_intg_sort_comp32(void *p1, void *p2)
{
	UINT32 a1 = *(UINT32 *)p1;
	UINT32 a2 = *(UINT32 *)p2;

	if(a1 == a2)
	{
		return 0;
	}
	else if(a1 < a2)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}


AM_RETURN am_stata_merge_sort(void *start_ptr, void *end_ptr, int data_size, am_sort_compare_fn sort_fn)
{

	return AM_RET_GOOD;

}

AM_RETURN am_stata_sort(AM_HANDLE handle, void * p1, UINT64 l1)
{
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	AM_SORT_PARAM_U *pSort = p1;
	void *start_ptr;
	void *end_ptr;
	UINT8 *byte_ptr;
	am_sort_compare_fn sort_fn = NULL;

	if(fd && p1 && (l1 >= sizeof(AM_SORT_PARAM_U)) )
	{
		/* TODO - On devices, we might not want todo recursive versions of a sort */
		/* Validate the range of sort vs your stack, etc here, and choose alternative method */
		/* or Return error */
		if(AM_SORT_TYPE_STATA_INTEGRAL_MERGE == pSort->common.type)
		{
			/* size for stata is max index + 1 i.e. array[100] size if 100 */
			if(pSort->stata_integral.start_idx < fd->stata.size)
			{
				byte_ptr = (UINT8 *)fd->stata.data;
				byte_ptr += (pSort->stata_integral.start_idx * fd->stata.data_size);
				start_ptr = byte_ptr;
			}
			else
			{
				return AM_RET_PARAM_ERR;
			}
	
			if(pSort->stata_integral.end_idx < fd->stata.size)
			{
				/* Lazy man's way to sort the whole array */
				if(0 == pSort->stata_integral.end_idx)
				{
					byte_ptr = (UINT8 *)fd->stata.data;
					byte_ptr += ((fd->stata.size - 1) * fd->stata.data_size);		
					end_ptr = byte_ptr;
				}
				else
				{
					byte_ptr = (UINT8 *)fd->stata.data;
					byte_ptr += (pSort->stata_integral.end_idx * fd->stata.data_size);
					end_ptr = byte_ptr;
				
				}
			}
			else
			{
				return AM_RET_PARAM_ERR;
			}
	
			if(pSort->stata_integral.data_signed)
			{
				/* TODO - just more compare functions */
				return AM_RET_PARAM_ERR;
	
			}
			else
			{
					
				if(sizeof(UINT32) == fd->stata.data_size)
				{
					sort_fn = am_unsigned_intg_sort_comp32;
				}
				else
				{
					/* TODO - basic array functions only support 32 bit for now */
					/* But except for the compare function, the interface should be generic */	
					return AM_RET_PARAM_ERR;
				}
			}
		
			return am_stata_merge_sort(start_ptr, end_ptr, fd->stata.data_size, sort_fn);
		
		
		}
		else
		{
			return AM_RET_PARAM_ERR;
		}
		
	
	
	}
	else
	{
		return AM_RET_PARAM_ERR;
	}
	return AM_RET_GOOD;


}


AM_RETURN am_stata_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}
AM_RETURN am_stata_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}


/* 32 bit */
AM_RETURN am_stata_read_multi_idx32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}
AM_RETURN am_stata_write_multi_idx32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}

AM_RETURN am_stata_read_idx32(AM_HANDLE handle, void *p1, void *p2)
{
	UINT32 idx = *(UINT32 *)p1;
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
    UINT32 *ptr = &((UINT32 *)fd->stata.data)[idx];    


	PUT32_TO_USER(ptr, p2);
    

	return AM_RET_GOOD;

}
AM_RETURN am_stata_write_idx32(AM_HANDLE handle, void *p1, void *p2)
{
	UINT32 idx = *(UINT32 *)p1;
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 *ptr = &((UINT32 *)fd->stata.data)[idx];

	GET32_FROM_USER(ptr, p2);


	return AM_RET_GOOD;

}
