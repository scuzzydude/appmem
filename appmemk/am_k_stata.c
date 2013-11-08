
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_k.h"


AM_RETURN appmem_stata_release(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd)
{
    /* We don't need the pKCmd, it can be NULL and driver can call this directly on exit */

    AM_ASSERT(pDevice);
    if(pDevice->pVdF)
    {
        if(pDevice->pVdF->stata.data)
        {
            vfree((void *)pDevice->pVdF->stata.data);
	    }	
    }
    return 0;

}


int appmem_create_stata_device(AM_MEM_CAP_T *pCap, APPMEM_CMD_BIDIR_T *pBDCmd)
{
    int error = 0;
    AM_FUNC_DATA_U *pVdF = NULL;
    APPMEM_KDEVICE *pDevice = NULL;
    APPMEM_RESP_CR_FUNC_T respCr;
    

    AM_DEBUGPRINT( "appmem_create_stata_device: maxSize=%lld\n", pCap->maxSize );

    pVdF = AM_MALLOC(sizeof(AM_FUNC_DATA_U));
    
    if(pVdF)
    {
		pVdF->stata.idx_size = pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE];
		pVdF->stata.data_size = pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE];
		pVdF->stata.size = pVdF->stata.data_size * pCap->maxSize;
		pVdF->stata.data = vmalloc((size_t)pVdF->stata.size);

                

		if(NULL != pVdF->stata.data)
		{

            AM_DEBUGPRINT( "appmem_create_stata_device: data=%p\n", pVdF->stata.data);

			memset(pVdF->stata.data, 0, (size_t) pVdF->stata.size);
		
            pDevice = appmem_device_func_create("am_stata", AM_TYPE_ARRAY);

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

                    AM_DEBUGPRINT( "appmem_create_stata_device: am_name=%s devt=%d\n", pDevice->am_name, pDevice->devt);

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
        
        if(pVdF)
        {
            AM_FREE(pVdF);
        }    
    }

    return error;

}

