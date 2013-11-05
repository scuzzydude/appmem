#ifndef _AM_ASSCA_H
#define _AM_ASSCA_H
#include "uthash.h"

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
	UINT16 flags;
	UINT8  bFixedKey;
	UINT8  bFixedData;

	AMLIB_ASSCA_ITEM *head;
} AMLIB_ASSCA;

#define ASSCA_FLG_ERR_IF_KEY_EXITS            0x0001 /* No error - will delete previous entry and replace - typically array behavior */



AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData, UINT16 flags);
AM_RETURN amlib_assca_add_key_fixfix(AMLIB_ASSCA *pAA, void *pKey, void *pData);
AM_RETURN amlib_assca_get_key_val(AMLIB_ASSCA *pAA, void *pKey, void *pData);



AM_RETURN am_assca_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);


/* 32 bit */
AM_RETURN am_assca_read32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_write32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_assca_read32_align(AM_HANDLE handle, void * p1, void *p2);
AM_RETURN am_assca_write32_align(AM_HANDLE handle, void * p1, void *p2);




#endif

