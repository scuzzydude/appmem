#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_assca.h"


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


