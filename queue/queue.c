#include <linux/init.h>
#include <linux/module.h>
#include<linux/sched.h>
#include<linux/kfifo.h>
MODULE_LICENSE("GPL");

static struct kfifo fifo;

static int __init init_code(void){
	
	unsigned int ret;
	unsigned int i=0;
	
	ret = kfifo_alloc(&fifo, PAGE_SIZE, GFP_KERNEL);
	if (ret)
		return ret;
		
	for (i = 0; i < 32; i++)
		kfifo_in(&fifo, (void *)&i, sizeof(i));
		
	/*ret = kfifo_out_peek(fifo, &val, sizeof(val), 0);
		if (ret != sizeof(val))
			return -EINVAL;
*/

	while (kfifo_avail(&fifo)) {
        unsigned int val;
        unsigned int ret;
		ret = kfifo_out(&fifo, &val, sizeof(val));
		if (ret != sizeof(val))
			return -EINVAL;
		printk(KERN_INFO "%u \n", val);
	}
	return 0;
}


static void __exit exit_code(void){
    pr_info("clean-up code");
    kfifo_free(&fifo);

}


module_init(init_code);
module_exit(exit_code);
