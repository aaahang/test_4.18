#include "linux/delay.h"
#include "linux/printk.h"
#include "linux/trace_events.h"
#include <linux/i2c.h>

int am2320_read(struct i2c_client *am2320_client, int data[2])
{
    char read_data[7];
    char set_data[3] = {0x03,0x00,0x04};
    struct i2c_msg msg[2];
    msg[0].addr = am2320_client->addr;
    msg[0].flags = 0;
    msg[0].buf = set_data;
    msg[0].len = 3;
    int ret ;
    do {
    ret =i2c_transfer(am2320_client->adapter,&msg[0],1);
    }while (ret<0);
    mdelay(2);
    msg[1].addr = am2320_client->addr; /* ap3216c 地址 */
    msg[1].flags = I2C_M_RD; /* 标记为读取数据 */
    msg[1].buf = read_data; /* 读取数据缓冲区 */
    msg[1].len = 7; /* 要读取的数据长度 */
     

    
    ret =i2c_transfer(am2320_client->adapter,&msg[1],1);
    // int i;
    // for( i= 0 ;i<7; i++)
    // printk("read_data[%d] is 0x%x , ",i,read_data[i]);
    data[0]  = (read_data[3]|read_data[2]<<8); // 
    data[1] = (read_data[5]|read_data[4]<<8);
}