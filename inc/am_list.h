#ifndef _AM_LIST_H
#define _AM_LIST_H
#include "utlist.h"


#define AM_LIST_SUBTYPES 3

#define AM_LIST_SLL 0 
#define AM_LIST_DLL 1
#define AM_LIST_CLL 2

#define TS_LIST_DATA_MAX_SIZE        0 
#define TS_LIST_DATA_TYPE            1
#define TS_LIST_DATA_FIXED        0x1
#define TS_LIST_DATA_VAR          0x2


#define TS_LIST_MAX_TS              2



typedef struct _amlib_list_item
{
	void                    *data;
	UINT32                   data_size;
	struct _amlib_list_item *next;
	struct _amlib_list_item *prev;
	//UT_hash_handle hh;

} AMLIB_LIST_ITEM;

typedef struct _amlib_list
{
    UINT32 list_type;
    UINT32 elem_count;
    UINT32 data_type;
    UINT32 data_size;
    
	AMLIB_LIST_ITEM *head;

} AMLIB_LIST;



#if 0

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
    AM_FUNC_DATA_U * fd;
	AMLIB_ASSCA_ITEM *head;
} AMLIB_ASSCA;

#define ASSCA_FLG_ERR_IF_KEY_EXITS            0x0001 /* No error - will delete previous entry and replace - typically array behavior */



AMLIB_ASSCA * amlib_assca_init(UINT32 key_length, UINT32 data_length, UINT8 bFixedKey, UINT8 bFixedData, UINT16 flags);
AM_RETURN amlib_assca_add_key_fixfix(AMLIB_ASSCA *pAA, void *pKey, void *pData);
AM_RETURN amlib_assca_get_key_val(AMLIB_ASSCA *pAA, void *pKey, void *pData);




AM_RETURN am_assca_open(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_close(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_release(AM_FUNC *pFunc, void * p1);

AM_RETURN am_assca_iter(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2, UINT32 *iter_handle);

/* 32 bit */
AM_RETURN am_assca_read32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_assca_write32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_assca_read32_align(AM_FUNC *pFunc, void * p1, void *p2);
AM_RETURN am_assca_write32_align(AM_FUNC *pFunc, void * p1, void *p2);
#endif


AM_RETURN am_create_list_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap);


#endif

