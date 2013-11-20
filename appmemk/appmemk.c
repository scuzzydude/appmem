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


/* Driver Includes */
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_k.h"
#include "am_k_sock.h"



int appmemd_open(struct inode *inode, struct file *filp);
loff_t  appmemd_llseek(struct file *filp, loff_t off, int whence);
ssize_t appmemd_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos);
ssize_t appmemd_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos);
int appmemd_ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg);
int appmemd_release(struct inode *inode, struct file *filp);
int appmem_get_cap_count(struct _appmem_kdevice *pDevice, APPMEM_KAM_CMD_T *pKCmd);
int appmem_get_capabilites(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd);



/* Globals */
int appmemd_base_dev_count = 32;
int appmemd_minor = 0;
int appmemd_major = 0;
static struct class *cl; // Global variable for the device class
dev_t base_AMdev = 0;


APPMEM_KAM_CMD_T g_temp_KCMD; /* TODO: This will be a pool */


AM_MEM_CAP_T appmemd_caps[] = 
{

	{ 
		AM_TYPE_BASE_APPMEM,
		(1024 * 1024 * 128), /* Total Device Memory */
		2,
		{
			0, 0
		}
	},
	{ 
		AM_TYPE_FLAT_MEM, /* Type */ 
		(1024 * 1024),    /* maxSizeBytes */
		7,    			  /* maxFunction  */	
		{
			(8 | 4 | 2 | 1),  /* Mem - Address Size */ 
			1,                /* Mem - Min Byte action */
			(1024 * 1024)     /* Mem - Max Byte action */
		}
	
	},
	{ 
		AM_TYPE_ARRAY,
		(1024 * 1024),
		7,
		{
			(8 | 4 | 2 | 1),  /* Array - Index Size Bytes */ 
			1,                /* Min Value Size - Bytes */
			1024,              /* Max Value Size - Bytes */
			TS_STAT_DT_FIXED_WIDTH
		}

	},
	{
		AM_TYPE_ASSOC_ARRAY,
		(1024 * 1024),
		7,
		{
			512,              /* Max - Key Size Bytes */ 
			1024 * 1024,      /* Max - Data Size Bytes */
			TS_ASSCA_KEY_FIXED_WIDTH | TS_ASSCA_KEY_VAR_WIDTH,
			TS_ASSCA_DATA_FIXED_WIDTH | TS_ASSCA_DATA_VAR_WIDTH
		}
	}

};

    


