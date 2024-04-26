#include "stdio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/ioctl.h"

#define IO_TIMESTOP _IO(0,'C')
#define IO_TIMEWIRTE _IOW(1,'C',int)
int main(int argc,char *argv[])
{
    int fd = open(argv[1],O_RDWR);
    while (1);
    // {
    //     scanf("%s",)
    //     ioctl(fd,IO_TIMESTOP)
    // }
    return 0;
}