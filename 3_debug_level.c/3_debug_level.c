#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h> 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BHEEMANAPALLI NAVEEN KUMAR");
static unsigned int debug_level=8;
module_param(debug_level,int,0660);
static int debug_level_init(void)
{
    if(debug_level==0)
    {
	    pr_debug("NO DEBUG MESSAGE \n");
	}
	else if(debug_level==1) 
	{
	    pr_debug("LOW DEBUG VERBOSITY \n");
	}
	else if(debug_level==2)
	{
	    pr_debug("MEDIUM DEBUG VERBOSITY \n");
	}
	else if(debug_level==3)
	{
	    pr_debug("HIGH DEBUG VERBOSITY \n");
	}
	return 0;
}
static void debug_level_cleanup(void)
{
    if(debug_level == 8){
        pr_debug("INVALID DEBUG LEVEL \n");
        goto label;
    }
	pr_debug("DEBUG LEVEL = %d\n",debug_level);
    label :
        pr_debug("cleanup code \n");
} 
module_init(debug_level_init);
module_exit(debug_level_cleanup);