struct file_operations appmemd_fops = 
{
	.owner =    THIS_MODULE,
	.llseek =   appmemd_llseek,
	.read =     appmemd_read,
	.write =    appmemd_write,
	.ioctl =    appmemd_ioctl,
	.open =     appmemd_open,
	.release =  appmemd_release,
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



AM_RETURN appmem_create_function(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd)
{
   int error = 0;
   APPMEM_CMD_BIDIR_T *pBDCmd = (APPMEM_CMD_BIDIR_T *)&pKCmd->cmd;
   AM_MEM_CAP_T aCap;
   
   AM_DEBUGPRINT("appmem_create_function = data_in=%p  len_in=%d\n", (void *)pBDCmd->data_in, pBDCmd->len_in);

   if(AM_TYPE_BASE_APPMEM != pDevice->amType)
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

            switch(aCap.amType)
            {

                case AM_TYPE_FLAT_MEM:
                {
                    error = appmem_create_flat_device(&aCap, pBDCmd);     
                }
                break;

                case AM_TYPE_ARRAY:
                {
                    error = appmem_create_stata_device(&aCap, pBDCmd);     
                }
                break;

                case AM_TYPE_ASSOC_ARRAY:
                {
                    error = appmem_create_assca_device(&aCap, pBDCmd);

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
   else
   {
      return -ENOTTY;
   }



   return error;
}



AM_RETURN appmem_get_capabilites(APPMEM_KDEVICE *pDevice, APPMEM_KAM_CMD_T *pKCmd)
{
    
    void *pData;
    void *pCaps;
    UINT32 cap_count = 0;
    UINT32 transfer_len = 0;
    printk("Appmemd : appmem_get_capabilites\n");
    

    pData = (void *)pKCmd->cmd.common.data;

    if(AM_TYPE_BASE_APPMEM == pDevice->amType)
    {
        /* Don't over write the buffer / truncate to the nearest CAP */
        cap_count = pKCmd->cmd.common.len / sizeof(AM_MEM_CAP_T);        

        transfer_len = cap_count * sizeof(AM_MEM_CAP_T);
        
        pCaps = (void *)appmemd_caps;

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


AM_RETURN appmem_get_cap_count(struct _appmem_kdevice *pDevice, APPMEM_KAM_CMD_T *pKCmd)
{
    UINT32 *pData;
    UINT32 cap_count = 0;
    printk("Appmemd : appmem_get_cap_count\n");
    

    pData = (void *)pKCmd->cmd.common.data;
            
    
    if(AM_TYPE_BASE_APPMEM == pDevice->amType)
    {
        cap_count = sizeof(appmemd_caps) / sizeof(AM_MEM_CAP_T);
        printk("Appmemd : cap_count BASE_APPMEM = %d\n", cap_count);
        
    }
    else
    {
        cap_count = 1;
        printk("Appmemd : cap_count Device Type %d = %d\n", pDevice->amType, cap_count);
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
    void *rdptr;
    

//    printk("Appmemd : ioctl cmd=%08x\n", cmd);

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

    //printk("Appmemd : appmemd_ioctl inode=%p filp=%p cmd_bytes=%d cmd=0x%08x arg=0x%016lx\n", inode, filp, cmd_bytes, cmd, arg);

    if(arg)
    {

	    /*  Find the device */
	    pDevice = container_of(inode->i_cdev, APPMEM_KDEVICE, cdev);

        if(pDevice)
        {
    //        printk("Appmemd : minor=%d pDevice=%p\n", pDevice->minor, pDevice);
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
            }
            else
            {

     
           //     printk("Appmemd : cmd(common) cmd=0x%08x len=%d data=%p\n", pKCmd->cmd.common.op, pKCmd->cmd.common.len, (void *)pKCmd->cmd.common.data);

                if(NULL != pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].config)
                {


                
                    if(IS_OP_ALIGNED(pKCmd->cmd.common.op))
                    {
      //                  printk("Appmemd : aligned\n");

                        if(0x1 & pKCmd->cmd.common.op)
                        {
                        
                            return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pDevice, &pKCmd->cmd.aligned.offset, (void *)pKCmd->cmd.aligned.data);
                        }
                        else
                        {

                            return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pDevice, &pKCmd->cmd.aligned.offset, &pKCmd->cmd.aligned.data);
                        }

                    }
                    else if(IS_OP_PACKET(pKCmd->cmd.common.op))
                    {


                        if(0x1 & pKCmd->cmd.common.op)
                        {


                            if(cmd_bytes >= (pDevice->rd_pack_size))
                            {
                                rdptr = (void *)(*(UINT64 *)&pKCmd->cmd.packet.data[pDevice->pack_DataOffset]);
#if 0
                                
                                printk("Valid PACKET READ cmd_bytes=%d rd_pack_size =%d RDPTR=%p Rp0=0x%08x Rp1=0x%08x\n", 
                                cmd_bytes , pDevice->rd_pack_size, rdptr,
                                pKCmd->cmd.packet.data[pDevice->pack_DataOffset],
                                pKCmd->cmd.packet.data[pDevice->pack_DataOffset + 1]);
#endif
                                return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pDevice, &pKCmd->cmd.packet.data[0], rdptr);
                            }
                            else
                            {
                                printk("Invalid cmd_byte=%d : wr_pack_qword_size=%d\n", cmd_bytes, pDevice->wr_pack_size);
                                return -ENOTTY;
            
                            }


                                    
                        }
                        else
                        {

                            if(cmd_bytes >= (pDevice->wr_pack_size))
                            {
                                
  //                              printk("Valid PACKET WRITE cmd_bytes=%d wr_pack_size =%d\n", cmd_bytes , pDevice->wr_pack_size);
                                return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].align(pDevice, &pKCmd->cmd.packet.data[0], &pKCmd->cmd.packet.data[pDevice->pack_DataOffset]);
                            }
                            else
                            {
                                printk("Invalid cmd_byte=%d : wr_pack_qword_size=%d\n", cmd_bytes, pDevice->wr_pack_size);
                                return -ENOTTY;
            
                            }

                        }


                    }



                    else
                    {
                        printk("Appmemd : config\n");

                        return pDevice->pfnOps[AM_OPCODE(pKCmd->cmd.common.op)].config(pDevice, pKCmd);
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

            for(i = 0; i < AM_OP_MAX_OPS; i++)
            {

                pAMKDevices->pfnOps[i].config = NULL;     
            }

            pAMKDevices->pfnOps[AM_OPCODE(AM_OP_CODE_GETC_CAP_COUNT)].config = appmem_get_cap_count;
            pAMKDevices->pfnOps[AM_OPCODE(AM_OP_CODE_GET_CAPS)].config = appmem_get_capabilites;
            pAMKDevices->pfnOps[AM_OPCODE(AM_OP_CODE_CREATE_FUNC)].config = appmem_create_function;
            
            
	    }
        else
        {
            result = -ENOMEM;
        	printk(KERN_WARNING "appmemd: Error Alloc Base Device %d\n", result);
        }
        
	}

    appmemd_minor++;
    
     
    
//    am_k_sock_init(0xC0A801DD); //192.168.1.221
//     am_k_sock_init(0x7F000001); //127.0.0.1 - locahost
    


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
   

        cdev_del(&pDevice->cdev);

        AM_DEBUGPRINT("exit pDevice = %p : cdev_del\n", pDevice);
   
        device_destroy(cl, pDevice->devt);
 
        AM_DEBUGPRINT("exit pDevice = %p : device_destroy\n", pDevice);

        if(NULL != pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config)
        {
            pDevice->pfnOps[AM_OPCODE(AM_OP_CODE_RELEASE_FUNC)].config(pDevice, NULL);
        }
        kfree(pDevice);
        

    }
   




  if(pAMKDevices)
  {
     printk(KERN_INFO "appmemd : cdev_del %p", pAMKDevices);

     cdev_del(&pAMKDevices->cdev);
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
