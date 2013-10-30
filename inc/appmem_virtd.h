#ifndef _APPMEM_VIRTD_H
#define _APPMEM_VIRTD_H

AM_RETURN am_virtd_get_capabilites_count();
AM_RETURN am_virtd_get_capabilities(char *am_name, AM_MEM_CAP_T *pAmCaps, UINT32 count);
AM_RETURN am_virtd_create_function(char *am_name, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc);

#endif
