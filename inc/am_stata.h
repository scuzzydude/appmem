#ifndef _AM_STATA_H
#define _AM_STATA_H

#define AM_STATA_SUBTYPES  1
#define TS_STATIC_MAX_TS   4

#define TS_STAT_ARRAY_IDX_BYTE_SIZE  0 
#define TS_STAT_ARRAY_VAL_MIN_SIZE   1 
#define TS_STAT_ARRAY_VAL_MAX_SIZE   2 
#define TS_STAT_ARRAY_VAL_DATA_TYPES 3
#define TS_STAT_DT_FIXED_WIDTH   0x1
#define TS_STAT_DT_VAR_WIDTH     0x2


AM_RETURN am_stata_open(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_stata_close(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_stata_sort(AM_FUNC *pFunc, void * p1, UINT64 l1);
AM_RETURN am_create_stata_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap);
AM_RETURN am_stata_release(AM_FUNC *pFunc, void * p1);


/* 32 bit */
AM_RETURN am_stata_read_multi_idx32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_stata_write_multi_idx32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_stata_read_idx32(AM_FUNC *pFunc, void * p1, void *p2);
AM_RETURN am_stata_write_idx32(AM_FUNC *pFunc, void * p1, void *p2);

#endif


