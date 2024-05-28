#include "stdio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include "adrc_algorithm.c"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

struct 
{
  int  tem;
  int  hum;
}am2320_hdata;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
typedef  struct  {
  int fd[2];

}public_source;



/*向主机传输数据*/
#define HOST "192.168.0.112"        // 根据你服务器的IP地址修改
#define PORT 9000                   // 根据你服务器进程绑定的端口号修改
#define BUFFER_SIZ (4 * 1024)           // 4k的数据区域
void * temdata_routine(void * data)
{
  union {
    struct 
    {
        int tem;
        int hum;     
    }rl;
    char trans[8];
  }am2320_tranfer;
  public_source * ps = (public_source *) data;
  int fd = open("/dev/tem_con",O_RDWR);


    union 
    {
      struct 
      {
        int hum;
        int tem;
        int out;
      }value;
      char data[12]; 
    }socket_data;
     int sockfd, ret;
     int client_fd;
     struct sockaddr_in ser_addr;
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_fd < 0)
     {
        printf("create socket fail!\n");
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(HOST);  //注意网络序转换
    ser_addr.sin_port = htons(9000);  //注意网络序转换


  while (1)
  {
    read(fd,am2320_tranfer.trans,8);
    // printf("tem is %d,hum is %d\n",am2320_tranfer.rl.tem,am2320_tranfer.rl.hum);
    pthread_mutex_lock(&mutex);
    am2320_hdata.tem = am2320_tranfer.rl.tem;
    am2320_hdata.hum = am2320_tranfer.rl.hum;
    socket_data.value.tem = am2320_hdata.tem;
    socket_data.value.hum = am2320_hdata.hum;
    pthread_mutex_unlock(&mutex);
    sendto(client_fd, socket_data.data, sizeof(socket_data.data), 0,(struct sockaddr*)& ser_addr, sizeof(ser_addr));
  }

  return NULL;
}
union 
{
    struct {
        int pwm,con;
    }ik;
    char uk[16];
}pwm_k;
 void * con_routine(void * data)
 {
    struct 
    {
      int  tem;
      int  hum;
    }con_data;
    int fd = open("/dev/device_pwm",O_RDWR);
    while (1)
    {
      pthread_mutex_lock(&mutex);
      con_data.tem = am2320_hdata.tem;
      con_data.hum = am2320_hdata.hum;
      pthread_mutex_unlock(&mutex);
      static ADRC_Controller * adrc;
      adrc = adrc_algorithm(con_data.tem,con_data.hum);
      pwm_k.ik.pwm  = adrc->y;
      pwm_k.ik.con  = 1;
      write(fd,pwm_k.uk,16);
    }

    return NULL;
 }

int main(int argc,char *argv[])
{
    public_source ps;
    pthread_t con_pthread,tem_pthread;
    char buf[10];
    /*创建线程*/
    pthread_create(&tem_pthread,NULL, temdata_routine, &ps);
    pthread_create(&con_pthread,NULL, con_routine, &ps);
    pthread_join(con_pthread,NULL);
    pthread_join(tem_pthread,NULL);
    return 0;
}