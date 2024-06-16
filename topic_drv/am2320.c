#include "linux/delay.h"
#include "linux/printk.h"
#include "linux/trace_events.h"
#include <linux/i2c.h>
unsigned short crc16(unsigned char *ptr, unsigned char len)
{
unsigned short crc=0xFFFF;
unsigned char i;
        while(len--)
        {
        crc ^=*ptr++;
        for(i=0;i<8;i++)
        {
        if(crc & 0x01)
        {
        crc>>=1;
        crc^=0xA001;
            }else
            {
            crc>>=1;
            }
        }
        }
return crc;
}
int am2320_read(struct i2c_client *am2320_client, int data[2])
{
    char read_data[8];
    char set_data[3] = {0x03,0x00,0x04};
    char read_data_transfer[8];
    static char st_data[2];
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
    msg[1].len = 8; /* 要读取的数据长度 */
     
    ret =i2c_transfer(am2320_client->adapter,&msg[1],1);
    for (size_t i = 0; i < 8; i++)
        read_data_transfer[i] = read_data[i];
    if(crc16(read_data_transfer,6) == (read_data[6] | read_data[7]<<8))
    {
         data[0]  = (read_data[3]|read_data[2]<<8); // 
         data[1] =  (read_data[5]|read_data[4]<<8);
    }
    return 0;
}