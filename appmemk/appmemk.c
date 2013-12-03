/*****************************************************************************************

Copyright (c) 2013, Brandon Awbrey
All rights reserved.

https://github.com/scuzzydude/appmem



Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************************/

#include <linux/mm.h>		/* everything */
#include <linux/errno.h>	/* error codes */
#include <asm/pgtable.h>
#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/time.h>   // for using jiffies  
#include <linux/timer.h>

/* Driver Includes */
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_k.h"
#include "am_k_sock.h"
#include "am_caps.h"
#include "appmem_pack.h"


int appmemd_open(struct inode *inode, struct file *filp);
loff_t  appmemd_llseek(struct file *filp, loff_t off, int whence);
ssize_t appmemd_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos);
ssize_t appmemd_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos);
int appmemd_ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg);
int appmemd_release(struct inode *inode, struct file *filp);
int appmem_get_cap_count(AM_FUNC *pBaseFunc, APPMEM_KAM_CMD_T *pKCmd);
int appmem_get_capabilites(AM_FUNC *pBaseFunc, APPMEM_KAM_CMD_T *pKCmd);



/* Globals */
int appmemd_base_dev_count = 32;
int appmemd_minor = 0;
int appmemd_major = 0;
static struct class *cl; // Global variable for the device class
dev_t base_AMdev = 0;


APPMEM_KAM_CMD_T g_temp_KCMD; /* TODO: This will be a pool */





//******************************* Module Registration ***********************************/
#define MAX_APPMEMK_DEVICE_FUNCTIONS 100
AM_FUNCTION_ENTRY *gAppmemkDeviceFunctionEntry[MAX_APPMEMK_DEVICE_FUNCTIONS] = { 0 };

AM_RETURN am_register_am_function(AM_FUNCTION_ENTRY *pFunctionEntry)
{
	printk("am_register_am_function amType=%d\n", pFunctionEntry->amType);
	if(NULL != pFunctionEntry)
    {
		if(( pFunctionEntry->amType < MAX_APPMEMK_DEVICE_FUNCTIONS) && 
		    (NULL == gAppmemkDeviceFunctionEntry[pFunctionEntry->amType]))
		{
			gAppmemkDeviceFunctionEntry[pFunctionEntry->amType] = pFunctionEntry;
		}
		else
		{
			AM_DEBUGPRINT("Device Function Already Registered\n");
		}
	}
	return AM_RET_GOOD;
}
//******************************* Module Registration End ***********************************/



/* TODO: For Target code consolidation, rectify  with virtd functions later */
AM_RETURN am_targ_release(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;
		
}
AM_RETURN am_targ_open(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;

}
AM_RETURN am_targ_close(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;
}



void appmemd_vma_open(struct vm_area_struct *vma)
{
    printk(KERN_NOTICE "appmemd_vma_open\n");
    
}

void appmemd_vma_close(struct vm_area_struct *vma)
{
    printk(KERN_NOTICE "appmemd_vma_close\n");
}

