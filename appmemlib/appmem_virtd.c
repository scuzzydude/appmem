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
#include <stdio.h>
#include <stdlib.h>
#include "appmemlib.h"
#include "appmem_virtd.h"
#include "am_flat.h"
#include "am_stata.h"
#include "am_assca.h"


#define MAX_VIRTD_FUNC_HANDLES 8
#define VIRTD_HANDLE_SIG          (0xBADBA00)
#define VIRTD_HANDLE_IDX_MASK     (0x00000FF)

AM_FUNC_DATA_U *g_ptrFuncHandles[MAX_VIRTD_FUNC_HANDLES] = { NULL };

AM_MEM_CAP_T virtd_caps[] = 
{

	{ 
		AM_TYPE_BASE_APPMEM,
		(1024 * 1024 * 128), /* Total Device Memory */
		2,
		{
			0, 0
		}
	},
	{ 
		AM_TYPE_FLAT_MEM, /* Type */ 
		(1024 * 1024),    /* maxSizeBytes */
		7,    			  /* maxFunction  */	
		{
			(8 | 4 | 2 | 1),  /* Mem - Address Size */ 
			1,                /* Mem - Min Byte action */
			(1024 * 1024)     /* Mem - Max Byte action */
		}
	
	},
	{ 
		AM_TYPE_ARRAY,
		(1024 * 1024),
		7,
		{
			(8 | 4 | 2 | 1),  /* Array - Index Size Bytes */ 
			1,                /* Min Value Size - Bytes */
			1024,              /* Max Value Size - Bytes */
			TS_STAT_DT_FIXED_WIDTH
		}

	},
	{
		AM_TYPE_ASSOC_ARRAY,
		(1024 * 1024),
		7,
		{
			512,              /* Max - Key Size Bytes */ 
			1024 * 1024,      /* Max - Data Size Bytes */
			TS_ASSCA_KEY_FIXED_WIDTH | TS_ASSCA_KEY_VAR_WIDTH,
			TS_ASSCA_DATA_FIXED_WIDTH | TS_ASSCA_DATA_VAR_WIDTH
		}



	}

};

AM_RETURN am_virtd_get_capabilites_count(AMLIB_ENTRY_T *pEntry)
{
	UINT32 count = 0;

	count = sizeof(virtd_caps) / sizeof(AM_MEM_CAP_T);

	return count;
}

AM_RETURN am_virtd_get_capabilities(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count)
{
	AM_RETURN error = AM_RET_GOOD;
	
	AM_ASSERT(pAmCaps);
	AM_ASSERT(count);

	if(count <= (sizeof(virtd_caps) / sizeof(AM_MEM_CAP_T)))
	{
		memcpy(pAmCaps, (void *)virtd_caps, sizeof(AM_MEM_CAP_T) * count);
	
	}
	else
	{
		error = AM_RET_PARAM_ERR; 
	}


	return error;

}

AM_RETURN am_virtd_init_assca(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc, AM_FUNC_DATA_U *pVdF)
{
	UINT8 bFixedKey = TRUE;
	UINT8 bFixedData = TRUE;
	UINT16 flags = 0;
	UINT32 key_size;
	UINT32 data_size;
	AM_RETURN error = AM_RET_GOOD;
	AMLIB_ASSCA *pAA = (AMLIB_ASSCA *)NULL;
	AM_FUNC_CALLS_T *fn_array =  NULL;
	if(pFunc->fn)
	{
		fn_array = pFunc->fn;
	}

	if(fn_array && pVdF && pFunc && pCap && (AM_TYPE_ASSOC_ARRAY == pCap->amType))
	{
		key_size = pCap->typeSpecific[TS_ASSCA_KEY_MAX_SIZE];
		data_size =	pCap->typeSpecific[TS_ASSCA_DATA_MAX_SIZE];
		
		if(pCap->typeSpecific[TS_ASSCA_KEY_TYPE] & TS_ASSCA_KEY_FIXED_WIDTH)
		{
			bFixedKey = TRUE;
		}
		if(pCap->typeSpecific[TS_ASSCA_DATA_TYPE] & TS_ASSCA_DATA_FIXED_WIDTH)
		{
			bFixedData = TRUE;
		}
		

		pAA = amlib_assca_init(key_size, data_size, bFixedKey, bFixedData, flags);

		if(pAA)
		{
			pVdF->assca.size = pCap->maxSize;
			pVdF->assca.cur_count = 0;
			pVdF->flat.data = (void *)pAA;
	
			fn_array->open = am_virtd_open;
			fn_array->close = am_virtd_close;
			fn_array->read = am_assca_read32;
			fn_array->write = am_assca_write32;
			fn_array->read_al = am_assca_read32_align;
			fn_array->write_al = am_assca_write32_align;
			fn_array->copy = NULL;
	

		}
		

	}
	else
	{
		error = AM_RET_PARAM_ERR;
	}


	return error;



}

