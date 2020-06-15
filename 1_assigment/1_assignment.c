#include <linux/init.h>
#include <linux/module.h>
#include<linux/sched.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("BHEEMANAPALLI NAVEEN KUMAR");

static int __init assigment_init(void)
{
	printk(KERN_ALERT "Hello, world\n");
    pr_alert("My pid = %d\n",current->pid);
    pr_alert("My Tgid = %d\n",current->tgid);
    return 0;
}

static void __exit assigment_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(assigment_init);
module_exit(assigment_exit);