int appmemd_map_thread(void *pvDevice)
{
    int count = 0;
    APPMEM_KDEVICE *pDevice = pvDevice;
    AM_MEM_FUNCTION_T *pFunc;
    unsigned long j0,j1;
    int delay = 1*HZ;
    DEVICE_MMAP *pMapDevice;
    UINT32 tx_bytes;
    

    if(NULL == pDevice)
    {
        printk("appmemd_map_thread pDevice=NULL\n");
        return -1;
    }

    pFunc = pDevice->pFunc;

    if(NULL == pDevice)
    {
        printk("appmemd_map_thread pFunc=NULL\n");
        return -1;
        
    }
    
    if(NULL == pDevice->map.pMapped)
    {
        printk("appmemd_map_thread pMapped=NULL\n");
        return -1;
    }

    if(0 == pDevice->map.isMapped)
    {
        printk("appmemd_map_thread isMapped=0\n");
        return -1;
    }
    
    pMapDevice = pDevice->map.pMapped;

    /* Synchronize with client */


    pMapDevice->mapPi = AM_K_MAP_PI_START;
    pMapDevice->mapCi = AM_K_MAP_CI_START;

    while((pDevice->map.isMapped) && (pMapDevice->mapPi != pMapDevice->mapCi))
    {
        j0 = jiffies; 
        j1 = j0 + delay; 
        while (time_before(jiffies, j1))
        {
            schedule();
        }

        printk("appmemd_map_thread(%d) WAIT for Client SYNC pi=%08x ci=%08x\n", count, pMapDevice->mapPi, pMapDevice->mapCi);
        count++;
    }

    printk("appmemd_map_thread(%d) SYNCED  pi=%08x ci=%08x\n", count, pMapDevice->mapPi, pMapDevice->mapCi);
 
    pMapDevice->mapState = AM_MAP_STATE_SYNCED;
    pFunc->pVdF->common.flags |= AM_FUNC_DATA_FLAG_MMAPPED;
    
                                                   
    while(pDevice->map.isMapped)
    {
        count++;
        if(pMapDevice->mapPi == pMapDevice->mapCi)
        {
            if(0 == (count % 10000))
            {
                AM_DEBUGPRINT("appmemd_map_thread %s = %d NO CHANGE\n", pDevice->am_name, count);
                schedule();
            }
            
        }
        else
        {
            AM_DEBUGPRINT("appmemd_map_thread COMMAND RCV %s = PI=0x%08x\n", pDevice->am_name, pMapDevice->mapPi);


            am_pack_process_cmd(pFunc, &pMapDevice->mapTx, &pMapDevice->mapRx, &tx_bytes);

            AM_DEBUGPRINT("appmemd_map_thread COMMAND RESP data[4]=%08x TX_BYTES=%d\n", *(UINT32 *)&pMapDevice->mapRx.align_resp.resp_bytes[0], tx_bytes);


            /* Set the CI - Client is waiting */
            pMapDevice->mapCi = pMapDevice->mapPi;
            count = 0;


        }

    }


    printk("appmemd_map_thread EXIT(%d)\n", count);
    

    return 0;


}

int appmemd_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{

    struct page *pPage = NULL; 
    unsigned long physaddr;
    unsigned long pageframe;
    unsigned long len = 0; 
    unsigned long req_len = 0;
    void * pMapped;
    APPMEM_KDEVICE *pDevice;
    
    pDevice = vma->vm_private_data;
    req_len = (vma->vm_end - vma->vm_start);
    physaddr = (unsigned long)((len) + (vma->vm_pgoff << PAGE_SHIFT));
    pageframe = physaddr >> PAGE_SHIFT; 

    printk(KERN_NOTICE "appmemd_vma_fault(0) req_len=%ld  - %p : %p\n", req_len, (void *)vma->vm_end, (void *)vma->vm_start);
    printk(KERN_NOTICE "appmemd_vma_fault(1) vmf->virtual=%p vma->vm_start=%p\n", (void *)vmf->virtual_address, (void *)vma->vm_start);
    len = PAGE_SIZE << 1;  

    printk(KERN_NOTICE "appmemd_vma_fault(2) phys_addr=0x%016lx len=%ld pageframe=%ld\n", physaddr, len, pageframe);
    

    pMapped = (void *)__get_free_pages(GFP_KERNEL, 1);
    memset(pMapped, 0, len);

    printk(KERN_NOTICE "appmemd_vma_fault(3) pMapped=%p len=%ld\n", pMapped, len);
    printk(KERN_NOTICE "appmemd_vma_fault(4) pDevice=%p \n", pDevice);

    if((NULL != pMapped) && (NULL != pDevice))
    {

        pPage = virt_to_page(pMapped);        
        get_page(pPage);
        printk(KERN_NOTICE "appmemd_vma_fault(5) pPage=%p \n", pPage);
        vmf->page = pPage;


        pDevice->map.pMapped = pMapped;
        pDevice->map.req_len = req_len;
        pDevice->map.map_len = len;
        pDevice->map.isMapped = 1;

       
        strcpy((char *)pMapped, "BRANDON TEST");

        pDevice->map.pMapThread = kthread_create(appmemd_map_thread, pDevice, "am_map_thr_%s", pDevice->am_name);

        if(NULL != pDevice->map.pMapThread)
        {
            printk(KERN_NOTICE "Map Thread Created =%p\n", pDevice->map.pMapThread);
            wake_up_process(pDevice->map.pMapThread);    
        }
        else
        {
            printk(KERN_NOTICE "Map Thread Create Error =%p\n", pDevice->map.pMapThread);

        }

    }
    else
    {

        return VM_FAULT_NOPAGE;
    
    }


    return 0;
}