AM_RETURN am_virtd_create_function(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc)
{
	AM_RETURN error = AM_RET_GOOD;
	UINT32 i;
	AM_FUNC_DATA_U *pVdF = NULL;
	AM_FUNC_CALLS_T *fn_array =  NULL;

	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);

	/* TODO : Validate the Capactiy the client passed fit the actual capabilities of the device */
	
	memcpy((void *)&pFunc->amCap, (void *)pCap, sizeof(AM_MEM_CAP_T));

	for(i = 0; i < MAX_VIRTD_FUNC_HANDLES; i++)
	{
		if(NULL == g_ptrFuncHandles[i])
		{
			pVdF = (AM_FUNC_DATA_U *)AM_MALLOC(sizeof(AM_FUNC_DATA_U));
			if(pVdF)
			{
				g_ptrFuncHandles[i] = pVdF;
				
				pFunc->handle = (VIRTD_HANDLE_SIG | i);
			
			}
			break;

		
		}

	}

	if(NULL != pFunc)
	{
		switch(pCap->amType)
		{
			case AM_TYPE_FLAT_MEM:
			{
				pVdF->flat.size = pCap->maxSize;
				pVdF->flat.add_size = pCap->typeSpecific[TS_FLAT_ADDRESS_BYTE_SIZE];
				pVdF->flat.data = AM_MALLOC((size_t)pCap->maxSize);
				
				if(NULL != pVdF->flat.data)
				{
					if(pCap->typeSpecific[TS_INIT_MEM_VAL])
					{
						/* Loop, DEADBEEF or whatever 32 bit pattern user demands */
					}
					else
					{
						memset(pVdF->flat.data, 0, (size_t) pVdF->flat.size);
					}
		
					 fn_array = pFunc->fn;

					if(fn_array)
					{

						fn_array->open = am_virtd_open;
						fn_array->close = am_virtd_close;
						fn_array->read = am_flat_read32;
						fn_array->write = am_flat_write32;
						fn_array->read_al = am_flat_read32_align;
						fn_array->write_al = am_flat_write32_align;
						fn_array->copy = NULL;
						
					}



				}
				else
				{
					error = AM_RET_ALLOC_ERR; 
				}
			
			}	
			break;

			case AM_TYPE_ARRAY:
			{
	
				pVdF->stata.idx_size = pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE];
				pVdF->stata.data_size = pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE];
				pVdF->stata.size = pVdF->stata.data_size * pCap->maxSize;
				
				pVdF->stata.data = AM_MALLOC((size_t)pVdF->stata.size);

				if(NULL != pVdF->stata.data)
				{
					memset(pVdF->stata.data, 0, (size_t)pVdF->stata.size);
					fn_array = pFunc->fn;

					if(fn_array)
					{
						fn_array->open = am_virtd_open;
						fn_array->close = am_virtd_close;
						fn_array->read = NULL;
						fn_array->write = NULL;
						fn_array->read_al = am_stata_read_idx32;
						fn_array->write_al = am_stata_write_idx32;
						fn_array->copy = NULL;
					}

				}
				else
				{
					error = AM_RET_ALLOC_ERR; 
				}
			
			}
			break;

			case AM_TYPE_ASSOC_ARRAY:
			{
				error = am_virtd_init_assca(pEntry, pCap, pFunc, pVdF);
			}
			break;


			default:
			{
				error = AM_RET_INVALID_FUNC;
			}
			break;

		
		
		
		}
	
	
	
	
	}
	else
	{
		error = AM_RET_PARAM_ERR; 
	}



	return error;

}

AM_FUNC_DATA_U * am_handle_to_funcdata(UINT32 handle)
{
	AM_ASSERT(VIRTD_HANDLE_SIG == (~VIRTD_HANDLE_IDX_MASK & handle));
	AM_ASSERT(((VIRTD_HANDLE_IDX_MASK & handle) < MAX_VIRTD_FUNC_HANDLES));

	return g_ptrFuncHandles[(VIRTD_HANDLE_IDX_MASK & handle)];
}

AM_RETURN am_virtd_open(void * p1)
{
	AM_MEM_FUNCTION_T *pFunc = p1;
	if(pFunc->handle)
	{
		return AM_RET_GOOD;
	}
	else
	{
		return AM_RET_INVALID_HDL;
	}
}
AM_RETURN am_virtd_close(void * p1)
{
	AM_MEM_FUNCTION_T *pFunc = p1;

	if(pFunc->handle)
	{
		return AM_RET_GOOD;
	}
	else
	{
		return AM_RET_INVALID_HDL;
	}


}
