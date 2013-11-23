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
#include "am_assca.h"
#include "am_targ.h"


AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData, UINT16 flags)
{
	AMLIB_ASSCA *pAA = NULL;
	UINT32 i;

	pAA = (AMLIB_ASSCA *)AM_MALLOC(sizeof(AMLIB_ASSCA));

	if(pAA)
	{
		pAA->key_len = key_length;
		pAA->bFixedKey = bFixedKey;
		pAA->data_len = data_length;
		pAA->bFixedData = bFixedData;
		pAA->head = NULL;

		for(i = 0; i < MAX_ASSCA_ITERS; i++)
		{
			pAA->pIterArray[i] = NULL;
		}

	}


	return pAA;
}

AM_RETURN amlib_assca_get_key_val(AMLIB_ASSCA *pAA, void *pKey, void *pData)
{
	AMLIB_ASSCA_ITEM *pAI = NULL;

	AM_ASSERT(pAA);

	HASH_FIND(hh, pAA->head, pKey, pAA->key_len, pAI);

    if(pAI)
	{
        COPY_TO_USER(pData, pAI->data, pAA->data_len);
		
		return AM_RET_GOOD;
	}

	return AM_RET_KEY_NOT_FOUND;
}

/* This form takes a local copy (packet in kernel) so safe to use memcpy */

AM_RETURN amlib_assca_add_key_fixfix(AMLIB_ASSCA *pAA, void *pKey, void *pData)
{
	AMLIB_ASSCA_ITEM *pAI;

	AM_ASSERT(pAA);
	AM_ASSERT(pAA->bFixedKey == TRUE);
	AM_ASSERT(pAA->bFixedData == TRUE);
	AM_ASSERT(pKey);
	AM_ASSERT(pData);
	pAI = (AMLIB_ASSCA_ITEM *) AM_VALLOC( sizeof(AMLIB_ASSCA_ITEM) + pAA->key_len + pAA->data_len );

	if(pAI)
	{
		
		pAI->key = (UINT8 *)pAI + sizeof(AMLIB_ASSCA_ITEM);
		pAI->data = (UINT8 *)pAI->key + pAA->key_len;

        memcpy(pAI->data, pData, pAA->data_len);
		memcpy(pAI->key, pKey, pAA->key_len);


		HASH_ADD_KEYPTR( hh, pAA->head, pAI->key, pAA->key_len, pAI );

	}

	return AM_RET_GOOD;

}





AM_RETURN am_assca_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;
}

AM_RETURN am_assca_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;
}

AM_RETURN am_assca_release(AM_HANDLE handle, void * p1)
{

	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	AMLIB_ASSCA *pAA = NULL;
	AMLIB_ASSCA_ITEM *pAI = NULL;
	AMLIB_ASSCA_ITEM *pTmpAI = NULL;
    UINT32 count = 0;
    
    if(fd)
    {
        pAA = (AMLIB_ASSCA *) fd->assca.data;
		
        if(pAA)
        {

            HASH_ITER(hh, pAA->head, pAI, pTmpAI) 
            {
                if(pAI)
                {
                    AM_DEBUGPRINT("AI(iter=%d) %s %d\n", count, (char *)pAI->key, *(UINT32 *)pAI->data); 
                    count++;

                    HASH_DEL(pAA->head, pAI);  /* delete; users advances to next */

                    AM_VFREE(pAI);

                }

                

            }

        }


        AM_FREE(fd);
        

    }
	return AM_RET_GOOD;
}

UINT32 am_assca_get_iter(AMLIB_ASSCA *pAA, UINT32 iter_handle)
{
	UINT32 i;
	UINT32 local_iter = ASSCA_ITER_INVALID; 
	AMLIB_ASSCA_ITEM *pAIter = NULL;
	AMLIB_ASSCA_ITEM *pTmpAI = NULL;

	if(iter_handle & ASSCA_ITER_FLAG_NEW)
	{
		for(i = 0; i < MAX_ASSCA_ITERS; i++)
		{
			if(pAA->pIterArray[i] == NULL)
			{
				local_iter = (ASSCA_ITER_FLAG_VALID | i);
				break;
			}
		}

	}
	else if((iter_handle & ASSCA_ITER_FLAG_RESET) && ((iter_handle & ASSCA_ITER_IDX_MASK) < MAX_ASSCA_ITERS))
	{
		/* TODO: use the same handle but start it over */

	}
	else if((iter_handle & ASSCA_ITER_FLAG_VALID) && (iter_handle & ASSCA_ITER_IDX_MASK) < MAX_ASSCA_ITERS)
	{
		local_iter = iter_handle;
	}

	return local_iter;
}



