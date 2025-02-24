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
#include "appmemd_ioctl.h"
#include "am_flat.h"
#include "am_targ.h"

AM_RETURN am_flat_open(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2) 
{
	return AM_RET_GOOD;

}
AM_RETURN am_flat_close(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;
}

AM_RETURN am_flat_release(AM_FUNC *pFunc, void * p1)
{

	AM_FUNC_DATA_U * fd = pFunc->pVdF;

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
AM_RETURN am_flat_read32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;

}
AM_RETURN am_flat_write32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;
}

AM_RETURN am_flat_read32_align(AM_FUNC *pFunc, void * p1, void *p2)
{
	AM_FUNC_DATA_U * fd = pFunc->pVdF;
	UINT32 offset = *(UINT32 *) p1;

	PUT32_TO_USER(fd, ((UINT64)fd->flat.data + (UINT64)offset), p2);
#ifdef _APPMEMD
	printk("Type=%08x Read32 offset = 0x%08x data 0x%08x\n", fd->common.flags, offset, *(UINT32 *)((UINT64)fd->flat.data + (UINT64)offset));
#endif
	return AM_RET_GOOD;

}


AM_RETURN am_flat_write32_align(AM_FUNC *pFunc, void * p1, void *p2)
{

	AM_FUNC_DATA_U * fd = pFunc->pVdF;
	UINT32 offset = *(UINT32 *) p1;
	UINT32 *ptr = (UINT32 *)((UINT64)fd->flat.data + (UINT64)offset);

	GET32_FROM_USER(fd, ptr, p2);

#ifdef _APPMEMD
	printk("Type=%08x Write32 offset = 0x%08x data 0x%08x\n", fd->common.flags, offset, *(UINT32 *)p2);
#endif

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
		pVdF->flat.data = AM_VALLOC((size_t)pCap->maxSize); /* VMalloc for large virtual buffer */

		AM_DEBUGPRINT( "am_targ_create_flat_device: pVdF=%p\n", pVdF);

		if(NULL != pVdF->flat.data)
		{

			AM_DEBUGPRINT( "am_targ_create_flat_device: pVdF->flat.data=%p\n", pVdF->flat.data);

			if(pCap->typeSpecific[TS_INIT_MEM_VAL])
			{
				/* TODO --- */            
			}
			else
			{
				memset(pVdF->flat.data, 0, (size_t) pVdF->flat.size);
			}


#if 0//def _APPMEMD

			pFunc->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config  = (am_cmd_fn)am_flat_release;

			pFunc->pfnOps[AM_OPCODE(AM_OP_CODE_READ_ALIGN)].align  = am_flat_read32_align;
			pFunc->pfnOps[AM_OPCODE(AM_OP_CODE_WRITE_ALIGN)].align  = am_flat_write32_align;


			pFunc->crResp.acOps[ACOP_WRITE] = AM_OP_CODE_WRITE_ALIGN;
			pFunc->crResp.acOps[ACOP_READ] = AM_OP_CODE_READ_ALIGN;


#else			

			pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = (am_fn_op_only)am_targ_release;
			pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;

			pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = (am_fn_op_only)am_targ_open;
			pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

			pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = (am_fn_op_only)am_targ_close;
			pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;


			pFunc->pfnOps[AM_OP_WRITE_ALIGN].align = am_flat_write32_align;
			pFunc->pfnOps[AM_OP_READ_ALIGN].align = am_flat_read32_align;

			pFunc->crResp.acOps[ACOP_WRITE] = (AM_PACK_ALIGN << 16) | AM_OP_WRITE_ALIGN;
			pFunc->crResp.acOps[ACOP_READ] =  (AM_PACK_ALIGN << 16) | AM_OP_READ_ALIGN;
#endif
			pFunc->crResp.idx_size = pVdF->flat.add_size;
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


//*************************************************************************************************
//**   Register this Device Function 
//*************************************************************************************************

static AM_MEM_CAP_T flat_caps =
{ 
	AM_TYPE_FLAT_MEM, /* Type */ 
		(1024 * 1024),    /* maxSizeBytes */
		1,                   /* subType */
		7,/* maxFunction  */	
		0,
	{
		(8 | 4 | 2 | 1),  /* Mem - Address Size */ 
			1,                /* Mem - Min Byte action */
			(1024 * 1024)     /* Mem - Max Byte action */
	}

};


AM_CAP_DETAILS flat_cap_details[AM_FLAT_SUBTYPES] = 
{
 //Descriptors are 64 bytes 
	
	//             1         2         3         4         5         6
	//   01234567890123456789012345678901234567890123456789012345678901234
	{
		AM_TYPE_FLAT_MEM,
		"Flat Memory",
		0,
		"Flat Memory - Native C",
		TS_FLAT_MAX_TS,
		{
		"Address Size in Bytes (1,2,4,8)",
		"Min Data Addressability - Bytes",
		"32 Bit Initializer - (memset)"
		}
	}
};



static AM_FUNCTION_ENTRY flat_function_entry =
{
	AM_TYPE_FLAT_MEM,
	am_create_flat_device,
	&flat_caps,
	&flat_cap_details[0]

};


INITIALIZER(flat_init)
{
	am_register_am_function(&flat_function_entry);
}


