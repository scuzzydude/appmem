#include "appmemlib.h"
#include "am_assca.h"


AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData)
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

AM_RETURN amlib_assca_add_key_fixfix(AMLIB_ASSCA *pAA, void *pKey, void *pData)
{
	AMLIB_ASSCA_ITEM *pAI;

	AM_ASSERT(pAA);
	AM_ASSERT(pAA->bFixedKey == TRUE);
	AM_ASSERT(pAA->bFixedData == TRUE);
	AM_ASSERT(pKey);
	AM_ASSERT(pData);

	pAI = AM_MALLOC(sizeof(AMLIB_ASSCA_ITEM));

	if(pAI)
	{
		pAI->data = AM_MALLOC(pAA->data_len);
		pAI->key = AM_MALLOC(pAA->key_len);

		memcpy(pAI->data, pData, pAA->data_len);
		memcpy(pAI->key, pKey, pAA->key_len);
		
		HASH_ADD_KEYPTR( hh, pAA->head, pAI->key, pAA->key_len, pAI );

	}

	
	
	return AM_RET_GOOD;

}





