#ifndef _AM_ASSCA_H
#define _AM_ASSCA_H
#include "ut-hash\uthash.h"

typedef struct _amlib_assca_item
{
	void *key;
	void *data;
	UT_hash_handle hh;

} AMLIB_ASSCA_ITEM;


typedef struct _amlib_assca
{
	UINT32 key_len;
	UINT32 data_len;
	UINT8   bFixedKey;
	UINT8   bFixedData;
	AMLIB_ASSCA_ITEM *head;
} AMLIB_ASSCA;

AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData);



#endif

