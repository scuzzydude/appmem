
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
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 offset = *(UINT32 *) p1;

	PUT32_TO_USER(((UINT64)fd->flat.data + (UINT64)offset), p2);

	//AM_DEBUGPRINT( "am_flat_read32_align: offset=%d base=%p valaddr=%016llx val=%08x p2=%p\n", offset, fd->flat.data, ((UINT64)fd->flat.data + (UINT64)offset),*(UINT32 *)((UINT64)fd->flat.data + (UINT64)offset), p2 );

	return AM_RET_GOOD;

}


AM_RETURN am_flat_write32_align(AM_HANDLE handle, void * p1, void *p2)
{
	
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 offset = *(UINT32 *) p1;
	UINT32 *ptr = (UINT32 *)((UINT64)fd->flat.data + (UINT64)offset);

	GET32_FROM_USER(ptr, p2);

//	AM_DEBUGPRINT( "am_flat_write32_align: offset=%d val=%08x\n", offset, *ptr );

	return AM_RET_GOOD;
}