struct vm_operations_struct appmemd_vm_ops = 
{
	.open =     appmemd_vma_open,
	.close =    appmemd_vma_close,
	.fault =    appmemd_vma_fault,
};


int appmemd_mmap(struct file *filp, struct vm_area_struct *vma)
{
	APPMEM_KDEVICE *pDevice;
    struct inode *inode = filp->f_dentry->d_inode;

    
	/*  Find the device */
	pDevice = container_of(inode->i_cdev, APPMEM_KDEVICE, cdev);

    printk(KERN_NOTICE "appmemd_mmap(%p) am_name=%s\n", pDevice, pDevice->am_name);

    if(NULL == pDevice)
    {
    	return -ENODEV;
    }
    
	vma->vm_ops = &appmemd_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = pDevice;
	appmemd_vma_open(vma);
	return 0;
}



struct file_operations appmemd_fops = 
{
	.owner =    THIS_MODULE,
	.llseek =   appmemd_llseek,
	.read =     appmemd_read,
	.write =    appmemd_write,
	.ioctl =    appmemd_ioctl,
	.open =     appmemd_open,
	.release =  appmemd_release,
	.mmap =     appmemd_mmap,
};

APPMEM_KDEVICE *pAMKDevices;
APPMEM_KDEVICE *pFunctionDevicesHead = NULL;;



static void appmemd_setup_cdev(APPMEM_KDEVICE  *dev, int devno)
{
	int err; 
	
    printk("Appmemd : create devno=%d\n", devno);
    
	cdev_init(&dev->cdev, &appmemd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &appmemd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
    dev->devt = devno;
	/* Fail gracefully if need be */
	if (err)
	{
		printk(KERN_NOTICE "Error %d adding appmemd%d", err, devno);
    }
}




int appmemd_open(struct inode *inode, struct file *filp)
{
    printk("Appmemd : appmemd_open\n");

    return 0;
}


ssize_t appmemd_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos)
{
    printk("Appmemd : appmemd_read\n");

    return 0;
}
ssize_t appmemd_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos)
{
    printk("Appmemd : appmemd_write\n");

    return 0;
}
loff_t  appmemd_llseek(struct file *filp, loff_t off, int whence)
{
    printk("Appmemd : appmemd_seek\n");

    return 0;
}



APPMEM_KAM_CMD_T * appmemd_cmd_free_pool_get(void)
{
    /* TODO: This will be a pool */
    /* Ideally, we'll have a single lock on the KAM CMD */
    return &g_temp_KCMD;

}

