#ifndef _APPMEM_VIRTD_H
#define _APPMEM_VIRTD_H

AM_RETURN am_virtd_get_capabilites_count(AMLIB_ENTRY_T *pEntry);
AM_RETURN am_virtd_get_capabilities(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count);
AM_RETURN am_virtd_create_function(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc);

AM_RETURN am_virtd_open(AM_MEM_FUNCTION_T *pFunc);
AM_RETURN am_virtd_close(AM_MEM_FUNCTION_T *pFunc);

#endif
