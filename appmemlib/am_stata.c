#include "appmemlib.h"
#include "am_stata.h"


AM_RETURN am_stata_open(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}
AM_RETURN am_stata_close(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}


/* 32 bit */
AM_RETURN am_stata_read_multi_idx32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}
AM_RETURN am_stata_write_multi_idx32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}

AM_RETURN am_stata_read_idx32(UINT32 handle, void * p1, void *p2)
{
	UINT32 idx = *(UINT32 *)p1;
	AM_FUNC_DATA_U * fd = am_handle_to_funcdata(handle);

	*(UINT32 *)p2 =	((UINT32 *)fd->stata.data)[idx];


	return AM_RET_GOOD;

}
AM_RETURN am_flat_write_idx32(UINT32 handle, void * p1, void *p2)
{
	UINT32 idx = *(UINT32 *)p1;
	AM_FUNC_DATA_U * fd = am_handle_to_funcdata(handle);
	
	((UINT32 *)fd->stata.data)[idx] = *(UINT32 *)p2;

	return AM_RET_GOOD;

}
