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
#include "am_caps.h"
#include "am_targ.h"

int g_virtd_minor_count = 1;

//******************************* Module Registration ***********************************/
#define MAX_VIRTD_DEVICE_FUNCTIONS 100
AM_FUNCTION_ENTRY *gVirtdDeviceFunctionEntry[MAX_VIRTD_DEVICE_FUNCTIONS] = { 0 };

AM_RETURN am_register_am_function(AM_FUNCTION_ENTRY *pFunctionEntry)
{
	printf("am_register_am_function amType=%d\n", pFunctionEntry->amType);
	if(NULL != pFunctionEntry)
    {
		if(( pFunctionEntry->amType < MAX_VIRTD_DEVICE_FUNCTIONS) && 
		    (NULL == gVirtdDeviceFunctionEntry[pFunctionEntry->amType]))
		{
			gVirtdDeviceFunctionEntry[pFunctionEntry->amType] = pFunctionEntry;
		}
		else
		{
			AM_DEBUGPRINT("Device Function Already Registered\n");
		}
	}
	return AM_RET_GOOD;
}
//******************************* Module Registration End ***********************************/

AM_RETURN am_virtd_entry_close(AMLIB_ENTRY_T *pEntry)
{
	return AM_RET_GOOD;
}

UINT32 am_virtd_get_capabilites_count(AMLIB_ENTRY_T *pEntry)
{
	return virtd_base_cap.subType;
}

AM_RETURN am_virtd_get_capabilities(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count)
{
	AM_RETURN error = AM_RET_GOOD;
	UINT32 i;

	AM_ASSERT(pAmCaps);
	AM_ASSERT(count);

	for(i = 0; i < count; i++)
	{
		if(i < virtd_base_cap.subType)
		{
			if(0 == i)
			{
				memcpy(pAmCaps, (void *)&virtd_base_cap, sizeof(AM_MEM_CAP_T));
				pAmCaps++;
			}
			else
			{
				if(NULL != gVirtdDeviceFunctionEntry[i])
				{
					if(NULL != gVirtdDeviceFunctionEntry[i]->pCap)
					{
						memcpy(pAmCaps, (void *)gVirtdDeviceFunctionEntry[i]->pCap, sizeof(AM_MEM_CAP_T));
						pAmCaps++;	
					}
				}
			}
		
		}
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
	        pAA->fd = pVdF;    

		    AM_ASSERT(pVdF);
		    
			fn_array->release = am_assca_release;
			fn_array->open = am_virtd_open;
			fn_array->close = am_virtd_close;
			fn_array->read = am_assca_read32;
			fn_array->write = am_assca_write32;
			fn_array->read_al = am_assca_read32_align;
			fn_array->write_al = am_assca_write32_align;
			fn_array->iter = am_assca_iter;
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
	AM_FUNC_DATA_U *pVdF = NULL;
	AM_FUNC_CALLS_T *fn_array =  NULL;

	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);

	/* TODO : Validate the Capactiy the client passed fit the actual capabilities of the device */
	
	memcpy((void *)&pFunc->amCap, (void *)pCap, sizeof(AM_MEM_CAP_T));

	pVdF = (AM_FUNC_DATA_U *)AM_MALLOC(sizeof(AM_FUNC_DATA_U));

	if((NULL != pVdF) && (NULL != pFunc))
	{
		pFunc->pVdF = pVdF;

		switch(pCap->amType)
		{
			case AM_TYPE_FLAT_MEM:
			{
				sprintf(pFunc->crResp.am_name, "virtd_flat%d", g_virtd_minor_count);

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

						fn_array->release = am_flat_release;
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

			case AM_TYPE_STATIC_ARRAY:
			{
	
				sprintf(pFunc->crResp.am_name, "virtd_stata%d", g_virtd_minor_count);

				pVdF->stata.idx_size = pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE];
				pVdF->stata.data_size = pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE];
				pVdF->stata.size = pVdF->stata.data_size * pCap->maxSize;
				pVdF->stata.array_size = (UINT32) pCap->maxSize; //cast.. problematic over 4GB (indexes) size arrays 

				pVdF->stata.data = AM_MALLOC((size_t)pVdF->stata.size);

				if(NULL != pVdF->stata.data)
				{
					memset(pVdF->stata.data, 0, (size_t)pVdF->stata.size);
					fn_array = pFunc->fn;

					if(fn_array)
					{
						fn_array->release = am_stata_release;
						fn_array->open = am_virtd_open;
						fn_array->close = am_virtd_close;
						fn_array->read = NULL;
						fn_array->write = NULL;
						fn_array->read_al = am_stata_read_idx32;
						fn_array->write_al = am_stata_write_idx32;
						fn_array->copy = NULL;
						fn_array->sort = am_stata_sort;
						
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
				sprintf(pFunc->crResp.am_name, "virtd_assca%d", g_virtd_minor_count);
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


	g_virtd_minor_count++;

	return error;

}


/* TODO: For Target code consolidation, rectify  with virtd functions later */
AM_RETURN am_targ_release(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;
		
}
AM_RETURN am_targ_open(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;

}
AM_RETURN am_targ_close(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;
}


AM_RETURN am_virt_get_cap_details(struct amlib_entry_ *pEntry, AM_CAP_DETAILS *pCapDetails, UINT32 amType)
{
	UINT32 st_count = 1;

	if( amType < MAX_VIRTD_DEVICE_FUNCTIONS)
	{
		if(NULL != gVirtdDeviceFunctionEntry[amType])
		{

			if(NULL != gVirtdDeviceFunctionEntry[amType]->pCapDetails)
			{
				if(AM_TYPE_BASE_APPMEM == amType)
				{
					st_count = 1;
				}
				else
				{
					st_count = gVirtdDeviceFunctionEntry[amType]->pCap->subType;
				}
				memcpy(pCapDetails, gVirtdDeviceFunctionEntry[amType]->pCapDetails, st_count * sizeof(AM_CAP_DETAILS));
				
				return AM_RET_GOOD;
			}
		
		}

	}


	return AM_RET_INVALID_OPCODE;

}
	


AM_RETURN am_virtd_open(void * p1)
{
	//AM_MEM_FUNCTION_T *pFunc = p1;
	/* TODO - VIRTD Persistances (i.e. file based) */
	return AM_RET_GOOD;
}
AM_RETURN am_virtd_close(void * p1)
{
	//AM_MEM_FUNCTION_T *pFunc = p1;
	/* TODO - VIRTD Persistances (i.e. file based) */
	return AM_RET_GOOD;
	

}


//******************************************************
//**  Register Base Virtd Device 
//******************************************************

INITIALIZER(am_base_init)
{
	am_register_am_function(&am_base_function_entry);
}


