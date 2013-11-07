
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_flat.h"

AM_RETURN am_flat_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2) 
{
	return AM_RET_GOOD;

}
AM_RETURN am_flat_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}


/* 32 bit */
AM_RETURN am_flat_read32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;

}
AM_RETURN am_flat_write32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_IO_ERR;

}

AM_RETURN am_flat_read32_align(AM_HANDLE handle, void * p1, void *p2)
{
//	AM_FUNC_DATA_U * fd = am_handle_to_funcdata(handle);
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 offset = *(UINT32 *) p1;
	*(UINT32 *)p2 = *(UINT32 *)((UINT32)fd->flat.data + offset);

	return AM_RET_GOOD;

}


AM_RETURN am_flat_write32_align(AM_HANDLE handle, void * p1, void *p2)
{
	
//	AM_FUNC_DATA_U * fd = am_handle_to_funcdata(handle);
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 offset = *(UINT32 *) p1;
	UINT32 *ptr = (UINT32 *)((UINT32)fd->flat.data + offset);

	GET32_FROM_TO_USER(ptr, p2);

	return AM_RET_GOOD;
}




