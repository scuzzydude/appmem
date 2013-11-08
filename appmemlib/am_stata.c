#include "appmemlib.h"
#include "am_stata.h"


AM_RETURN am_stata_open(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}
AM_RETURN am_stata_close(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}


/* 32 bit */
AM_RETURN am_stata_read_multi_idx32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}
AM_RETURN am_stata_write_multi_idx32(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2)
{
	return AM_RET_GOOD;

}

AM_RETURN am_stata_read_idx32(AM_HANDLE handle, void *p1, void *p2)
{
	UINT32 idx = *(UINT32 *)p1;
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
    UINT32 *ptr = &((UINT32 *)fd->stata.data)[idx];    


	PUT32_TO_USER(ptr, p2);
    

	return AM_RET_GOOD;

}
AM_RETURN am_stata_write_idx32(AM_HANDLE handle, void *p1, void *p2)
{
	UINT32 idx = *(UINT32 *)p1;
	AM_FUNC_DATA_U * fd = AM_HANDLE_TO_FUNCDATA(handle);
	UINT32 *ptr = &((UINT32 *)fd->stata.data)[idx];

	GET32_FROM_USER(ptr, p2);


	return AM_RET_GOOD;

}
