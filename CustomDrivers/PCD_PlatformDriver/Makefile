obj-m := pcd_device_setup.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR = /home/kumarappan-arunachalam/Work/Projects/Source/linux
HOST_KER_DIR = /lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) help

host:
	make -C $(HOST_KER_DIR) M=$(PWD) modules