AM_RETURN am_assca_iter(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2, UINT32 *iter_handle)
{
	
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	AMLIB_ASSCA *pAA = NULL;
	AMLIB_ASSCA_ITEM *pAI = NULL;
	UINT32 local_iter;
	UINT8 bHead = FALSE;
	

	if(fd)
	{
		pAA = (AMLIB_ASSCA *)fd->assca.data;

		if(*iter_handle & (ASSCA_ITER_FLAG_RESET | ASSCA_ITER_FLAG_NEW))
		{
			bHead = TRUE;
		}
		
		if(pAA)
		{
			local_iter = am_assca_get_iter(pAA, *iter_handle);
		}
		
		if(local_iter != ASSCA_ITER_INVALID)
		{
			if(local_iter != *iter_handle)
			{
				*iter_handle = local_iter;
			}
			
			
			if(bHead)
			{
				pAI = pAA->head;	
			}
			else
			{
				pAI = pAA->pIterArray[GET_ITER_IDX(local_iter)];
			}


			if(NULL != pAI)
			{

				pAA->pIterArray[GET_ITER_IDX(local_iter)] = (AMLIB_ASSCA_ITEM *)pAI->hh.next;

				COPY_TO_USER(p1, pAI->key, (UINT32)l1);
				COPY_TO_USER(p2, pAI->data, (UINT32)l2);
			}
			else
			{
				return AM_RET_ITER_DONE;
			}
		
		}
		else
		{
			*iter_handle = GET_ITER_IDX(local_iter) | ASSCA_ITER_DONE;  
			pAA->pIterArray[GET_ITER_IDX(local_iter)] = NULL;
			return AM_RET_INVALID_ITER;
		}
	
	
	}
		



	return AM_RET_GOOD;
}



/* 32 bit */
AM_RETURN am_assca_read32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;
}

AM_RETURN am_assca_write32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;
}


AM_RETURN am_assca_read32_align(AM_HANDLE handle, void * p1, void *p2)
{
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	AMLIB_ASSCA *pAA = NULL;
	if(fd)
	{
		pAA = (AMLIB_ASSCA *) fd->assca.data;
		AM_ASSERT(pAA);
		AM_ASSERT(p1);
		AM_ASSERT(p2);

		return amlib_assca_get_key_val(pAA, p1, p2);

	}

	return AM_RET_INVALID_HDL;
}

AM_RETURN am_assca_write32_align(AM_HANDLE handle, void * p1, void *p2)
{
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	AMLIB_ASSCA *pAA = NULL;

//    printk("assca_write32_align p1=%p p2=%p\n", p1, p2);
    
	if(fd)
	{
		pAA = (AMLIB_ASSCA *) fd->assca.data;
		AM_ASSERT(pAA);
		AM_ASSERT(p1);
		AM_ASSERT(p2);

		
		return amlib_assca_add_key_fixfix(pAA, p1, p2);

	}

	return AM_RET_INVALID_HDL;

}




