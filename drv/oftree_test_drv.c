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
    struct device_node *dn;
    struct property *  pt; //链表
    struct class * class;
}ot_drv;
/*
    backlight {
        compatible = "pwm-backlight";
        pwms = <&pwm1 0 1000>;
        brightness-levels = <0 1 2 3 4 5 6 8 10>;
        default-brightness-level = <8>;
        status = "okay";
    };
*/
static int ot_open (struct inode * id, struct file * file)
{
    const char *str ;
    u32 num;
    u32 *data; 
    u8 i;
    ot_drv.dn = of_find_node_by_path("/backlight");//根据节点位置找到节点
    ot_drv.pt = of_find_property(ot_drv.dn,"compatible",NULL);//根据节点名字找到值
    of_property_read_string(ot_drv.dn,"status",&str);
    of_property_read_u32(ot_drv.dn,"default-brightness-level",&num); //设备树上的数据基本都是32位
    int  elemnent_size = of_property_count_elems_of_size(ot_drv.dn,"brightness-levels",sizeof(u32)); //得到设备树中多个数据的个数
    //动态分配内存  kmalloc 分配的内存是连续的 可以用来申请动态数组（使用变量给数组大小）
    data = (u32 *)kmalloc(sizeof(u32)*elemnent_size,GFP_KERNEL);
    of_property_read_u32_array(ot_drv.dn,"brightness-levels",data,elemnent_size);
    printk("value is %s\n",(char*)ot_drv.pt->value);
    printk("status is %s\n",str);
    printk("default-brightness-level is %d\n",num);
    for(i=0;i<elemnent_size;i++)
    printk("brightness-levels[%d] is %d\n",i,data[i]); //kmalloc申请的可以这样使用
    kfree(data);//释放内存
}
static struct file_operations ot_fp =
{
    .open = ot_open,
};  
static int __init of_init(void)
{
   if( alloc_chrdev_region(&ot_drv.ot_dev,0,1,"stoicus_ot"))
   {
        printk("%d\n",__LINE__);
    	return -EFAULT;
   }
    if((ot_drv.cdev = cdev_alloc()) ==NULL)
    {
        printk("%d",__LINE__);
        goto err0;
    }
    cdev_init(ot_drv.cdev,&ot_fp);
    cdev_add(ot_drv.cdev,ot_drv.ot_dev,1);
    if(IS_ERR(ot_drv.class = class_create(THIS_MODULE,"stoicus_OT_TREE")))
    {
        printk("%d",__LINE__);
        goto err1;
    }
    device_create(ot_drv.class,NULL,ot_drv.ot_dev,NULL,"stoicus_ot_tree");
    return 0;
err1:    
        cdev_del(ot_drv.cdev);
err0: 
    unregister_chrdev_region(ot_drv.ot_dev,1);
    return -1;
}
static void  __exit of_exit(void)
{
    device_destroy(ot_drv.class,ot_drv.ot_dev);
    class_destroy(ot_drv.class);
    cdev_del(ot_drv.cdev);
    unregister_chrdev_region(ot_drv.ot_dev,1);

}
module_init(of_init);
module_exit(of_exit);
MODULE_LICENSE("GPL");
