#include <linux/version.h>
#include <linux/kernel.h>
 

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>


#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>

#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */


/* Driver Includes */
#include "appmemlib.h"
#include "appmemd_ioctl.h"



/* Globals */
int appmemd_base_dev_count = 1;
int appmemd_minor = 0;
int appmemd_major = 0;
static struct class *cl; // Global variable for the device class
dev_t base_AMdev = 0;


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
int     appmemd_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg)
{

    APPMEM_CMD_U amCmd;
    UINT32 *pData;
    
    
    printk("Appmemd : appmemd_ioctl inode=%p filp=%p cmd=0x%08x arg=0x%016lx\n", inode, filp, cmd, arg);

    if(arg)
    {

        if(copy_from_user (&amCmd, (void *)arg, 16))
        {
            printk("Appmemd : copy from user error\n");
        }
        else
        {
            printk("Appmemd : amCmd(common) cmd=0x%08x len=%d data=%p\n", amCmd.common.cmd, amCmd.common.len, (void *)amCmd.common.data);

            pData = (UINT32 *)amCmd.common.data;
            
            if(put_user(33, pData))
            {
                printk("Appmemd : put_user ERROR %p\n", pData);
            }

        }
        


    }


    return 0;
}


int appmemd_release(struct inode *inode, struct file *filp)
{
   printk("Appmemd : appmemd_realease\n");

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
};


typedef struct _appmem_kdevice
{
    int minor;
    struct cdev cdev;
} APPMEM_KDEVICE;

APPMEM_KDEVICE *pAMKDevices;

static void appmemd_setup_cdev(APPMEM_KDEVICE  *dev, int index)
{
	int err; 
	int devno = MKDEV(appmemd_major, appmemd_minor + index);
    

    printk("Appmemd : create devno=%d\n", devno);
    
	cdev_init(&dev->cdev, &appmemd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &appmemd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
	{
		printk(KERN_NOTICE "Error %d adding appmemd%d", err, index);
    }
}


static int __init appmemd_init(void) 
{

    int result = 0;

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

        if((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
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
            appmemd_setup_cdev(pAMKDevices, 0);
	    }
        else
        {
            result = -ENOMEM;
        	printk(KERN_WARNING "appmemd: Error Alloc Base Device %d\n", result);
        }
        
	}

    return result;
}
 
static void __exit appmemd_exit(void) 
{

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
