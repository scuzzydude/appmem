#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_k.h"


AM_RETURN appmem_assca_release(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd)
{
    /* We don't need the pKCmd, it can be NULL and driver can call this directly on exit */
#if 0
    AM_ASSERT(pDevice);
    if(pDevice->pVdF)
    {
        if(pDevice->pVdF->stata.data)
        {
            vfree((void *)pDevice->pVdF->stata.data);
	    }	
    }
#endif
    return 0;

}

int appmem_create_assca_device(AM_MEM_CAP_T *pCap, APPMEM_CMD_BIDIR_T *pBDCmd)
{
    UINT8 bFixedKey = TRUE;
	UINT8 bFixedData = TRUE;
    UINT16 flags = 0;
	UINT32 key_size;
	UINT32 data_size;
	int error = 0;
    AM_FUNC_DATA_U *pVdF = NULL;
    APPMEM_KDEVICE *pDevice = NULL;
    APPMEM_RESP_CR_FUNC_T respCr;
	AMLIB_ASSCA *pAA = (AMLIB_ASSCA *)NULL;
    

    AM_DEBUGPRINT( "appmem_create_assca_device: maxSize=%lld\n", pCap->maxSize );
    pVdF = AM_MALLOC(sizeof(AM_FUNC_DATA_U));

   
	if(pVdF && pCap && (AM_TYPE_ASSOC_ARRAY == pCap->amType))
    {
		pVdF->stata.idx_size = pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE];
		pVdF->stata.data_size = pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE];
		pVdF->stata.size = pVdF->stata.data_size * pCap->maxSize;
		pVdF->stata.data = vmalloc((size_t)pVdF->stata.size);


        key_size = pCap->typeSpecific[TS_ASSCA_KEY_MAX_SIZE];
		data_size =	pCap->typeSpecific[TS_ASSCA_DATA_MAX_SIZE];
		
		if(pCap->typeSpecific[TS_ASSCA_KEY_TYPE] & TS_ASSCA_KEY_FIXED_WIDTH)
		{
			bFixedKey = TRUE;
		}
		if(pCap->typeSpecific[TS_ASSCA_DATA_TYPE] & TS_ASSCA_DATA_FIXED_WIDTH)
		{
			bFixedData = TRUE;
		}
		

		pAA = amlib_assca_init(key_size, data_size, bFixedKey, bFixedData, flags);


		if(NULL != pAA)
		{

			pVdF->assca.size = pCap->maxSize;
			pVdF->assca.cur_count = 0;
			pVdF->flat.data = (void *)pAA;

            AM_DEBUGPRINT( "appmem_create_stata_device: pAA=%p\n", pAA);

            pDevice = appmem_device_func_create("am_assca", AM_TYPE_ASSOC_ARRAY);

            if(pDevice)
            {
                pDevice->pfnOps = kmalloc((sizeof(am_cmd_fn) * AM_OP_MAX_OPS), GFP_KERNEL);
                
                memset(pDevice->pfnOps, 0, (sizeof(am_cmd_fn) * AM_OP_MAX_OPS));
                
                pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config  = appmem_stata_release;

                pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_READ_ALIGN)].align  = am_stata_read_idx32;
                pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_WRITE_ALIGN)].align  = am_stata_write_idx32;


                pDevice->pVdF = pVdF;
                
                if(pBDCmd->len_out >= sizeof(APPMEM_RESP_CR_FUNC_T))
                {
                    memset(&respCr, 0, sizeof(APPMEM_RESP_CR_FUNC_T));
                    strncpy((char *)&respCr.am_name, (char *)pDevice->am_name, 32);
                    respCr.devt = pDevice->devt;

                    AM_DEBUGPRINT( "appmem_create_assca_device: am_name=%s devt=%d\n", pDevice->am_name, pDevice->devt);

                    if(copy_to_user ((void *)pBDCmd->data_out, &respCr, sizeof(APPMEM_RESP_CR_FUNC_T)))
                    {
                         error =  -ENOMEM;
                    }

                }
                else
                {
                     error =  -ENOMEM;
                }


            }
            else
            {
                 error =  -ENOMEM;
            }


	    }
	    else
	    {
            AM_DEBUGPRINT( "appmem_create_flat_device: vmalloc failed size=%lld\n", pCap->maxSize);
            error =  -ENOMEM;
   
	    }
    }
    else
    {
        return -ENOMEM;
    }

    if(error)
    {
        if(pVdF->stata.data)
        {
           vfree(pVdF->stata.data);
        }
        
        if(pAA)
        {
            /* TODO: clean up pAA */
        }

        if(pVdF)
        {
            AM_FREE(pVdF);
        }    
    }
    return error;

}

