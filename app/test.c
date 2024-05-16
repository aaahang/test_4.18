#include "stdio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <unistd.h>


int main(int argc,char *argv[])
{
    char buf[10];
    int fd = open(argv[1],O_RDWR);
    while (1)
    {
      read(fd,buf,10);
      sleep(1);
    }
    return 0;
}