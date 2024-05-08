#include "asm-generic/current.h"
#include "asm-generic/gpio.h"
#include "asm-generic/poll.h"
#include "asm/atomic.h"
#include "asm/gpio.h"
#include "asm/io.h"
#include "asm/uaccess.h"
#include "linux/blkdev.h"
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
#include "linux/sched.h"
#include "linux/stddef.h"
#include "linux/types.h"
#include "linux/wait.h"
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
#include <linux/irqreturn.h>

static struct oftree_drv {
    //LED
    dev_t ot_dev; 
    struct cdev *cdev; //字符设备的
    struct property *  pt; //链表
    struct class * class;
    struct timer_list led_time;   
    int time_num ;
    int led_gpio;
    //KEY
    int key_gpio; 
    int irqnum;
    atomic_t key_value;
}general_drv;
//初始化等待队列头
static DECLARE_WAIT_QUEUE_HEAD(stoicus_wait);
static DECLARE_WAIT_QUEUE_HEAD(stoicus_wait_poll);
// tasklet 中断函数
void tasklet_func(unsigned long data)
{
    printk("%s,%d\n",__FUNCTION__,__LINE__);
}
// tasklet初始化 最后一个参数为传入中断函数的值
static DECLARE_TASKLET(led_tasklet,tasklet_func,0);
// timer初始化 
void key_timer_function(unsigned long dat)
{
    if(gpio_get_value(general_drv.key_gpio)==1)
    {
        atomic_set(&general_drv.key_value,1);
        printk("key is  down\n");
    }
    else
    {
        printk("key is  up\n");
        atomic_set(&general_drv.key_value,0);
        // wake_up_interruptible(&stoicus_wait); // 唤醒等待队列令其不再阻塞
    }

}
static DEFINE_TIMER(key_timer,key_timer_function,0,0); 
void led_time_function(unsigned long data)
{
    gpio_set_value(general_drv.led_gpio,!gpio_get_value(general_drv.led_gpio));
    mod_timer(&general_drv.led_time,jiffies  + msecs_to_jiffies(general_drv.time_num));
}
static irqreturn_t stoicus_irq_handler_t(int irq_num, void * data)
{
    tasklet_schedule(&led_tasklet); //令tasklet运行
    mod_timer(&key_timer,jiffies + msecs_to_jiffies(10));  //开启key定时器 消抖

    return IRQ_RETVAL(IRQ_HANDLED);
}

static int ot_open(struct inode * id, struct file * file)
{ 
    //LED
    struct device_node * led_devnode =  of_find_node_by_path("/led_stoicus");
    if (led_devnode == NULL)
     printk("%d\n",__LINE__);
    general_drv.led_gpio = of_get_named_gpio(led_devnode,"D5",0);
    if(gpio_request(general_drv.led_gpio,"led_gpio")!= 0)
    {
        printk("%s,%d\n",__FUNCTION__,__LINE__);
    }
    gpio_direction_output(general_drv.led_gpio,0);
    
    general_drv.led_time.function =  led_time_function;
    general_drv.time_num = 1000;
    general_drv.led_time.expires  = jiffies  + msecs_to_jiffies(general_drv.time_num);
    add_timer(&general_drv.led_time);

    //KEY
    struct device_node * key_devnode =  of_find_node_by_path("/led_stoicus/key_stoicus");
    if (key_devnode == NULL)
        printk("%d\n",__LINE__);
    general_drv.key_gpio = of_get_named_gpio(key_devnode,"sw2",0);
    if(gpio_request(general_drv.key_gpio,"key_gpio")!= 0)
     {
        printk("%s,%d\n",__FUNCTION__,__LINE__);
        goto key_err;
     }
    gpio_direction_input(general_drv.key_gpio);

    general_drv.irqnum =irq_of_parse_and_map(key_devnode,0);
    // 需要和设备树上一致 否则会报错
    if(request_irq(general_drv.irqnum,stoicus_irq_handler_t,IRQF_TRIGGER_RISING |IRQF_TRIGGER_FALLING,"stoicus_irq",&general_drv)<0) //上升沿下降沿都触发
     {
        printk("%s,%d\n",__FUNCTION__,__LINE__);
        goto irq_err;
     }
     //阻塞io配置

    return 0;
irq_err:
    gpio_free(general_drv.key_gpio);
key_err: 
    del_timer_sync(&general_drv.led_time);
// time_err:
    gpio_free(general_drv.led_gpio);
        return -1;   
}
#define IO_TIMESTOP _IO(0,'C')
#define IO_TIMEWIRTE _IOW(1,'C',int)

// 进程退出时会调用
int ot_drv_release(struct inode * id, struct file * file)
{
    printk("%s,%d\n",__FUNCTION__,__LINE__);
    free_irq(general_drv.irqnum,&general_drv);
    //del_timer_sync 函数是 del_timer 函数的同步版，会等待其他处理器使用完定时器再删除，del_timer_sync 不能使用在中断上下文中
    del_timer_sync(&general_drv.led_time);
    gpio_free(general_drv.led_gpio);
    gpio_free(general_drv.key_gpio);
    return 0;
}
union 
{
    int value;
    char data[4];
}read_data;
ssize_t ot_read(struct file * fe, char __user * buf, size_t size, loff_t * loff)
{
    read_data.value =atomic_read(&general_drv.key_value);
    printk("%s , %d",__FUNCTION__,__LINE__);
    //申请当前进程的等待
    return   copy_to_user(buf, read_data.data, 4);

}
unsigned int ot_poll(struct file * file, struct poll_table_struct * wait)
{
    unsigned int mask = 0; //一直返回0 就一直阻塞 直到超时 
    poll_wait(file, &stoicus_wait_poll, wait); //poll 函数的主要函数
    printk("%s,%d\n",__FUNCTION__,__LINE__);
    if (atomic_read(&general_drv.key_value)==1)
    {
         mask |= POLLIN|POLLRDNORM;
    }
    return mask;
}

static struct file_operations ot_fp =
{
    .open = ot_open,
    .release = ot_drv_release,
    .read = ot_read,
    .poll = ot_poll
};  
 static const struct of_device_id irq_match[] = {
	{ .compatible = "stiocus_led_stoicus" },
	{},
};
int irq_probe(struct platform_device * device)
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
    unregister_chrdev_region(general_drv.ot_dev,1);\
    return -1;
}
int irq_remove(struct platform_device *device)
{
    device_destroy(general_drv.class,general_drv.ot_dev);
    class_destroy(general_drv.class);
    cdev_del(general_drv.cdev);
    unregister_chrdev_region(general_drv.ot_dev,1);
    return 0;
}
static struct platform_driver  stiocus_irq = {
	.probe = irq_probe,
    .remove  = irq_remove,
	.driver = {
		.name	=   "stiocus_led_stoicus",
		.of_match_table	= irq_match,
	},
};
static int __init of_init(void)
{
    platform_driver_register(&stiocus_irq);
    return 0;
}
static void  __exit of_exit(void)
{
    platform_driver_unregister(&stiocus_irq);
}

module_init(of_init);
module_exit(of_exit);
MODULE_LICENSE("GPL");