APPMEM_KDEVICE * appmem_device_func_create(char *name, UINT32 amType)
{
    UINT32 i;
    APPMEM_KDEVICE *pDevice = NULL;
    dev_t dev_func_t = 0;
    char full_name[32];

    sprintf(full_name, "%s%d", name, appmemd_minor);
    
    AM_DEBUGPRINT("appmem_device_func_create = %s : %d\n", name, amType);

    dev_func_t = MKDEV(appmemd_major, appmemd_minor);

    appmemd_minor++;
    
    if (device_create(cl, NULL, dev_func_t, NULL, full_name) == NULL)
    {
        printk(KERN_WARNING "appmemd: appmem_device_func_create error appmem\n");
        return pDevice;
        
    }
   
    AM_DEBUGPRINT("appmem_device_func_create = device_create\n");
    

    pDevice = (APPMEM_KDEVICE *)kmalloc(sizeof(APPMEM_KDEVICE), GFP_KERNEL);
	
    if(pDevice)
    {

        AM_DEBUGPRINT("appmem_device_func_create = alloc pDevice=%p\n", pDevice);
   
        memset(pDevice, 0, sizeof(APPMEM_KDEVICE));

        pDevice->amType = amType;

        appmemd_setup_cdev(pDevice, dev_func_t);

        pDevice->pfnOps = kmalloc((sizeof(am_cmd_fn) * AM_OP_MAX_OPS), GFP_KERNEL);
        strncpy(pDevice->am_name, full_name, 32);
        
        for(i = 0; i < AM_OP_MAX_OPS; i++)
        {

            pDevice->pfnOps[i].config = NULL;     
        }
            
     }           

    if(NULL == pFunctionDevicesHead)
    {
        /* We're at the head */
        pDevice->next = NULL; /* For clarity */
        pFunctionDevicesHead = pDevice;
    }
    else
    {
        pDevice->next = pFunctionDevicesHead;
        pFunctionDevicesHead = pDevice;
    }
    

    return pDevice;
}


AM_RETURN appmemd_release_device(APPMEM_KDEVICE *pDevice)
{
    AM_MEM_FUNCTION_T      *pFunc = NULL;

    printk("appmemd_release_device(%p)\n", pDevice);

    if(NULL != pDevice)
    {
        pFunc = pDevice->pFunc;

        if(NULL != pFunc)
        {
            AM_FREE(pFunc);
        }

        if(NULL != pDevice->pfnOps)
        {
            kfree(pDevice->pfnOps);
        }

        kfree(pDevice);

    }

    return AM_RET_GOOD;
    
}


