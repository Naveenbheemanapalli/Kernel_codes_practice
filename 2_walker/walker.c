#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h> 
#include <linux/sched.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BHEEMANAPALLI NAVEEN KUMAR");
static unsigned int walk=8;
module_param(walk,int,0660);
static int walker_init(void)
{
    if(walk == 8){
        pr_debug("Usage : sudo insmod ./walker.ko walk = value[0-3]\n");    
    }
    if(walk >= 1 && walk <= 3){
        pr_debug("walk = %d\n",walk);
    }
	else
	{  
	    pr_debug("default =%d\n",walk);
	}
	return 0;
}
static void walker_cleanup(void)
{
	pr_debug("module cleaning up\n");
} 
module_init(walker_init);
module_exit(walker_cleanup);


