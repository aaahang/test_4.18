#include "asm-generic/gpio.h"
#include "asm/gpio.h"
#include "asm/io.h"
#include "asm/uaccess.h"
#include "linux/cdev.h"
#include "linux/dmi.h"
#include "linux/err.h"
#include "linux/export.h"
#include "linux/gpio.h"
#include "linux/jump_label.h"
#include "linux/kdev_t.h"
#include "linux/mod_devicetable.h"
#include "linux/node.h"
#include "linux/of.h"
#include "linux/printk.h"
#include "linux/stddef.h"
#include "linux/types.h"
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/timer.h>


static struct oftree_drv {
    dev_t ot_dev; 
    struct cdev *cdev; //字符设备的
    struct property *  pt; //链表
    struct class * class;
    struct timer_list led_time ;   
    int led_gpio
}general_drv;
void led_time_function(unsigned long data)
{
    gpio_set_value(general_drv.led_gpio,!gpio_get_value(general_drv.led_gpio));
    mod_timer(&general_drv.led_time,jiffies  + msecs_to_jiffies(1000));
}
static int ot_open(struct inode * id, struct file * file)
{ 
    struct device_node * led_devnode =  of_find_node_by_path("/led_stoicus");
    if (led_devnode == NULL)
     printk("%d\n",__LINE__);
    general_drv.led_gpio = of_get_named_gpio(led_devnode,"D5",0);
    if(gpio_request(general_drv.led_gpio,"led_gpio")!= 0)
    {
        printk("%s,%d\n",__FUNCTION__,__LINE__);
    }
    gpio_direction_output(general_drv.led_gpio,0);
    asd;
    init_timer(&general_drv.led_time);
    general_drv.led_time.function =  led_time_function;
    general_drv.led_time.expires  = jiffies  + msecs_to_jiffies(1000);
    add_timer(&general_drv.led_time);
    return 0;

}
// 进程退出时会调用
int general_drv_release(struct inode * id, struct file * file)
{
    printk("%s,%d\n",__FUNCTION__,__LINE__);
    gpio_free( general_drv.led_gpio);
    del_timer(&general_drv.led_time);
    
}
static struct file_operations ot_fp =
{
    .open = ot_open,
    .release = general_drv_release
};  
static int __init of_init(void)
{
   if( alloc_chrdev_region(&general_drv.ot_dev,0,1,"stoicus_ot"))
   {
        printk("%d\n",__LINE__);
    	return -EFAULT;
   }
    if((general_drv.cdev = cdev_alloc()) ==NULL)
    {
        printk("%d",__LINE__);
        goto err0;
    }
    cdev_init(general_drv.cdev,&ot_fp);
    cdev_add(general_drv.cdev,general_drv.ot_dev,1);
    if(IS_ERR(general_drv.class = class_create(THIS_MODULE,"stoicus_OT_TREE")))
    {
        printk("%d",__LINE__);
        goto err1;
    }
    device_create(general_drv.class,NULL,general_drv.ot_dev,NULL,"stoicus_ot_tree");
    return 0;
err1:    
        cdev_del(general_drv.cdev);
err0: 
    unregister_chrdev_region(general_drv.ot_dev,1);
    return -1;
}
static void  __exit of_exit(void)
{
    device_destroy(general_drv.class,general_drv.ot_dev);
    class_destroy(general_drv.class);
    cdev_del(general_drv.cdev);
    unregister_chrdev_region(general_drv.ot_dev,1);
}

module_init(of_init);
module_exit(of_exit);
MODULE_LICENSE("GPL");
