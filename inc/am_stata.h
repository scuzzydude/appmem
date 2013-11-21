#ifndef _AM_STATA_H
#define _AM_STATA_H

AM_RETURN am_stata_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_stata_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_stata_sort(AM_HANDLE handle, void * p1, UINT64 l1);
AM_RETURN am_create_stata_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap);


/* 32 bit */
AM_RETURN am_stata_read_multi_idx32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_stata_write_multi_idx32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_stata_read_idx32(AM_HANDLE handle, void * p1, void *p2);
AM_RETURN am_stata_write_idx32(AM_HANDLE handle, void * p1, void *p2);

#endif


