#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_assca.h"
#include "am_targ.h"


AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData, UINT16 flags)
{
	AMLIB_ASSCA *pAA = NULL;

	pAA = (AMLIB_ASSCA *)AM_MALLOC(sizeof(AMLIB_ASSCA));

	if(pAA)
	{
		pAA->key_len = key_length;
		pAA->bFixedKey = bFixedKey;
		pAA->data_len = data_length;
		pAA->bFixedData = bFixedData;
		pAA->head = NULL;

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
//		memcpy(pData, pAI->data, pAA->data_len);		
        COPY_TO_USER(pData, pAI->data, pAA->data_len);
        
		
		return AM_RET_GOOD;
	}

	return AM_RET_KEY_NOT_FOUND;
}

/* This form takes a local copy (packet in kernel) so safe to use memcpy */

AM_RETURN amlib_assca_add_key_fixfix(AMLIB_ASSCA *pAA, void *pKey, void *pData)
{
	char tempkey[64];
	AMLIB_ASSCA_ITEM *pAI;

	AM_ASSERT(pAA);
	AM_ASSERT(pAA->bFixedKey == TRUE);
	AM_ASSERT(pAA->bFixedData == TRUE);
	AM_ASSERT(pKey);
	AM_ASSERT(pData);
	pAI = (AMLIB_ASSCA_ITEM *) AM_MALLOC( sizeof(AMLIB_ASSCA_ITEM) + pAA->key_len + pAA->data_len );

	if(pAI)
	{
		pAI->data = AM_MALLOC(pAA->data_len);
		pAI->key = AM_MALLOC(pAA->key_len);

		

		pAI->key = (UINT8 *)pAI + sizeof(AMLIB_ASSCA_ITEM);
		pAI->data = (UINT8 *)pAI->key + pAA->key_len;

		//COPY_FROM_USER(pAI->data, pData, pAA->data_len);
		//COPY_FROM_USER(pAI->key, pKey, pAA->key_len);

        memcpy(pAI->data, pData, pAA->data_len);
		memcpy(pAI->key, pKey, pAA->key_len);

		memcpy(&tempkey[0], pKey, pAA->key_len);

		tempkey[pAA->key_len] = 0;

		printf("ADD KEY_LEN=%d KEY = %s DATA = %d\n", pAA->key_len, tempkey, pAI->data);

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

			pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = am_targ_release;
			pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;

			pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = am_targ_open;
			pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

			pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = am_targ_close;
			pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;

			pFunc->crResp.data_size = data_size;
			pFunc->crResp.idx_size = key_size;


			if((TRUE == bFixedKey) && (TRUE == bFixedData))
			{


				pFunc->pfnOps[AM_OP_READ_ALIGN].align  = am_assca_read32_align;
				pFunc->crResp.ops[AM_OP_READ_ALIGN] = (AM_PACK_ALIGN << 16) | AM_OP_READ_ALIGN;

				pFunc->pfnOps[AM_OP_WRITE_ALIGN].align  = am_assca_write32_align;
				pFunc->crResp.ops[AM_OP_WRITE_ALIGN] = (AM_PACK_ALIGN << 16) | AM_OP_WRITE_ALIGN;


				pFunc->crResp.acOps[ACOP_WRITE] = pFunc->crResp.ops[AM_OP_WRITE_ALIGN];
				pFunc->crResp.acOps[ACOP_READ]  = pFunc->crResp.ops[AM_OP_READ_ALIGN];


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


				//pDevice->pack_DataOffset = respCr.pack_DataOffset;     

				//pDevice->wr_pack_size = respCr.wr_pack_qword_size * 8;
				//pDevice->rd_pack_size = respCr.rd_pack_qword_size * 8;

				//printk("WT PACKET SIZE BYTES=%d\n", pDevice->wr_pack_size);
				//printk("RD PACKET SIZE BYTES=%d\n", pDevice->rd_pack_size);


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


