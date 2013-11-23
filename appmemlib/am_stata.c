/*****************************************************************************************

Copyright (c) 2013, Brandon Awbrey
All rights reserved.

https://github.com/scuzzydude/appmem



Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************************/
#include "appmemlib.h"
#include "am_stata.h"
#include "am_targ.h"

typedef int (*am_sort_compare_fn)(void *p1, void *p2);


int am_unsigned_intg_sort_comp32(void *p1, void *p2)
{
	UINT32 a1 = *(UINT32 *)p1;
	UINT32 a2 = *(UINT32 *)p2;
#if 0
	if(a1 == a2)
	{
		return 0;
	}
	else 
#endif
		if(a1 < a2)
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
	
	while( (iptr <= mid_ptr) && (jptr <= end_ptr))
	{
		compare = sort_fn(iptr, jptr);
	
		if(compare ==  -1)
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

	while(iptr <= mid_ptr)
	{

		am_set_ptrval(kptr, iptr, data_size);
		kptr += data_size;
		iptr += data_size;

	}
	while(jptr <= end_ptr)
	{
		am_set_ptrval(kptr, jptr, data_size);
		kptr += data_size;
		jptr += data_size;
	}

	iptr = start_ptr;
	kptr = merge_buffer;

	while(iptr <= end_ptr)
	{
		am_set_ptrval(iptr, kptr, data_size);
		iptr += data_size;
		kptr += data_size;
	}



}

void am_stata_merge_sort(UINT8 *merge_buffer, UINT8 *start_ptr, UINT8 *end_ptr, int data_size, am_sort_compare_fn sort_fn)
{
	UINT8 *mid_ptr;
	UINT32 data_unit_count;
	data_unit_count = (end_ptr - start_ptr) / data_size;
	data_unit_count /= 2;
		
	if((start_ptr < end_ptr))
	{
		/* (low + high) / 2 */
		/* but since these are pointers we could overflow with the addition */

		mid_ptr = start_ptr + (data_unit_count * data_size);  
		am_stata_merge_sort(merge_buffer, start_ptr, mid_ptr, data_size, sort_fn);
		am_stata_merge_sort(merge_buffer, mid_ptr + data_size, end_ptr, data_size, sort_fn);
		am_stata_merge(merge_buffer, start_ptr, end_ptr, mid_ptr, data_size, sort_fn);

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
			if(pSort->stata_integral.start_idx < fd->stata.array_size)
			{
				byte_ptr = (UINT8 *)fd->stata.data;
				byte_ptr += (pSort->stata_integral.start_idx * fd->stata.array_size);
				start_ptr = byte_ptr;
			}
			else
			{
				return AM_RET_PARAM_ERR;
			}
	
			if(pSort->stata_integral.end_idx < fd->stata.array_size)
			{
				/* Lazy man's way to sort the whole array */
				if(0 == pSort->stata_integral.end_idx)
				{
					byte_ptr = (UINT8 *)fd->stata.data;
					byte_ptr += ((fd->stata.array_size - 1) * fd->stata.data_size);		
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

AM_RETURN am_stata_release(AM_HANDLE handle, void * p1)
{

    AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);

	if(NULL != fd)
	{

        if(NULL != fd->flat.data)
        {
            AM_VFREE(fd->flat.data);
            fd->flat.data = NULL;
        }

        AM_FREE(fd);
    }
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


AM_RETURN am_create_stata_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap)
{
    AM_RETURN error = 0;
    AM_FUNC_DATA_U *pVdF = NULL;
 
	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);
	AM_ASSERT(pFunc->pfnOps);


    AM_DEBUGPRINT( "am_create_stata_device: maxSize=%lld\n", pCap->maxSize );

    pVdF = (AM_FUNC_DATA_U *)AM_MALLOC(sizeof(AM_FUNC_DATA_U));
    
    if(pVdF)
    {
		pVdF->stata.idx_size = pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE];
		pVdF->stata.data_size = pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE];
		pVdF->stata.size = pVdF->stata.data_size * pCap->maxSize;
		pVdF->stata.array_size = pCap->maxSize;
		
		pVdF->stata.data = AM_VALLOC((size_t)pVdF->stata.size);
        
		pFunc->crResp.data_size = pVdF->stata.data_size;
		pFunc->crResp.idx_size = pVdF->stata.idx_size;


		if(NULL != pVdF->stata.data)
		{


		    pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = (am_fn_op_only)am_targ_release;
			pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;
			
			pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = (am_fn_op_only)am_targ_open;
			pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

			pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = (am_fn_op_only)am_targ_close;
			pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;

			pFunc->pfnOps[AM_OP_SORT].action = am_stata_sort;
			pFunc->crResp.ops[AM_OP_SORT] = (AM_PACK_ACTION << 16) | AM_OP_SORT;

			
			pFunc->pfnOps[AM_OP_WRITE_ALIGN].align = am_stata_write_idx32;
			pFunc->pfnOps[AM_OP_READ_ALIGN].align = am_stata_read_idx32;
			 
			pFunc->crResp.acOps[ACOP_WRITE] = (AM_PACK_ALIGN << 16) | AM_OP_WRITE_ALIGN;
			pFunc->crResp.acOps[ACOP_READ] =  (AM_PACK_ALIGN << 16) | AM_OP_READ_ALIGN;

			pFunc->crResp.pack_DataOffset = pVdF->stata.idx_size;
			
			pFunc->pVdF = pVdF;
		
			
			sprintf(pFunc->crResp.am_name, "am_stata_%04x", pFunc->handle);
			AM_DEBUGPRINT("CREATE STATA = AM_NAME=%s\n", pFunc->crResp.am_name);		
			pFunc->crResp.devt = pFunc->handle;
		}
		else
		{
			error = AM_RET_ALLOC_ERR;
		}


	}
	else
	{
		error = AM_RET_ALLOC_ERR;
	}

	if(AM_RET_GOOD != error)
	{
		if(NULL != pVdF)
		{
			AM_FREE(pVdF);
		}
	}

	return error;
}


