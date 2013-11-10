#include "appmemlib.h"
#include "am_stata.h"

typedef int (*am_sort_compare_fn)(void *p1, void *p2);
#if 0
void merge(int a[], int low, int high, int mid)
{
	int i, j, k, c[50];
	i=low;
	j=mid+1;
	k=low;
	while((i<=mid)&&(j<=high))
	{
		if(a[i]<a[j])
		{
			c[k]=a[i];	
			k++;
			i++;
		}
		else
		{
			c[k]=a[j];
			k++;
			j++;
		}
	}
	while(i<=mid)
	{
		c[k]=a[i];
		k++;
		i++;
	}
	while(j<=high)
	{
		c[k]=a[j];
		k++;
		j++;
	}
	for(i=low;i<k;i++)
	{
		a[i]=c[i];
	}
} 

int mergesort(int a[], int low, int high)
{
	int mid;
	if(low < high)
	{
		mid=(low+high)/2;
		mergesort(a, low, mid);
		mergesort(a, mid+1, high);
		merge(a,low,high,mid);
	}
	return(0);
}
#endif



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

void am_set_ptrval(void *set, void *from, int data_size)
{
	if(data_size == sizeof(UINT32))
	{
		*(UINT32 *)set = *(UINT32 *)from;
	}
	else
	{
		/* TODO : all the possible integral (simple compare) values */
		AM_ASSERT(0);
	}

}

void am_stata_merge(UINT8 *merge_buffer, UINT8 *start_ptr, UINT8 *end_ptr, UINT8 *mid_ptr, int data_size, am_sort_compare_fn sort_fn)
{
	UINT8 *iptr;
	UINT8 *jptr;
	UINT8 *kptr;
	int compare;
	
	iptr = start_ptr;
	jptr = mid_ptr + data_size;
	kptr = merge_buffer;
	
	while( (iptr < mid_ptr) && (jptr < end_ptr))
	{
		compare = sort_fn(iptr, jptr);
	
		if(compare < 0)
		{
			am_set_ptrval(kptr, iptr, data_size);
			kptr += data_size;
			iptr += data_size;
		}
		else
		{
			am_set_ptrval(kptr, jptr, data_size);
			kptr += data_size;
			jptr += data_size;
		}
	}

	while(iptr < mid_ptr)
	{

		am_set_ptrval(kptr, iptr, data_size);
		kptr += data_size;
		iptr += data_size;

	}
	while(jptr < end_ptr)
	{
		am_set_ptrval(kptr, jptr, data_size);
		kptr += data_size;
		jptr += data_size;
	}

	iptr = start_ptr;
	kptr = merge_buffer;

	while(iptr < end_ptr)
	{
		am_set_ptrval(iptr, kptr, data_size);
		iptr += data_size;
		kptr += data_size;
	}



}

void am_stata_merge_sort(UINT8 *merge_buffer, UINT8 *start_ptr, UINT8 *end_ptr, int data_size, am_sort_compare_fn sort_fn)
{
	UINT8 *mid_ptr;

		
	if(start_ptr < end_ptr)
	{
		/* (low + high) / 2 */
		/* but since these are pointers we could overflow with the addition */

		mid_ptr = ((end_ptr - start_ptr) / 2) + start_ptr;  
		am_stata_merge_sort(merge_buffer, start_ptr, mid_ptr, data_size, sort_fn);
		am_stata_merge_sort(merge_buffer, mid_ptr + data_size, end_ptr, data_size, sort_fn);
		am_stata_merge(merge_buffer, start_ptr, end_ptr, mid_ptr, data_size, sort_fn);

	//	merge(a,low,high,mid);
	}
	

}

AM_RETURN am_stata_merge_sort_begin(void *start_ptr, void *end_ptr, int data_size, am_sort_compare_fn sort_fn)
{
	UINT32 size_merge_bytes;
	void *merge_buffer;
	
	AM_ASSERT(start_ptr);
	AM_ASSERT(end_ptr);
	AM_ASSERT(data_size);

	if(end_ptr > start_ptr)
	{
		size_merge_bytes = ((UINT8 *) end_ptr - (UINT8 *)start_ptr) + data_size;
		merge_buffer = AM_VALLOC(size_merge_bytes);

		if(merge_buffer)
		{
			am_stata_merge_sort(merge_buffer, start_ptr, end_ptr, data_size, sort_fn);
			AM_VFREE(merge_buffer);
		}
		else
		{
			return AM_RET_ALLOC_ERR;
		}

	}
	else
	{

		return AM_RET_PARAM_ERR;
	}

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
		
			return am_stata_merge_sort_begin(start_ptr, end_ptr, fd->stata.data_size, sort_fn);
		
		
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
