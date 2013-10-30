#ifndef _AM_FLAT_H
#define _AM_FLAT_H


AM_RETURN am_flat_open(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_flat_close(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);


/* 32 bit */
AM_RETURN am_flat_read32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_flat_write32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_flat_read32_align(UINT32 handle, void * p1, void *p2);
AM_RETURN am_flat_write32_align(UINT32 handle, void * p1, void *p2);

#endif