#ifndef _AM_STATA_H
#define _AM_STATA_H

AM_RETURN am_stata_open(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_stata_close(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);


/* 32 bit */
AM_RETURN am_stata_read_multi_idx32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
AM_RETURN am_stata_write_multi_idx32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);

AM_RETURN am_stata_read_idx32(UINT32 handle, void * p1, void *p2);
AM_RETURN am_flat_write_idx32(UINT32 handle, void * p1, void *p2);

#endif