AM_RETURN appmem_create_function(AM_FUNC *pBaseFunc, APPMEM_KAM_CMD_T *pKCmd)
{
   int error = 0;
   APPMEM_CMD_BIDIR_T     *pBDCmd = (APPMEM_CMD_BIDIR_T *)&pKCmd->cmd;
   AM_MEM_CAP_T           aCap;
   APPMEM_KDEVICE         *pNewDevice = NULL;
   AM_MEM_FUNCTION_T      *pFunc = NULL;
    
   AM_DEBUGPRINT("appmem_create_function = data_in=%p  len_in=%d\n", (void *)pBDCmd->data_in, pBDCmd->len_in);

   if(AM_TYPE_BASE_APPMEM != pBaseFunc->amType)
   {
        return -ENOTTY;
   }
   


   if(pBDCmd->len_in >= sizeof(AM_MEM_CAP_T))
   {
        
        if(copy_from_user (&aCap, (void *)pBDCmd->data_in, sizeof(AM_MEM_CAP_T)))
        {
            AM_DEBUGPRINT( "copy from user error\n");
            return -ENOTTY;

        }
        else
        {
            AM_DEBUGPRINT("Switch aCap.amType=0x%08x maxSize=%lld\n", aCap.amType, aCap.maxSize);

            pFunc = AM_MALLOC(sizeof(AM_MEM_FUNCTION_T));

            
            if(NULL == pFunc)
            {
                error = AM_RET_ALLOC_ERR;
                
            }    
            else
            {


                switch(aCap.amType)
                {


                    case AM_TYPE_FLAT_MEM:
                    {
                        pNewDevice = appmem_device_func_create("am_flat", AM_TYPE_FLAT_MEM);

                        AM_DEBUGPRINT("am_flat pNewDevice=%p\n", pNewDevice);
                        
                        if((NULL != pNewDevice) && (NULL != pNewDevice->pfnOps))
                        {
                            pFunc->pfnOps = pNewDevice->pfnOps;
                            
                            error = am_create_flat_device(pFunc, &aCap);

                            AM_DEBUGPRINT("am_flat am_create_flat_device(error=%d)\n", error);

                            if(AM_RET_GOOD == error)
                            {
                                pNewDevice->pFunc = pFunc;
                            }
                        

                        }
                        else
                        {
                            error = AM_RET_ALLOC_ERR;
                        }

                    }
                    break;

                    case AM_TYPE_ARRAY:
                    {
                        pNewDevice = appmem_device_func_create("am_stata", AM_TYPE_ARRAY);

                        AM_DEBUGPRINT("am_stata pNewDevice=%p\n", pNewDevice);
                        
                        if((NULL != pNewDevice) && (NULL != pNewDevice->pfnOps))
                        {
                            pFunc->pfnOps = pNewDevice->pfnOps;
                            
                            error = am_create_stata_device(pFunc, &aCap);

                            AM_DEBUGPRINT("am_stata am_create_stata_device(error=%d)\n", error);

                            if(AM_RET_GOOD == error)
                            {
                                pNewDevice->pFunc = pFunc;
                            }
                        }
                        else
                        {
                            error = AM_RET_ALLOC_ERR;
                        }

                    }
                    break;


                    case AM_TYPE_ASSOC_ARRAY:
                    {
                        pNewDevice = appmem_device_func_create("am_assca", AM_TYPE_ASSOC_ARRAY);

                        AM_DEBUGPRINT("am_assca pNewDevice=%p\n", pNewDevice);
                        
                        if((NULL != pNewDevice) && (NULL != pNewDevice->pfnOps))
                        {
                            pFunc->pfnOps = pNewDevice->pfnOps;
                            
                            error = am_create_assca_device(pFunc, &aCap);

                            AM_DEBUGPRINT("am_assca am_create_am_assca_device(error=%d)\n", error);

                            if(AM_RET_GOOD == error)
                            {
                                pNewDevice->pFunc = pFunc;

                                /* TODO - duplication of this info in various places */
				                pNewDevice->pack_DataOffset = pFunc->crResp.pack_DataOffset;     
				                pNewDevice->wr_pack_size = pFunc->crResp.wr_pack_qword_size * 8;
				                pNewDevice->rd_pack_size = pFunc->crResp.rd_pack_qword_size * 8;

                            }
                        }
                        else
                        {
                            error = AM_RET_ALLOC_ERR;
                        }

                    }
                    break;

                default:
                {
                    error = -ENOTTY;
                }
                break;
               
                }
            }
        }


   }
   else
   {
      return -ENOTTY;
   }

   if(AM_RET_GOOD == error)
   {

        strncpy((char *)&pFunc->crResp.am_name[0], (char *)pNewDevice->am_name, 32);
        pFunc->crResp.devt = pNewDevice->devt;
        pFunc->amType = pNewDevice->amType;
        
        AM_DEBUGPRINT("create_function copy crResp = %s devt = %d\n", pFunc->crResp.am_name, pFunc->crResp.devt );

        if(copy_to_user ((void *)pBDCmd->data_out, &pFunc->crResp, sizeof(APPMEM_RESP_CR_FUNC_T)))
        {
            error =  -ENOMEM;
            AM_DEBUGPRINT("create_function copy crResp error!!!!\n");

        }
   }
   else
   {
        /* Clean up */



   }




   return error;
}



AM_RETURN appmem_get_capabilites(AM_FUNC *pBaseFunc, APPMEM_KAM_CMD_T *pKCmd)
{
    
    void *pData;
    void *pCaps;
    UINT32 cap_count = 0;
    UINT32 transfer_len = 0;
    printk("Appmemd : appmem_get_capabilites\n");
    

    pData = (void *)pKCmd->cmd.common.data;

    if(AM_TYPE_BASE_APPMEM == pBaseFunc->amType)
    {
        /* Don't over write the buffer / truncate to the nearest CAP */
        cap_count = pKCmd->cmd.common.len / sizeof(AM_MEM_CAP_T);        

        transfer_len = cap_count * sizeof(AM_MEM_CAP_T);
        
        pCaps = (void *)virtd_caps;

        printk("Appmemd : appmem_get_capabilites BASE_APPMEM len=%d transfer_len=%d cap_count=%d : %p\n", pKCmd->cmd.common.len, transfer_len, cap_count, pCaps);


    }
    else
    {
        /* TODO: get the current configured capabilites of the function that was created as this device */
        
    }

    if(0 != transfer_len)
    {
        if(copy_to_user (pData, pCaps, transfer_len))
        {
           printk("Appmemd : copy_to_user len=%d ERROR %p\n", transfer_len, pData);
           return -ENOTTY;
 
        }

    }
    
    return 0;
}


