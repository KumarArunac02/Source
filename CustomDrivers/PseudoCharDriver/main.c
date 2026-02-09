#include <linux/module.h>
#include <linux/fs.h>

#define DEV_MEM_SIZE 512

/* Pseudo device's memory */
char device_buffer[DEV_MEM_SIZE];

/* Holds the device number */
dev_t device_number;

static int __init PCD_Driver_Init(void)
{
    /* Dynamically allocate a device number */
    alloc_chrdev_region(&device_number, 0, 1, "pcd");
    return 0;
}

static void __exit PCD_Driver_CleanUp(void)
{

}

module_init(PCD_Driver_Init);
module_exit(PCD_Driver_CleanUp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kumarappan Arunachalam");
MODULE_DESCRIPTION("A simple Out of Tree Kernel module to access the memory region");
MODULE_INFO(board,"Beaglebone Black REV C");

