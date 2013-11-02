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

UINT32 am_virtd_get_capabilites_count(AMLIB_ENTRY_T *pEntry)
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
	UINT8 bFixedKey = FALSE;
	UINT8 bFixedData = FALSE;
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
			bFixedKey = FALSE;
		}
		

		pAA = amlib_assca_init(key_size, data_size, bFixedKey, bFixedData, flags);

		if(pAA)
		{
			pVdF->assca.size = pCap->maxSize;
			pVdF->assca.cur_count = 0;
			pVdF->flat.data = (void *)pAA;
	
			fn_array->open = am_assca_open;
			fn_array->close = am_assca_close;
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

						fn_array->open = am_flat_open;
						fn_array->close = am_flat_close;
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
						fn_array->open = am_stata_open;
						fn_array->close = am_stata_close;
						fn_array->read = NULL;
						fn_array->write = NULL;
						fn_array->read_al = am_stata_read_idx32;
						fn_array->write_al = am_flat_write_idx32;
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