//AM_RETURN appmem_get_cap_count(struct _appmem_kdevice *pDevice, APPMEM_KAM_CMD_T *pKCmd)
AM_RETURN appmem_get_cap_count(AM_FUNC *pBaseFunc, APPMEM_KAM_CMD_T *pKCmd)

{
    UINT32 *pData;
    UINT32 cap_count = 0;
    printk("Appmemd : appmem_get_cap_count\n");
    

    pData = (void *)pKCmd->cmd.common.data;
            
    
    if(AM_TYPE_BASE_APPMEM == pBaseFunc->amType)
    {
        cap_count = sizeof(virtd_caps) / sizeof(AM_MEM_CAP_T);
        printk("Appmemd : cap_count BASE_APPMEM = %d\n", cap_count);
        
    }
    else
    {
        cap_count = 1;
        printk("Appmemd : cap_count Device Type %d = %d\n", pBaseFunc->amType, cap_count);
    }

    if(put_user(cap_count, pData))
    {   
        printk("Appmemd : put_user ERROR %p\n", pData);
        return -ENOTTY;
   
    }


    return 0;
}




int appmemd_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg)
{

    APPMEM_KAM_CMD_T *pKCmd;
    UINT32 cmd_bytes;
    APPMEM_KDEVICE *pDevice;
    AM_FUNC *pFunc;
    
    void *rdptr;
    

    AM_DEBUGPRINT("Appmemd : ioctl cmd=%08x\n", cmd);

	if (_IOC_TYPE(cmd) != APPMEMD_IOC_MAGIC)
    {
	    printk("Appmemd : not magic =%08x\n", _IOC_TYPE(cmd));
        return -ENOTTY;
    }

    cmd_bytes = _IOC_NR(cmd);
    
	if (cmd_bytes > APPMEMD_IOC_MAXNR) 
    {
	    printk("Appmemd : %d larger than %d \n", cmd_bytes, APPMEMD_IOC_MAXNR);
       return -ENOTTY;
    }

    cmd_bytes = (cmd_bytes * 8) + 8;

    AM_DEBUGPRINT("Appmemd : appmemd_ioctl inode=%p filp=%p cmd_bytes=%d cmd=0x%08x arg=0x%016lx\n", inode, filp, cmd_bytes, cmd, arg);

    if(arg)
    {

	    /*  Find the device */
	    pDevice = container_of(inode->i_cdev, APPMEM_KDEVICE, cdev);

        if(pDevice)
        {
            AM_DEBUGPRINT("Appmemd : minor=%d pDevice=%p\n", pDevice->minor, pDevice);
            pFunc = pDevice->pFunc;
            if(NULL == pFunc)
            {
                return -ENOTTY; 
            }
        }    
        else
        {
            return -ENOTTY;
        }

        pKCmd = appmemd_cmd_free_pool_get();

        if(pKCmd)
        {
            //ASSERT(cmd_bytes <= sizeof(APPMEM_CMD_U));

       

            if(copy_from_user (&pKCmd->cmd, (void *)arg, cmd_bytes))
            {
                printk("Appmemd : copy from user error\n");
           //     return -ENOTTY;
            }
            else
            {

     
                AM_DEBUGPRINT("Appmemd : cmd(common) cmd=0x%08x len=%d data=%p\n", pKCmd->cmd.common.op, pKCmd->cmd.common.len, (void *)pKCmd->cmd.common.data);

                if(NULL != pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].config)
                {


                
                    if(IS_OP_ALIGNED(pKCmd->cmd.common.op))
                    {
                        AM_DEBUGPRINT("Appmemd : aligned\n");

                        if(0x1 & pKCmd->cmd.common.op)
                        {
                        
                            return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pFunc, &pKCmd->cmd.aligned.offset, (void *)pKCmd->cmd.aligned.data);
                        }
                        else
                        {

                            return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pFunc, &pKCmd->cmd.aligned.offset, &pKCmd->cmd.aligned.data);
                        }

                    }
                    else if(IS_OP_PACKET(pKCmd->cmd.common.op))
                    {


                        if(0x1 & pKCmd->cmd.common.op)
                        {


                            if(cmd_bytes >= (pDevice->rd_pack_size))
                            {
                                rdptr = (void *)(*(UINT64 *)&pKCmd->cmd.packet.data[pFunc->crResp.pack_DataOffset]);
#if 0
                                
                                AM_DEBUGPRINT("Valid PACKET READ cmd_bytes=%d rd_pack_size =%d RDPTR=%p Rp0=0x%08x Rp1=0x%08x\n", 
                                cmd_bytes , pDevice->rd_pack_size, rdptr,
                                pKCmd->cmd.packet.data[pDevice->pack_DataOffset],
                                pKCmd->cmd.packet.data[pDevice->pack_DataOffset + 1]);
#endif
                                return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pFunc, &pKCmd->cmd.packet.data[0], rdptr);
                            }
                            else
                            {
                                AM_DEBUGPRINT("Invalid cmd_byte=%d : wr_pack_qword_size=%d\n", cmd_bytes, pDevice->wr_pack_size);
                                return -ENOTTY;
            
                            }


                                    
                        }
                        else
                        {

                            if(cmd_bytes >= (pDevice->wr_pack_size))
                            {
                                
//                                printk("Valid PACKET WRITE cmd_bytes=%d wr_pack_size =%d\n", cmd_bytes , pDevice->wr_pack_size);
                                return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pFunc, &pKCmd->cmd.packet.data[0], &pKCmd->cmd.packet.data[pDevice->pack_DataOffset]);
                            }
                            else
                            {
//                                printk("Invalid cmd_byte=%d : wr_pack_qword_size=%d\n", cmd_bytes, pDevice->wr_pack_size);
                                return -ENOTTY;
            
                            }

                        }


                    }

                    else
                    {
                        printk("Appmemd : config\n");

                        return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].config(pFunc, pKCmd);
                    }
                }
                else
                {
                    printk("Appmemd: Invalid Opcode for device : %08x\n", pKCmd->cmd.common.op);
                    return -ENOTTY;
        
                }
            
            }

        }
        else
        {
            printk("Appmemd : error pKCmd Free pool get\n");
           
            return -ENOTTY;
        
        }


    }


    return 0;
}


