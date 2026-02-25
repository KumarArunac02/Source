#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define DEV1_MEM_SIZE_MAX 1024
#define DEV2_MEM_SIZE_MAX 512
#define DEV3_MEM_SIZE_MAX 1024
#define DEV4_MEM_SIZE_MAX 512

#define NO_OF_DEVICES 4

/*Macros for file permissions */
#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR 0x11

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

/* Pseudo device's memory */
char device1_buffer[DEV1_MEM_SIZE_MAX];
char device2_buffer[DEV2_MEM_SIZE_MAX];
char device3_buffer[DEV3_MEM_SIZE_MAX];
char device4_buffer[DEV4_MEM_SIZE_MAX];

/* Device's private Data structure */
struct pcdev_private_data
{
    char *buffer;
    unsigned size;
    const char *serial_num;
    int perm;
    struct cdev cdev;
};

/* Driver's Private Data structure */
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_number; /* Holds the device number */
    struct class *class_pcd;
    struct device *device_pcd;
    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv_data =
{
    .total_devices = NO_OF_DEVICES,
    .pcdev_data = {
        [0] = {
            .buffer = device1_buffer,
            .size = DEV1_MEM_SIZE_MAX,
            .serial_num = "PCDEV1XYZ123",
            .perm = RDONLY,
        },
        [1] = {
            .buffer = device2_buffer,
            .size = DEV2_MEM_SIZE_MAX,
            .serial_num = "PCDEV2XYZ123",
            .perm = WRONLY,
        },
        [2] = {
            .buffer = device3_buffer,
            .size = DEV3_MEM_SIZE_MAX,
            .serial_num = "PCDEV3XYZ123",
            .perm = RDWR,
        },
        [3] = {
            .buffer = device4_buffer,
            .size = DEV4_MEM_SIZE_MAX,
            .serial_num = "PCDEV4XYZ123",
            .perm = RDWR,
        }
    }
};

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
    loff_t temp;

    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*) filp->private_data;
    int max_size = pcdev_data->size;

    pr_info("lseek requested\n");
    pr_info("Current value of the file position = %lld\n", filp->f_pos);

    switch(whence)
    {
        case SEEK_SET:
            if ((offset > max_size) || (offset < 0) )
                return -EINVAL;
            filp->f_pos = offset;
        break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;
            filp->f_pos = temp;
        break;
        case SEEK_END:
            temp = max_size + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;
            filp->f_pos = temp;
        break;
        default:
             return -EINVAL;
        break;
    }
    pr_info("New value of the file position = %lld\n", filp->f_pos);
	return filp->f_pos;
}
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*) filp->private_data;
    int max_size = pcdev_data->size;

    pr_info("Read requested for %zu bytes\n", count);
    pr_info("Current file position = %lld\n", *f_pos);

    /*Adjust the count */
    if((*f_pos+count) > max_size)
        count = max_size - *f_pos;

    /* Copy to user */
    if (copy_to_user(buff, pcdev_data->buffer+(*f_pos), count))
        return -EFAULT;
    
    /* Update the current file position */
    *f_pos += count;

    pr_info("number of bytes which have been successfully read = %zu\n", count);

    /* file pos is of type long long Int. So use %lld format specifier according to the kernel Documentation */
    pr_info("Updated file position = %lld\n", *f_pos);

    /* Return the number of bytes which have been successfully read */
	return count;
}
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*) filp->private_data;
    int max_size = pcdev_data->size;

    pr_info("Write requested for %zu bytes\n", count);
    pr_info("Current file position = %lld\n", *f_pos);

    /*Adjust the count */
    if((*f_pos+count) > max_size)
        count = max_size - *f_pos;

    if (!count)
    {
        pr_err("Memory Full \n");
        return -ENOMEM;
    }


    /* Copy from user */
    if (copy_from_user(pcdev_data->buffer+(*f_pos), buff, count))
        return -EFAULT;
    
    /* Update the current file position */
    *f_pos += count;

    pr_info("number of bytes which have been successfully written = %zu\n", count);

    /* file pos is of type long long Int. So use %lld format specifier according to the kernel Documentation */
    pr_info("Updated file position = %lld\n", *f_pos);

    /* Return the number of bytes which have been successfully read */
	return count;
}

