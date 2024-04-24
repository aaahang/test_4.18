#include "stdio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc,char *argv[])
{
    open(argv[1],O_RDWR);
    while (1);
    return 0;
}