int appmemd_release(struct inode *inode, struct file *filp)
{
   printk("Appmemd : appmemd_realease\n");

	return 0;
}




static int __init appmemd_init(void) 
{

    int result = 0;
    int i;
    printk(KERN_INFO "appmemd: registered");
    printk(KERN_INFO "appmemd: Linux Version Code(%08x)", LINUX_VERSION_CODE);

    printk(KERN_INFO "Appmemlib Version v%d.%d.%d.%d\n", AM_VER_MAJOR, AM_VER_MINOR, AM_VER_PATCH, AM_VER_BUILD);


    for(i = 0; i < sizeof(virtd_caps) / sizeof(AM_MEM_CAP_T); i++)
    {
        if(i)
        {
          /* Advertise that we can access through MMAP */
           virtd_caps[i].access_flags |= AM_CAP_AC_FLAG_PACK_MMAP;
        }
    }

    result = alloc_chrdev_region(&base_AMdev, appmemd_minor, appmemd_base_dev_count,
				"appmem");
	appmemd_major = MAJOR(base_AMdev);
	if (result < 0) 
	{
		printk(KERN_WARNING "appmemd: can't get major %d\n", appmemd_major);
	}
	else
	{

        if((cl = class_create(THIS_MODULE, "appmem")) == NULL)
        {
            printk(KERN_WARNING "appmemd: class create error\n");
            return -1;
        }

        if (device_create(cl, NULL, base_AMdev, NULL, "appmem") == NULL)
        {
            printk(KERN_WARNING "appmemd: device create error appmem\n");
            return -1;
        
        }
		printk(KERN_WARNING "appmemd: major number %d\n", appmemd_major);

        pAMKDevices = kmalloc(appmemd_base_dev_count * sizeof(APPMEM_KDEVICE), GFP_KERNEL);
	
        if(pAMKDevices)
        {
            memset(pAMKDevices, 0, appmemd_base_dev_count * sizeof(APPMEM_KDEVICE));

            pAMKDevices->amType = AM_TYPE_BASE_APPMEM;
            appmemd_setup_cdev(pAMKDevices, MKDEV(appmemd_major, appmemd_minor));

            pAMKDevices->pfnOps = kmalloc((sizeof(am_cmd_fn) * AM_OP_MAX_OPS), GFP_KERNEL);

            pAMKDevices->pFunc = kmalloc(sizeof(AM_FUNC), GFP_KERNEL);

            pAMKDevices->pFunc->amType = AM_TYPE_BASE_APPMEM;
            
            for(i = 0; i < AM_OP_MAX_OPS; i++)
            {

                pAMKDevices->pfnOps[i].config = NULL;     
            }

            pAMKDevices->pfnOps[AM_OPCODE(AM_OP_CODE_GETC_CAP_COUNT)].config = (am_cmd_fn)appmem_get_cap_count;
            pAMKDevices->pfnOps[AM_OPCODE(AM_OP_CODE_GET_CAPS)].config = (am_cmd_fn)appmem_get_capabilites;
            pAMKDevices->pfnOps[AM_OPCODE(AM_OP_CODE_CREATE_FUNC)].config = (am_cmd_fn)appmem_create_function;
            
            pAMKDevices->pFunc->pfnOps = pAMKDevices->pfnOps; 
	    }
        else
        {
            result = -ENOMEM;
        	printk(KERN_WARNING "appmemd: Error Alloc Base Device %d\n", result);
        }
        
	}

    appmemd_minor++;
    
     
    
//    am_k_sock_init(0xC0A801DD); //192.168.1.221
//    am_k_sock_init(0x7F000001); //127.0.0.1 - locahost
    


    return result;
}
 
