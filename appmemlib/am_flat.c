
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_flat.h"
#include "am_targ.h"

AM_RETURN am_flat_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2) 
{
	return AM_RET_GOOD;

}
AM_RETURN am_flat_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;
}


/* 32 bit */
AM_RETURN am_flat_read32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;

}
AM_RETURN am_flat_write32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;
}

AM_RETURN am_flat_read32_align(AM_HANDLE handle, void * p1, void *p2)
{
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 offset = *(UINT32 *) p1;

	PUT32_TO_USER(((UINT64)fd->flat.data + (UINT64)offset), p2);

	return AM_RET_GOOD;

}


AM_RETURN am_flat_write32_align(AM_HANDLE handle, void * p1, void *p2)
{
	
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 offset = *(UINT32 *) p1;
	UINT32 *ptr = (UINT32 *)((UINT64)fd->flat.data + (UINT64)offset);

	GET32_FROM_USER(ptr, p2);

	return AM_RET_GOOD;
}





/* TODO: Use this function in both appmemk and virtd */
AM_RETURN am_create_flat_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap)
{
    AM_RETURN error = 0;
    AM_FUNC_DATA_U *pVdF = NULL;
 
	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);
	AM_ASSERT(pFunc->pfnOps);


    AM_DEBUGPRINT( "am_targ_create_flat_device: maxSize=%lld\n", pCap->maxSize );

    pVdF = (AM_FUNC_DATA_U *)AM_MALLOC(sizeof(AM_FUNC_DATA_U));
    
    if(pVdF)
    {
		pVdF->flat.size = pCap->maxSize;
		pVdF->flat.add_size = pCap->typeSpecific[TS_FLAT_ADDRESS_BYTE_SIZE];
		pVdF->flat.data = AM_MALLOC((size_t)pCap->maxSize); /* VMalloc for large virtual buffer */
				
 
		if(NULL != pVdF->flat.data)
		{
		    pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = am_targ_release;
			pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;
			
			pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = am_targ_open;
			pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

			pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = am_targ_close;
			pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;

			if(pCap->typeSpecific[TS_INIT_MEM_VAL])
			{
				/* TODO --- */            
			}
			else
			{
			    memset(pVdF->flat.data, 0, (size_t) pVdF->flat.size);
			}

			
			pFunc->pfnOps[AM_OP_WRITE_ALIGN].align = am_flat_write32_align;
			pFunc->pfnOps[AM_OP_READ_ALIGN].align = am_flat_read32_align;
			 
			pFunc->crResp.acOps[ACOP_WRITE] = (AM_PACK_ALIGN << 16) | AM_OP_WRITE_ALIGN;
			pFunc->crResp.acOps[ACOP_READ] =  (AM_PACK_ALIGN << 16) | AM_OP_READ_ALIGN;

			pFunc->crResp.data_size = pVdF->flat.add_size;
			pFunc->crResp.pack_DataOffset = pVdF->flat.add_size;
			
			pFunc->pVdF = pVdF;
		
			
			sprintf(pFunc->crResp.am_name, "am_flat_%04x", pFunc->handle);
			AM_DEBUGPRINT("CREATE FLAT = AM_NAME=%s\n", pFunc->crResp.am_name);		
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



