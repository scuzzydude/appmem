#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_k.h"




AM_RETURN appmem_flat_release(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd)
{
    /* We don't need the pKCmd, it can be NULL and driver can call this directly on exit */

    AM_ASSERT(pDevice);

    if(pDevice->pVdF)
    {
        if(pDevice->pVdF->flat.data)
        {
            vfree((void *)pDevice->pVdF->flat.data);
	    }	
    }
    return 0;
}


int appmem_create_flat_device(AM_MEM_CAP_T *pCap, APPMEM_CMD_BIDIR_T *pBDCmd)
{
    int error = 0;
    AM_FUNC_DATA_U *pVdF = NULL;
    APPMEM_KDEVICE *pDevice = NULL;
    APPMEM_RESP_CR_FUNC_T respCr;
    

    AM_DEBUGPRINT( "appmem_create_flat_device: maxSize=%lld\n", pCap->maxSize );

    pVdF = AM_MALLOC(sizeof(AM_FUNC_DATA_U));
    
    if(pVdF)
    {
	    pVdF->flat.size = pCap->maxSize;
		pVdF->flat.add_size = pCap->typeSpecific[TS_FLAT_ADDRESS_BYTE_SIZE];
		pVdF->flat.data = vmalloc((size_t)pCap->maxSize); /* VMalloc for large virtual buffer */
				
        

		if(NULL != pVdF->flat.data)
		{

            AM_DEBUGPRINT( "appmem_create_flat_device: data=%p\n", pVdF->flat.data);

		    if(pCap->typeSpecific[TS_INIT_MEM_VAL])
			{
            
			}
			else
			{
			    memset(pVdF->flat.data, 0, (size_t) pVdF->flat.size);
			}

            pDevice = appmem_device_func_create("am_flat", AM_TYPE_FLAT_MEM);

            if(pDevice)
            {
                pDevice->pfnOps = kmalloc((sizeof(am_cmd_fn) * AM_OP_MAX_OPS), GFP_KERNEL);
                
                memset(pDevice->pfnOps, 0, (sizeof(am_cmd_fn) * AM_OP_MAX_OPS));
                
                pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config  = appmem_flat_release;

                pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_READ_ALIGN)].align  = am_flat_read32_align;
                pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_WRITE_ALIGN)].align  = am_flat_write32_align;


                pDevice->pVdF = pVdF;
                
                if(pBDCmd->len_out >= sizeof(APPMEM_RESP_CR_FUNC_T))
                {
                    memset(&respCr, 0, sizeof(APPMEM_RESP_CR_FUNC_T));
                    strncpy((char *)&respCr.am_name, (char *)pDevice->am_name, 32);
                    respCr.devt = pDevice->devt;

                    AM_DEBUGPRINT( "appmem_create_flat_device: am_name=%s devt=%d\n", pDevice->am_name, pDevice->devt);

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
        if(pVdF->flat.data)
        {
           vfree(pVdF->flat.data);
        }
        
        if(pVdF)
        {
            AM_FREE(pVdF);
        }    
    }

    return error;

}