static void __exit appmemd_exit(void) 
{


    APPMEM_KDEVICE *pDevice;

    while(pFunctionDevicesHead)
    {
        pDevice = pFunctionDevicesHead;
        pFunctionDevicesHead = pDevice->next;

        AM_DEBUGPRINT("exit pDevice = %p : amType=%d, dev=%d\n", pDevice, pDevice->amType, pDevice->devt );

        if(pDevice->map.isMapped)
        {
            pDevice->map.isMapped = 0;

            if(pDevice->map.pMapThread)
            {
                kthread_stop(pDevice->map.pMapThread);
            }    
            
        }

        cdev_del(&pDevice->cdev);

        AM_DEBUGPRINT("exit pDevice = %p : cdev_del\n", pDevice);
   
        device_destroy(cl, pDevice->devt);
 
        AM_DEBUGPRINT("exit pDevice = %p : device_destroy\n", pDevice);

        if(NULL != pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config)
        {
            pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config(pDevice->pFunc, NULL);
        }

        appmemd_release_device(pDevice);
          

    }
   




  if(pAMKDevices)
  {
     printk(KERN_INFO "appmemd : cdev_del %p", pAMKDevices);

     cdev_del(&pAMKDevices->cdev);

     

     appmemd_release_device(pAMKDevices);

  }
  if(cl)
  {
    printk(KERN_INFO "appmemd : device/class destroy %p", cl);
    device_destroy(cl, base_AMdev);
    class_destroy(cl);
  }
  unregister_chrdev_region(base_AMdev, 1);

  printk(KERN_INFO "appmemd : unregistered");
}
 
module_init(appmemd_init);
module_exit(appmemd_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brandon Awbrey");
MODULE_DESCRIPTION("App Memory Driver");
