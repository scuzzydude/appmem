#ifndef _AM_ASSCA_H
#define _AM_ASSCA_H
#include "uthash.h"

typedef struct _amlib_assca_item
{
	void *key;
	void *data;
	UT_hash_handle hh;

} AMLIB_ASSCA_ITEM;

#define MAX_ASSCA_ITERS (4)
#define ASSCA_ITER_FLAG_RESET 0x80000000
#define ASSCA_ITER_FLAG_NEW   0x40000000
#define ASSCA_ITER_FLAG_VALID 0x20000000  /* don't use zero iter handles */
#define ASSCA_ITER_DONE       0x10000000  
#define ASSCA_ITER_IDX_MASK   0x00000003
#define ASSCA_ITER_INVALID    0xF1FFFFFF
#define GET_ITER_IDX(_x) (_x & ASSCA_ITER_IDX_MASK)

typedef struct _amlib_assca
{
	UINT32 key_len;
	UINT32 data_len;
	UINT16 flags;
	UINT8  bFixedKey;
	UINT8  bFixedData;
	AMLIB_ASSCA_ITEM *pIterArray[MAX_ASSCA_ITERS];

	AMLIB_ASSCA_ITEM *head;
} AMLIB_ASSCA;

#define ASSCA_FLG_ERR_IF_KEY_EXITS            0x0001 /* No error - will delete previous entry and replace - typically array behavior */



AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData, UINT16 flags);
AM_RETURN amlib_assca_add_key_fixfix(AMLIB_ASSCA *pAA, void *pKey, void *pData);
AM_RETURN amlib_assca_get_key_val(AMLIB_ASSCA *pAA, void *pKey, void *pData);



AM_RETURN am_create_assca_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap);

AM_RETURN am_assca_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_release(AM_HANDLE handle, void * p1);

AM_RETURN am_assca_iter(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2, UINT32 *iter_handle);

/* 32 bit */
AM_RETURN am_assca_read32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_write32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_assca_read32_align(AM_HANDLE handle, void * p1, void *p2);
AM_RETURN am_assca_write32_align(AM_HANDLE handle, void * p1, void *p2);




#endif

