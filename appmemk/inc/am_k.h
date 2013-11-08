#ifndef _AM_K_H
#define _AM_K_H


#include "am_flat.h"
#include "am_stata.h"
#include "am_assca.h"

/* Common to base driver */
APPMEM_KDEVICE * appmem_device_func_create(char *name, UINT32 amType);



/* am_k_flat INTERFACE TO am_flat.c */

AM_RETURN appmem_flat_release(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd);
int appmem_create_flat_device(AM_MEM_CAP_T *pCap, APPMEM_CMD_BIDIR_T *pBDCmd);

/* am_k_stata INTERFACE To am_stata.c */

AM_RETURN appmem_stata_release(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd);
int appmem_create_stata_device(AM_MEM_CAP_T *pCap, APPMEM_CMD_BIDIR_T *pBDCmd);

/* am_k_assca INTERACE to am_assca.c */

AM_RETURN appmem_assca_release(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd);
int appmem_create_assca_device(AM_MEM_CAP_T *pCap, APPMEM_CMD_BIDIR_T *pBDCmd);















#endif

