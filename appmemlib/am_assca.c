#include "appmemlib.h"
#include "am_assca.h"


AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData)
{
	AMLIB_ASSCA *pAA = NULL;

	pAA = (AMLIB_ASSCA *)malloc(sizeof(AMLIB_ASSCA));

	if(pAA)
	{
		pAA->key_len = key_length;
		pAA->bFixedKey = bFixedKey;
		pAA->data_len = data_length;
		pAA->bFixedData = bFixedData;
	
	}


	return pAA;
}







