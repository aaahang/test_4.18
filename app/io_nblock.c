#include "stdio.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <unistd.h>
#include <poll.h>
#include "stdlib.h"
#define POLLSIZE  3
union 
{
    int value;
    char data[4];
}read_data;
int main(int argc ,char *argv[])
{

    int fd = open(argv[1],O_RDWR);
    // struct pollfd plfd[POLLSIZE];
    // plfd[0].fd = fd ;
    // plfd[0].events = POLLIN;
    // int ret = 0;
    // while(1)
    // { 
    //     ret = poll(plfd,1,100); //不能写成 无超时唤醒遥遥无期 超时时间内会检查两次poll_drv 
    //     if((ret) == 0) printf("poll is time out\n"); 
    //     if (plfd[0].revents&POLLIN) // 只对三个位进行赋值 只需要 与操作便可     
    //     {
    //     read(plfd[0].fd, read_data.data, 4);
    //     printf(" key value is %d  revents is %p\n",read_data.value,plfd[0].revents);        
    //     plfd[0].revents =0 ;
    // }
    // }
    // return  0;
    read(fd, read_data.data, 4);
    printf("%d",read_data.value);

    return 0;
}