int check_permission(int dev_perm, int acc_mode)
{
    /* To ensure the access from user Space is RDWR */
    if(dev_perm == RDWR)
        return 0;
    
    /* To ensure the access from user Space is RDONLY */
    if((dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE) ))
        return 0;
    
    /* To ensure the access from user Space is RDONLY */
    if((dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ) ))
        return 0;
    
    return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp)
{
    int ret;
    int minor_n;
    struct pcdev_private_data *pcdev_data;
    
    /* Find out on which device the FileOpen Operation is attempted from the user space  */
    minor_n = MINOR(inode->i_rdev);
    pr_info("Minor Access = %d\n", minor_n);

    /* get device's private data structure */
    pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);
    
    /* Supply device private data to other methods of the driver.
     * the received filp is passed to other methods of this device driver like
     * read, write, etc. So storing it in the private data helps to achieve it. */

    filp->private_data = pcdev_data;

    /* Open() should Check permission of the user space call whether its accessing a
     * RDONLY method.
     */
    ret = check_permission(pcdev_data->perm, filp->f_mode);
    (!ret)?pr_info("Open was successful\n"):pr_info("Open was unsuccessful\n");

    return 0;
}
int pcd_release (struct inode *inode, struct file *filp)
{
    pr_info("file closed successfully\n");
    return 0;
}

/* variable for the file operations the driver */
struct file_operations pcd_fops =
{
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .llseek = pcd_lseek,
    .release = pcd_release,
    .owner = THIS_MODULE
};

static int __init PCD_Driver_Init(void)
{

    int ret, i;

    /* 1. Dynamically allocate a device number */
    ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcd_devices");
    if (ret < 0)
    {
        pr_err("chrdev alloc failed\n");
        goto out;
    }
    /* 2. Create device class under /sys/class/ */
    pcdrv_data.class_pcd = class_create(THIS_MODULE,"pcd_class");

    /* class_create returns pointer to a created class or an error code
     * So, Check the returned pointer using IS_ERR.
     */
    if(IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("Class creation failed\n");
        /* convert the returned error code into the pointer by using the below MACRO */
        ret = PTR_ERR(pcdrv_data.class_pcd);
        goto unreg_chrdev;
    }

    for (i=0;i<NO_OF_DEVICES; i++)
    {
        pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(pcdrv_data.device_number+i), MINOR(pcdrv_data.device_number+i));
        pcd_fops.open = pcd_open;

        /* Initialize the cdev structure with fops */
        cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);

        /* Register a device (cdev) structure with VFS */
        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number+i, 1);
        if (ret < 0)
        {   
            pr_err("chrdev add failed\n");
            goto cdev_del;
        }

        /* Populate the sysfs with device information */
        pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number, NULL, "pcdev-%d", i);
        /* device_create returns a pointer to created device or an error code
        * So, Check the returned pointer using IS_ERR.
        */
        if(IS_ERR(pcdrv_data.device_pcd))
        {
            pr_err("Device creation failed\n");

            /* convert the returned error code into the pointer by using the below MACRO */
            ret = PTR_ERR(pcdrv_data.device_pcd);
            goto class_del;
        }
    }

    pr_info("Module init was successful\n");
    
    return 0;

cdev_del:
class_del:
    for(;i>=0;i--)
    {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number+i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.class_pcd);

/* Charecter Device is allocated during the stage of adding the device(cdev_add)
 * So, we have to undo the operations that were completed before adding the device
 * (i.e) unregister the charecter device region that is allocated.
*/
unreg_chrdev:
    unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);
out:
    pr_info("Module insertion failed\n");
    return ret;
}

static void __exit PCD_Driver_CleanUp(void)
{
    int i;
    for(i=0;i>=0;i--)
    {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number+i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);

    pr_info("module unloaded\n");

}

module_init(PCD_Driver_Init);
module_exit(PCD_Driver_CleanUp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kumarappan Arunachalam");
MODULE_DESCRIPTION("Pseudo Char Device Driver to handle n devices");
MODULE_INFO(board,"Beaglebone Black REV C");

