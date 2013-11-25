#ifndef _AM_FLAT_H
#define _AM_FLAT_H

AM_RETURN am_create_flat_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap);


AM_RETURN am_flat_open(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_flat_close(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_flat_release(AM_FUNC *pFunc, void * p1);



/* 32 bit */
AM_RETURN am_flat_read32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_flat_write32(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);

extern AM_RETURN am_flat_read32_align(AM_FUNC *pFunc, void * p1, void *p2);
extern AM_RETURN am_flat_write32_align(AM_FUNC *pFunc, void * p1, void *p2);
#endif

