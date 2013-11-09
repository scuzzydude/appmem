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
	//	pVdF->stata.idx_size = pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE];
	//	pVdF->stata.data_size = pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE];
	//	pVdF->stata.size = pVdF->stata.data_size * pCap->maxSize;
	//	pVdF->stata.data = vmalloc((size_t)pVdF->stata.size);


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


                if(pBDCmd->len_out >= sizeof(APPMEM_RESP_CR_FUNC_T))
                {
                    memset(&respCr, 0, sizeof(APPMEM_RESP_CR_FUNC_T));
                    strncpy((char *)&respCr.am_name, (char *)pDevice->am_name, 32);
                    respCr.devt = pDevice->devt;

                
                    pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config  = appmem_assca_release;
                    respCr.data_size = data_size;
                    respCr.idx_size = key_size;

                    if((TRUE == bFixedKey) && (TRUE == bFixedData))
                    {

                        pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_READ_FIX_PACKET)].align  = am_assca_read32_align;
                        pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_WRITE_FIX_PACKET)].align  = am_assca_write32_align;

                        respCr.acOps[ACOP_WRITE] = AM_OP_CODE_WRITE_FIX_PACKET;
                        respCr.acOps[ACOP_READ] = AM_OP_CODE_READ_FIX_PACKET;

               
                        /* To Make Sure that data segment is 32 bit aligned */
                        respCr.pack_DataOffset = respCr.idx_size / sizeof(UINT32);
                    
                        if(respCr.idx_size % sizeof(UINT32))
                        {
                            respCr.pack_DataOffset++;    
                        }

                        printk("PACKET DATA IDX SIZE=%d\n", respCr.idx_size);
                        printk("PACKET DATA DATA SIZE=%d\n", respCr.data_size);
                        printk("PACKET DATA DATA OFFSET DWORDS=%d\n", respCr.pack_DataOffset);
                    
                    
                        respCr.wr_pack_qword_size = respCr.pack_DataOffset;

                        respCr.wr_pack_qword_size += (respCr.data_size / sizeof(UINT32));
                    
                        if(respCr.data_size % sizeof(UINT32))
                        {
                            respCr.wr_pack_qword_size++;
                        }

                        respCr.wr_pack_qword_size++; /* Always need one for opcode */   
                    
                        printk("WR PACKET SIZE DWORDS=%d\n", respCr.wr_pack_qword_size);
                    
                        if(respCr.wr_pack_qword_size % 2)
                        {
                            respCr.wr_pack_qword_size = 1 + (respCr.wr_pack_qword_size / 2);
                        }
                        else
                        {
                            respCr.wr_pack_qword_size = (respCr.wr_pack_qword_size / 2);
                        }

                        
                        printk("WR PACKET SIZE QWORDS=%d\n", respCr.wr_pack_qword_size);
                    


                        respCr.rd_pack_qword_size = respCr.pack_DataOffset;                    

                        respCr.rd_pack_qword_size++; //read user data pointer - two dwords 

                        respCr.rd_pack_qword_size++; //read user data pointer - two dwords 
                        respCr.rd_pack_qword_size++; //read user data pointer - two dwords 


                        printk("RD PACKET SIZE DWORDS=%d\n", respCr.rd_pack_qword_size);

                        if(respCr.rd_pack_qword_size % 2)
                        {

                            respCr.rd_pack_qword_size = 1 + (respCr.rd_pack_qword_size / 2);
                        }
                        else
                        {
                             respCr.rd_pack_qword_size = (respCr.rd_pack_qword_size / 2);
                        
                        }

                        printk("RD PACKET SIZE QWORDS=%d\n", respCr.rd_pack_qword_size);


                        pDevice->pack_DataOffset = respCr.pack_DataOffset;     
                    
                        pDevice->wr_pack_size = respCr.wr_pack_qword_size * 8;
                        pDevice->rd_pack_size = respCr.rd_pack_qword_size * 8;
                        
                        printk("WT PACKET SIZE BYTES=%d\n", pDevice->wr_pack_size);
                        printk("RD PACKET SIZE BYTES=%d\n", pDevice->rd_pack_size);

                        
                    }


                
                    pDevice->pVdF = pVdF;
                
                
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

