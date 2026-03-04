struct pcdev_platform_data
{
    int size;
    int perm;
    const char *serial_number;
};

/* Macros for file permissions */
#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR 0x11

void pcdev_release(struct device *dev);