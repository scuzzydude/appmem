
#include "appmemlib.h"
#include "am_flat.h"

AM_RETURN am_flat_open(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2) 
{
	return AM_RET_GOOD;

}
AM_RETURN am_flat_close(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}


/* 32 bit */
AM_RETURN am_flat_read32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;

}
AM_RETURN am_flat_write32(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;

}

AM_RETURN am_flat_read32_align(UINT32 handle, void * p1, void *p2)
{
	AM_FUNC_DATA_U * fd = am_handle_to_funcdata(handle);
	UINT32 offset = *(UINT32 *) p1;
	*(UINT32 *)p2 = *(UINT32 *)((UINT32)fd->flat.data + offset);

	return AM_RET_GOOD;

}

AM_RETURN am_flat_write32_align(UINT32 handle, void * p1, void *p2)
{
	
	AM_FUNC_DATA_U * fd = am_handle_to_funcdata(handle);
	UINT32 offset = *(UINT32 *) p1;
	UINT32 *ptr = (UINT32 *)((UINT32)fd->flat.data + offset);

	*ptr = *(UINT32 *)p2;

	return AM_RET_GOOD;
}