AM_RETURN am_create_assca_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap)
{
	UINT8 bFixedKey = TRUE;
	UINT8 bFixedData = TRUE;
	AM_RETURN error = 0;
	AM_FUNC_DATA_U *pVdF = NULL;
	UINT32 key_size;
	UINT32 data_size;
	UINT16 flags = 0;
	AMLIB_ASSCA *pAA = (AMLIB_ASSCA *)NULL;

	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);
	AM_ASSERT(pFunc->pfnOps);


	AM_DEBUGPRINT( "am_create_assca_device: maxSize=%lld\n", pCap->maxSize );

	pVdF = (AM_FUNC_DATA_U *)AM_MALLOC(sizeof(AM_FUNC_DATA_U));

	if(pVdF && pCap && (AM_TYPE_ASSOC_ARRAY == pCap->amType))
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

		if(NULL != pAA)
		{

			pVdF->assca.size = pCap->maxSize;
			pVdF->assca.cur_count = 0;
			pVdF->assca.data = (void *)pAA;
			AM_DEBUGPRINT( "am_create_assca_device: pAA=%p\n", pAA);

			pFunc->crResp.data_size = data_size;
			pFunc->crResp.idx_size = key_size;


#if 0//def _APPMEMD
            pFunc->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config = (am_cmd_fn)am_assca_release;
#else

			pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = am_targ_release;
			pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;

			pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = am_targ_open;
			pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

			pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = am_targ_close;
			pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;

#endif

			if((TRUE == bFixedKey) && (TRUE == bFixedData))
			{

#if 0//def _APPMEMD
                pFunc->pfnOps[AM_OPCODE(AM_OP_CODE_READ_FIX_PACKET)].align  = am_assca_read32_align;
                pFunc->pfnOps[AM_OPCODE(AM_OP_CODE_WRITE_FIX_PACKET)].align  = am_assca_write32_align;

                pFunc->crResp.acOps[ACOP_WRITE] = AM_OP_CODE_WRITE_FIX_PACKET;
                pFunc->crResp.acOps[ACOP_READ] = AM_OP_CODE_READ_FIX_PACKET;
#else
				pFunc->pfnOps[AM_OP_READ_ALIGN].align  = am_assca_read32_align;
				pFunc->crResp.ops[AM_OP_READ_ALIGN] = (AM_KPACK << 16) | AM_OP_READ_ALIGN;

				pFunc->pfnOps[AM_OP_WRITE_ALIGN].align  = am_assca_write32_align;
				pFunc->crResp.ops[AM_OP_WRITE_ALIGN] = (AM_KPACK << 16) | AM_OP_WRITE_ALIGN;


				pFunc->crResp.acOps[ACOP_WRITE] = pFunc->crResp.ops[AM_OP_WRITE_ALIGN];
				pFunc->crResp.acOps[ACOP_READ]  = pFunc->crResp.ops[AM_OP_READ_ALIGN];
#endif

				/* To Make Sure that data segment is 32 bit aligned */
				pFunc->crResp.pack_DataOffset = pFunc->crResp.idx_size / sizeof(UINT32);


				if( pFunc->crResp.idx_size % sizeof(UINT32))
				{
					pFunc->crResp.pack_DataOffset++;    
				}

				AM_DEBUGPRINT("PACKET DATA IDX SIZE=%d\n", pFunc->crResp.idx_size);
				AM_DEBUGPRINT("PACKET DATA DATA SIZE=%d\n", pFunc->crResp.data_size);
				AM_DEBUGPRINT("PACKET DATA DATA OFFSET DWORDS=%d\n", pFunc->crResp.pack_DataOffset);


				pFunc->crResp.wr_pack_qword_size = pFunc->crResp.pack_DataOffset;

				pFunc->crResp.wr_pack_qword_size += (pFunc->crResp.data_size / sizeof(UINT32));

				if(pFunc->crResp.data_size % sizeof(UINT32))
				{
					pFunc->crResp.wr_pack_qword_size++;
				}

				pFunc->crResp.wr_pack_qword_size++; /* Always need one for opcode */   

				AM_DEBUGPRINT("WR PACKET SIZE DWORDS=%d\n", pFunc->crResp.wr_pack_qword_size);

				if(pFunc->crResp.wr_pack_qword_size % 2)
				{
					pFunc->crResp.wr_pack_qword_size = 1 + (pFunc->crResp.wr_pack_qword_size / 2);
				}
				else
				{
					pFunc->crResp.wr_pack_qword_size = (pFunc->crResp.wr_pack_qword_size / 2);
				}


				AM_DEBUGPRINT("WR PACKET SIZE QWORDS=%d\n", pFunc->crResp.wr_pack_qword_size);

				pFunc->crResp.rd_pack_qword_size = pFunc->crResp.pack_DataOffset;                    

				pFunc->crResp.rd_pack_qword_size++; //read user data pointer - two dwords 

				pFunc->crResp.rd_pack_qword_size++; //read user data pointer - two dwords 
				pFunc->crResp.rd_pack_qword_size++; //read user data pointer - two dwords 


				AM_DEBUGPRINT("RD PACKET SIZE DWORDS=%d\n", pFunc->crResp.rd_pack_qword_size);

				if(pFunc->crResp.rd_pack_qword_size % 2)
				{

					pFunc->crResp.rd_pack_qword_size = 1 + (pFunc->crResp.rd_pack_qword_size / 2);
				}
				else
				{
					pFunc->crResp.rd_pack_qword_size = (pFunc->crResp.rd_pack_qword_size / 2);

				}

				AM_DEBUGPRINT("RD PACKET SIZE QWORDS=%d\n", pFunc->crResp.rd_pack_qword_size);
			//	AM_DEBUGPRINT("WT PACKET SIZE BYTES=%d\n", pDevice->wr_pack_size);
			//	AM_DEBUGPRINT("RD PACKET SIZE BYTES=%d\n", pDevice->rd_pack_size);


			}

			pFunc->pVdF = pVdF;
			sprintf(pFunc->crResp.am_name, "am_assca_%04x", pFunc->handle);
			AM_DEBUGPRINT("CREATE ASSCA = AM_NAME=%s\n", pFunc->crResp.am_name);		
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


