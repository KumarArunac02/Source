#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>

#define DEV_MEM_SIZE 512

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

/* Pseudo device's memory */
char device_buffer[DEV_MEM_SIZE];

/* Holds the device number */
dev_t device_number;

/*cdev Variable*/
struct cdev pcd_cdev;

loff_t pcd_lseek(struct file *filp, loff_t off, int whence)
{
    pr_info("lseek requested\n");
	return 0;
}
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("Read requested for %zu bytes\n", count);
	return 0;
}
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("Write requested for %zu bytes\n", count);
	return 0;
}
int pcd_open(struct inode *inode, struct file *filp)
{
    pr_info("Open was successful\n");
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

struct class *class_pcd;
struct device *device_pcd;

static int __init PCD_Driver_Init(void)
{
    int ret;

    /* Dynamically allocate a device number */
    ret = alloc_chrdev_region(&device_number, 0, 1, "pcd_devices");

    pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

    pcd_fops.open = pcd_open;

    /* Initialize the cdev structure with fops */
    cdev_init(&pcd_cdev, &pcd_fops);

    /* Register a device (cdev) structure with VFS */
    pcd_cdev.owner = THIS_MODULE;
    ret = cdev_add(&pcd_cdev, device_number, 1);

    /* Create device class under /sys/class/ */
    class_pcd = class_create(THIS_MODULE,"pcd_class");

    /* Populate the sysfs with device information */
    device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");

    pr_info("Module init was successful\n");
    
    return 0;
}

static void __exit PCD_Driver_CleanUp(void)
{
    device_destroy(class_pcd, device_number);
    class_destroy(class_pcd);
    cdev_del(&pcd_cdev);
    unregister_chrdev_region(device_number, 1);
    pr_info("module unloaded\n");
}

module_init(PCD_Driver_Init);
module_exit(PCD_Driver_CleanUp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kumarappan Arunachalam");
MODULE_DESCRIPTION("A simple Out of Tree Kernel module to access the memory region");
MODULE_INFO(board,"Beaglebone Black REV C");

