#include "client.h"
//循环发送循环接收
void send_n(int sockfd,char *buf,int len)
{
    int ret;
    int total=0;
    while(total<len)
    {
        ret=send(sockfd,buf+total,len-total,0);
        total+=ret;
    }
}

void recv_n(int sockfd,char *buf,int len)
{
    int ret;
    int total=0;
    while(total<len)
    {
        ret=recv(sockfd,buf+total,len-total,0);
        total+=ret;
    }
}

