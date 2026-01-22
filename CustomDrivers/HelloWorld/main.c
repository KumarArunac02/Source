#include<linux/module.h>

static int __init HelloWorld_Init(void)
{
    pr_info("Hello World\n");
    return 0;

}

static void __exit HelloWorld_CleanUp(void)
{
    pr_info("Bye World\n");
}

module_init(HelloWorld_Init);
module_exit(HelloWorld_CleanUp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kumarappan Arunachalam");
MODULE_DESCRIPTION("A simple Hello World Linux kernel Module");
MODULE_INFO(board,"Beaglebone Black REV C");
